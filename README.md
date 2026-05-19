# zigbee-smart-switch

A Zigbee-based smart switch prototype built for Industry 4.0 reliability, connectivity, and remote management.

## Purpose
- Build a smart switch to control a device remotely
- Integrate IoT connection through Mosquitto
- Containerize services for portability
- Prototype using the Digi XBee3 Zigbee 3.0 TH module with a SparkFun XBee Shield on an Arduino Uno R4 WiFi
  
## Frameworks & Software

| Tool | Purpose |
|---|---|
| [Digi XCTU](__https://www.digi.com/products/embedded-systems/digi-xbee/digi-xbee-tools/xctu__) | Programs the RCP unit |
| [PlatformIO](__https://platformio.org/__) | Build system and library manager for Arduino firmware |
| [XBee-Arduino](__https://github.com/andrewrapp/xbee-arduino__) | Arduino library for Digi XBee RCP support |
| [Mosquitto](__https://github.com/eclipse-mosquitto/mosquitto__) | MQTT5 broker |
| [Docker Compose](__https://docs.docker.com/compose/__) | Containerizes each service |

## Firmware

Firmware is written in C++ using the Arduino framework, built and flashed via [PlatformIO](__https://platformio.org/__) in VS Code. Libraries are managed per-project in `platformio.ini`.

### Setup

1. Install [VS Code](https://code.visualstudio.com/) and the [PlatformIO extension](__https://platformio.org/install/ide?install=vscode__)
2. Open `Commissioner/` or `Joiner/` as the workspace root in VS Code
3. PlatformIO will auto-install dependencies on first build
4. Flash via the upload arrow in the bottom toolbar

### Programming the RCP

> **Note:** You **must** use the XBee Grove Development Board and XCTU to program the RCP. Plug in with USB micro B and add device to XCTU.

| | Commissioner | Joiner |
| --- | --- | --- |
| **CE** Device Role | Form Network [1] | Join Network [0] |
| **ID** Extended PAN ID | 1234 | 1234 |
| **JV** Coordinator Verification | Enabled [1] | Disabled [0] |
| **JN** Join Notification | Disabled [0] | Enabled [1] |
| **NI** Node Identifier | SS_Comm | SS_x (0...) |
| **BD** UART Baud Rate | 115200 | 115200 |
| **AP** API Enable | API Mode With Escapes [2] | API Mode With Escapes [2] |
| **SM** Sleep Mode | No Sleep (Router) [0] | No Sleep (Router) [0] |
| **EE** Encryption Enable | Disabled [0] | Disabled [0] |

## Python CLI - ZSS

`zss` (Zigbee Smart Switch) is a CLI tool implemented using [Typer](__https://typer.tiangolo.com/__).
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

### ZSS API Reference

| Command | Function | Implemented |
|---------|----------|-------------|
| AT_PING | Basic "are you there?" signal. Tests serial communication between the XBee radio and your computer through the Arduino. | Yes |
| ATWR (commit) | Writes current radio configuration to non-volatile memory. Use after making config changes to persist them across power cycles. | No |
| DISCOVER | Promiscuously finds and lists all nodes on the network. | Yes |
| STATUS {node_id} | Get the status of a specific node by its Node Identifier (e.g. SS_0). | Yes |
| SWITCH {node_id} {on\|off} | Turn a node's switch on or off by Node Identifier. Use ALL to target every node. | Yes |
| SIGNAL {node_id} | Get the RSSI signal strength of a specific node. Useful for diagnosing network health. | No |
| SUMMARY | Discover all nodes and display a combined status overview in a single table. | No |


## References
- [XBee Shield Hookup Guide](__https://learn.sparkfun.com/tutorials/xbee-shield-hookup-guide/all#example-communication-test__)
- [Arduino Uno R4 WiFi Docs](__https://docs.arduino.cc/hardware/uno-r4-wifi/__)
- [Arduino Language Reference](__https://docs.arduino.cc/learn/programming/reference/#structure/__)
- [XBee Grove Development Board User Guide](__https://docs.digi.com/resources/documentation/digidocs/pdfs/90001457-13.pdf__)
- [PlatformIO Documentation](__https://docs.platformio.org/__)