# zigbee-smart-switch
A Zigbee-based smart switch prototype built with Digi XBee3, Arduino Uno R4 WiFi, MQTT5, and Docker.
## Purpose
- Explore Zigbee, MQTT5, and Docker
- Containerize services for portability
- Build a smart switch to control a device remotely
- Integrate into a broader system via MQTT5
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
## Python CLI -- ZSS
A CLI implemented using [pyserial](https://pypi.org/project/pyserial/) and [Typer](__https://typer.tiangolo.com/__). It provides:
1. An interface between the end user and the smart switch
2. Methods for device control, network discovery, and testing
This will be called ZSS, or Zigbee Smart Switch.
### Setup
Create and activate a virtual environment, then install dependencies:
```bash
python -m venv venv
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
## References
- [XBee Shield Hookup Guide](__https://learn.sparkfun.com/tutorials/xbee-shield-hookup-guide/all#example-communication-test__)
- [Arduino Uno R4 WiFi Docs](__https://docs.arduino.cc/hardware/uno-r4-wifi/__)
- [Arduino Language Reference](__https://docs.arduino.cc/learn/programming/reference/#structure/__)
- [XBee Grove Development Board User Guide](__https://docs.digi.com/resources/documentation/digidocs/pdfs/90001457-13.pdf__)
- [PlatformIO Documentation](__https://docs.platformio.org/__)