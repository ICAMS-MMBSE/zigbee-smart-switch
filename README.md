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
| [Digi XCTU](https://www.digi.com/products/embedded-systems/digi-xbee/digi-xbee-tools/xctu) | Programs the RCP unit |
| [XBee-Arduino](https://github.com/andrewrapp/xbee-arduino) | Arduino library for Digi XBee RCP support |
| [Mosquitto](https://github.com/eclipse-mosquitto/mosquitto) | MQTT5 broker |
| [Docker Compose](https://docs.docker.com/compose/) | Containerizes each service |

## Firmware

Firmware is written in C++ using the [Arduino IDE](https://www.arduino.cc/en/software/#app-lab-section), which is also used to install the required libraries.

### Programming the RCP

> **Note:** You **must** use the XBee Grove Development Board and XCTU to program the RCP. Connect via USB Micro-B and add the device in XCTU.

The following AT parameters should be configured for the Commissioner (coordinator) and Joiner (router) roles:

| Parameter | Commissioner | Joiner |
|---|---|---|
| **CE** Device Role | Form Network `[1]` | Join Network `[0]` |
| **ID** Extended PAN ID | `1234` | `1234` |
| **JV** Coordinator Verification | Enabled `[1]` | Disabled `[0]` |
| **JN** Join Notification | Disabled `[0]` | Enabled `[1]` |
| **NI** Node Identifier | `SS_Comm` | `SS_x` (0, 1, ...) |
| **BD** UART Baud Rate | `115200` | `115200` |
| **AP** API Enable | API Mode With Escapes `[2]` | API Mode With Escapes `[2]` |
| **SM** Sleep Mode | No Sleep / Router `[0]` | No Sleep / Router `[0]` |
| **EE** Encryption Enable | Disabled `[0]` | Disabled `[0]` |

## Python CLI -- ZSS

A CLI is implemented using the [Digi XBee Python Library](https://github.com/digidotcom/xbee-python) ([docs](https://xbplib.readthedocs.io/en/latest/)) and [Typer](https://typer.tiangolo.com/). It provides:

1. An interface between the end user and the smart switch
2. Methods for device control, network discovery, and testing

This will be called ZSS, or Zigbee Smart Switch.
### Setup

Create and activate a virtual environment, then install dependencies:

```bash
python -m venv venv
source venv/bin/activate        # Windows: venv\Scripts\activate
pip install -r zss/requirements.txt
```

> **VSCode tip:** Open the command palette (`Ctrl+Shift+P` / `Cmd+Shift+P`) → **Python: Select Interpreter** → select `./venv/bin/python`. VSCode will then auto-activate the venv in every new integrated terminal.

### Usage

```bash
python main.py --help
python main.py <command> [OPTIONS]
```

## References

- [XBee Shield Hookup Guide](https://learn.sparkfun.com/tutorials/xbee-shield-hookup-guide/all#example-communication-test)
- [Arduino Uno R4 WiFi Docs](https://docs.arduino.cc/hardware/uno-r4-wifi/)
- [Arduino Language Reference](https://docs.arduino.cc/learn/programming/reference/#structure)
- [XBee Grove Development Board User Guide](https://docs.digi.com/resources/documentation/digidocs/pdfs/90001457-13.pdf)