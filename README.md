# zigbee-smart-switch

A Zigbee-based smart switch system for remote dehumidifier control, built for Industry 4.0 reliability, connectivity, and remote management.

## Purpose
- Build a smart switch to control a device remotely
- Integrate IoT connection through Mosquitto
- Containerize services for portability
- Prototype using the Digi XBee3 Zigbee 3.0 TH module with an ESP32-S3 (commissioner) and ESP32-C3 (joiner)

## Frameworks & Software

| Tool | Purpose |
|---|---|
| [Digi XCTU](https://www.digi.com/products/embedded-systems/digi-xbee/digi-xbee-tools/xctu) | Programs XBee modules |
| [PlatformIO](https://platformio.org/) | Build system and library manager for all firmware |
| [Mosquitto](https://github.com/eclipse-mosquitto/mosquitto) | MQTT broker |
| [Docker Compose](https://docs.docker.com/compose/) | Containerizes each service |

## Firmware

Firmware is written in C using the ESP-IDF framework with FreeRTOS, built and flashed via VS Code with the PlatformIO extension.

### Setup

1. Install [VS Code](https://code.visualstudio.com/) and the [PlatformIO extension](https://platformio.org/install/ide?install=vscode)
2. Open `Commissioner/` or `Joiner/` as the workspace root in VS Code
3. PlatformIO will auto-detect the target from `platformio.ini`
4. Build and flash via the PlatformIO toolbar, or `pio run -t upload`

### Programming the XBee Modules

> **Note:** You **must** use the XBee Grove Development Board and XCTU to program the XBee modules. Plug in with USB micro-B and add device to XCTU.

| | Commissioner | Joiner |
| --- | --- | --- |
| **CE** Device Role | Form Network [1] | Join Network [0] |
| **ID** Extended PAN ID | 1234 | 1234 |
| **JV** Coordinator Verification | Enabled [1] | Disabled [0] |
| **JN** Join Notification | Disabled [0] | Enabled [1] |
| **NI** Node Identifier | SS_Comm | SS_x (0...) |
| **BD** UART Baud Rate | 115200 | 115200 |
| **AP** API Enable | API Mode [1] | API Mode [1] |
| **SM** Sleep Mode | No Sleep [0] | No Sleep [0] |
| **EE** Encryption Enable | Disabled [0] | Disabled [0] |

### Firmware Structure

#### Commissioner (`firmware/Commissioner/`)
```
config.h
xbee.h/.c       — raw API frame TX/RX, AT commands, ping, RX task
discovery.h/.c  — ND-based dynamic node table
relay.h/.c      — command routing, LED state
button.h/.c     — physical button, triggers toggle to all nodes
ethernet.h/.c   — W5500 SPI ethernet, static IP
mqtt.h/.c       — MQTT client, command handler, state publisher
main.c
```

#### Joiner (`firmware/Joiner/`)
```
config.h
xbee.h/.c       — raw API frame TX/RX, state reporting
relay.h/.c      — relay GPIO control
button.h/.c     — manual override, reports state to commissioner
main.c
```

### Key Technical Notes

- All XBee communication uses manually constructed raw API frames
- Frame types used: `0x10` ZBTxRequest, `0x08` AT command, `0x88` AT response, `0x90` ZB RX, `0x8B` TX status

## Hardware

### Commissioner
- ESP32-S3-WROOM-1 + XBee3 Zigbee 3.0 TH + W5500 Ethernet
- GPIO: IO16 LED, IO15 button, IO6/IO7 XBee UART, IO10-13 W5500 SPI

### Joiner
- ESP32-C3 Super Mini + XBee3 Zigbee 3.0 TH
- 2N2222A transistor (low-side switch), 1N4001 flyback diode, 5V relay
- Mains load switched via relay on cut extension cord
- Powered from relay supply line, no USB required

## Python CLI - ZSS

`zss` (Zigbee Smart Switch) is a CLI tool implemented using [Typer](https://typer.tiangolo.com/).
Purpose:
1. Provide an interface between the end user and the commissioner
2. Serve as middleware to be connected into a larger system (abstraction)
3. Provide a simple way to interact with RCP without reflashing firmware

### Setup

This guide assumes **Python3** is already installed.

 - Create and activate a virtual environment, then install dependencies:
```bash
python -m venv venv          # Version 3.13.5 (current)
venv\Scripts\activate        # Windows: venv\bin\activate (Linux/Mac)
pip install -r requirements.txt
```
> **VSCode tip:** Open the command palette (`Ctrl+Shift+P` / `Cmd+Shift+P`) → **Python: Select Interpreter** → select `./venv/bin/python`. VSCode will then auto-activate the venv in every new integrated terminal.

### Usage

```bash
python main.py --help
python main.py init <port>              # e.g. init COM4
python main.py ping                     # test serial connection
python main.py atping                   # query XBee RCP via AT commands
python main.py switch <node_id> <0|1>   # e.g. switch SS_1 1, switch ALL 0
```

### ZSS API Reference

[Zigbee API frame structure](https://docs.digi.com/resources/documentation/digidocs/rf-docs/blu/blu-api-frames_c.html)
| Command | Function | Implemented |
|---------|----------|-------------|
| AT_PING | Basic "are you there?" signal. Tests serial communication between the XBee radio and your computer through the Arduino. | Yes |
| ATWR (commit) | Writes current radio configuration to non-volatile memory. Use after making config changes to persist them across power cycles. | No |
| DISCOVER | Promiscuously finds and lists all nodes on the network. | Yes |
| STATUS {node_id} | Get the status of a specific node by its Node Identifier (e.g. SS_0). | Yes |
| SWITCH {node_id} {on\|off} | Turn a node's switch on or off by Node Identifier. Use ALL to target every node. | Yes |
| SIGNAL {node_id} | Get the RSSI signal strength of a specific node. Useful for diagnosing network health. | No |
| SUMMARY | Discover all nodes and display a combined status overview in a single table. | No |

## KiCad Setup
- Commissioner PCB in progress
- Joiner PCB pending Dr. Osho power spec confirmation

## Raspberry Pi Broker Setup

The Pi runs a Mosquitto MQTT broker and connects directly to the commissioner via ethernet.

### Network
Create `/etc/systemd/network/10-eth0.network`:
```
[Match]
Name=eth0

[Network]
Address=192.168.2.1/24
```
```bash
sudo systemctl enable systemd-networkd
sudo systemctl restart systemd-networkd
```

### Mosquitto
```bash
sudo apt update && sudo apt install -y mosquitto mosquitto-clients
sudo systemctl enable mosquitto
```

Create `/etc/mosquitto/conf.d/local.conf`:
```
listener 1883
allow_anonymous true
```
```bash
sudo systemctl restart mosquitto
```

### MQTT Topics
| Topic | Direction | Description |
|-------|-----------|-------------|
| `switch/all/cmd` | Pi → Commissioner | Command all nodes |
| `switch/1/cmd` | Pi → Commissioner | Command node 1 |
| `switch/SS_1/state` | Commissioner → Pi | Joiner state report |

Supported commands: `ON`, `OFF`, `TOGGLE`, `POLL`

### Examples
```bash
# Turn all nodes on
mosquitto_pub -h localhost -t "switch/all/cmd" -m "ON"

# Turn all nodes off
mosquitto_pub -h localhost -t "switch/all/cmd" -m "OFF"

# Poll current state without changing it
mosquitto_pub -h localhost -t "switch/all/cmd" -m "POLL"

# Subscribe to state updates
mosquitto_sub -h localhost -t "switch/SS_1/state"
```

## Roadmap
- [x] W5500 Ethernet on commissioner
- [x] Connect commissioner to Raspberry Pi MQTT broker
- [ ] MQTT integration
- [x] Humidity-threshold automation
- [ ] Joiner PCB
- [ ] Finalize Commissioner PCB
- [ ] Order and Assemble PCBs
- [ ] Containerize services

## References
- [Digi XBee 3](https://hub.digi.com/support/products/digi-xbee/digi-xbee3/)
- [Digi XBee API Frame Reference](https://docs.digi.com/resources/documentation/digidocs/rf-docs/blu/blu-api-frames_c.html)
- [XBee Grove Development Board User Guide](https://docs.digi.com/resources/documentation/digidocs/pdfs/90001457-13.pdf)
- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [Digi XBee Python Library (not used, but helpful)](https://xbplib.readthedocs.io/en/latest/index.html)