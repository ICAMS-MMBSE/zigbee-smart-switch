import typer
import serial
import json
from typing import Annotated
from pathlib import Path

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

# Helper function -- sends command serially to arduino
def send_command(command: str) -> list[str]:
    """Send a command to the Arduino and collect the response lines."""
    config = load_config() # defined by init (port, baud rate)
    lines = []
    with serial.Serial(config["port"], config["baud_rate"], timeout=5) as ser: # establish serial connection
        ser.write(f"{command}\n".encode()) # write command
        while True:
            line = ser.readline().decode().strip() # read
            if not line:
                break # break on empty line (end of response)
            lines.append(line) # add to lines array
    return lines # return

# Helper command to setup commissioner once and remember
@app.command()
def init(
    port: Annotated[str, typer.Argument(help="Serial port, e.g. COM4 or /dev/ttyACM0")],
    baud_rate: Annotated[int, typer.Option(help="Baud rate")] = 115200
):
    """Initialize the commissioner connection and save config."""
    CONFIG_PATH.parent.mkdir(parents=True, exist_ok=True)
    config = {"port": port, "baud_rate": baud_rate}
    with open(CONFIG_PATH, "w") as f:
        json.dump(config, f, indent=2)
    print(f"Config saved: {port} at {baud_rate} baud")

# Establish serial communication to Arduino
@app.command()
def ping():
    """Test basic serial connection to the Arduino."""
    config = load_config()
    try:
        ser = serial.Serial(config["port"], config["baud_rate"], timeout=3)
        print(f"Connected to {config['port']} at {config['baud_rate']} baud")
        ser.close()
        print("Connection closed successfully")
    except Exception as e:
        print(f"Failed to connect: {e}")

# Queries SH, SL, and AI from the XBee RCP through the Arduino
@app.command()
def atping():
    """Test serial communication to RCP through the Arduino."""
    print("Establishing connection...")
    try:
        lines = send_command("AT_PING")
        for line in lines:
            print(line)
    except Exception as e:
        print(f"ATping failed: {e}")

# Discovers all XBee devices on the network
@app.command()
def discover():
    """Discover all XBee devices on the network."""
    print("Discovering devices...")
    try:
        lines = send_command("DISCOVER")
        for line in lines:
            print(line)
    except Exception as e:
        print(f"Discovery failed: {e}")

# Fetches status of a specific node by Node Identifier
@app.command(no_args_is_help=True)
def status(
    node_id: Annotated[str, typer.Argument(help="Node Identifier, e.g. SS_0")],
):
    """Fetch the status of a specific device by its Node Identifier."""
    try:
        lines = send_command(f"STATUS {node_id}")
        for line in lines:
            print(line)
    except Exception as e:
        print(f"Status failed: {e}")

# Turn a joiner LED on or off by Node Identifier
@app.command(no_args_is_help=True)
def switch(
    node_id: Annotated[str, typer.Argument(help="Node Identifier, e.g. SS_0 or ALL")],
    state: Annotated[int, typer.Argument(help="1 = on, 0 = off")],
):
    """Turn a joiner LED on or off by Node Identifier."""
    try:
        lines = send_command(f"LED {node_id} {state}")
        for line in lines:
            print(line)
    except Exception as e:
        print(f"Switch failed: {e}")

# TO DO:
# signal -- get RSSI/signal strength of a specific device, useful for network health
# summary -- give useful stats at a glance
# MQTT configuration: listen and publish

if __name__ == "__main__":
    app()