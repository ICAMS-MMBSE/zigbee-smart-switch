#include "ethernet.h"
#include "config.h"
#include "esp_log.h"
#include "esp_eth.h"
#include "esp_eth_mac_w5500.h"
#include "esp_eth_phy_w5500.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "driver/spi_master.h"

static const char *TAG = "ETHERNET";

static void eth_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    switch (event_id) {
        case ETHERNET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Ethernet connected");
            break;
        case ETHERNET_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "Ethernet disconnected");
            break;
        case ETHERNET_EVENT_START:
            ESP_LOGI(TAG, "Ethernet started");
            break;
        default:
            break;
    }
}

static void ip_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    if (event_id == IP_EVENT_ETH_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

void ethernet_init(void)
{
    ESP_LOGI(TAG, "Ethernet init starting...");
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&netif_cfg);

    // Static IP
    esp_netif_dhcpc_stop(eth_netif);
    esp_netif_ip_info_t ip_info = {
        .ip      = { .addr = ESP_IP4TOADDR(192, 168, 2, 10) },
        .netmask = { .addr = ESP_IP4TOADDR(255, 255, 255, 0) },
        .gw      = { .addr = ESP_IP4TOADDR(192, 168, 2, 1) },
    };
    esp_netif_set_ip_info(eth_netif, &ip_info);

    // SPI bus
    spi_bus_config_t buscfg = {
        .miso_io_num   = W5500_MISO,
        .mosi_io_num   = W5500_MOSI,
        .sclk_io_num   = W5500_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // W5500 config
    spi_device_interface_config_t devcfg = {
        .command_bits   = 16,
        .address_bits   = 8,
        .mode           = 0,
        .clock_speed_hz = 20 * 1000 * 1000,
        .spics_io_num   = W5500_CS,
        .queue_size     = 20,
    };
    eth_w5500_config_t w5500_cfg = ETH_W5500_DEFAULT_CONFIG(SPI2_HOST, &devcfg);
    w5500_cfg.int_gpio_num   = W5500_INT;
    w5500_cfg.poll_period_ms = 0;

    eth_mac_config_t mac_cfg = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_cfg = ETH_PHY_DEFAULT_CONFIG();
    phy_cfg.reset_gpio_num = W5500_RST;

    esp_eth_mac_t *mac = esp_eth_mac_new_w5500(&w5500_cfg, &mac_cfg);
    esp_eth_phy_t *phy = esp_eth_phy_new_w5500(&phy_cfg);

    esp_eth_config_t eth_cfg = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth_handle = NULL;
    ESP_ERROR_CHECK(esp_eth_driver_install(&eth_cfg, &eth_handle));

    // Set MAC address
    uint8_t mac_addr[6] = {0x02, 0x00, 0x00, 0x12, 0x34, 0x56};
    esp_eth_ioctl(eth_handle, ETH_CMD_S_MAC_ADDR, mac_addr);

    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));

    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID,
                                                &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP,
                                                &ip_event_handler, NULL));

    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
    ESP_LOGI(TAG, "Ethernet init done");
}