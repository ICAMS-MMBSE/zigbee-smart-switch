import typer
import serial
import json
from typing import Annotated
from pathlib import Path
from digi.xbee.devices import XBeeDevice

app = typer.Typer(no_args_is_help=True,
                  help="Manage and monitor Zigbee smart switch network via the commissioner"
                  )

CONFIG_PATH = Path.home() / ".zss" / "config.json"


def load_config() -> dict:
    if not CONFIG_PATH.exists():
        typer.echo("No config found. Run 'init' first.")
        raise typer.Exit()
    with open(CONFIG_PATH) as f:
        return json.load(f)


@app.command(no_args_is_help=True)
def init(
    port: Annotated[str, typer.Argument(help="Serial port, e.g. /dev/ttyACM0")],
    baud_rate: Annotated[int, typer.Option(help="Baud rate")] = 115200
):
    """Initialize the commissioner connection and save config."""
    CONFIG_PATH.parent.mkdir(parents=True, exist_ok=True)
    config = {"port": port, "baud_rate": baud_rate}
    with open(CONFIG_PATH, "w") as f:
        json.dump(config, f, indent=2)
    print(f"Config saved: {port} at {baud_rate} baud")


@app.command()
def ping():
    """Test basic serial connection to Commissioner."""
    config = load_config()
    try:
        ser = serial.Serial(config["port"], config["baud_rate"], timeout=3)
        print(f"Connected to {config['port']} at {config['baud_rate']} baud")
        ser.close()
        print("Connection closed successfully")
    except Exception as e:
        print(f"Failed to connect: {e}")


@app.command()
def discover():
    """Discover all XBee devices on the network."""
    config = load_config()
    device = XBeeDevice(config["port"], config["baud_rate"])

    try:
        device.open()

        network = device.get_network()
        network.start_discovery_process()

        print("Discovering devices...")
        while network.is_discovery_running():
            pass

        devices = network.get_devices()
        for d in devices:
            print(f"Found: {d.get_node_id()} - {d.get_64bit_addr()}")
    except Exception as e:
        print(f"Discovery failed: {e}")
    finally:
        device.close()


@app.command(no_args_is_help=True)
def status(
    node_id: Annotated[str, typer.Argument(help="Node Identifier, e.g. SS_0")],
):
    """Fetch the status of a specific device by its Node Identifier."""
    config = load_config()
    commander = XBeeDevice(config["port"], config["baud_rate"])
    try:
        commander.open()
        network = commander.get_network()
        network.start_discovery_process()
        while network.is_discovery_running():
            pass

        device = network.get_device_by_node_id(node_id)
        if device is None:
            print(f"Device '{node_id}' not found on network")
            return

        print(f"Node ID: {device.get_node_id()}")
        print(f"Address: {device.get_64bit_addr()}")

    except Exception as e:
        print(f"Failed: {e}")
    finally:
        commander.close()


# TO DO:
# switch -- turn a device on/off
# signal -- get RSSI/signal strength of a specific device, useful for network health
# summary -- give useful stats at a glance
# MQTT configuration: listen and publish

if __name__ == "__main__":
    app()