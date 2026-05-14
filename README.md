# zigbee-smart-switch

## Purpose 
The purpose of this project is to:
 - Explore zigbee, MQTT5, and docker
 - Containerize code such that it is portable
 - Create a smart switch to control a device 
 - Connect into a greater system through MQTT5
 - Prototype with Digi Xbee3 Zigbee 3.0 TH with Sparkfun Xbee Shield to interface with Arduino Uno R4 Wifi

## Frameworks \& Software
 - [Digi XCTU](https://www.digi.com/products/embedded-systems/digi-xbee/digi-xbee-tools/xctu) used to program RCP unit.
 - [XBee-Arduino](https://github.com/andrewrapp/xbee-arduino) library used to support Digi Xbee RCP.
 - [Mosquitto](https://github.com/eclipse-mosquitto/mosquitto) to handle MQTT5 API.
 - [Docker Compose](https://docs.docker.com/compose/) to containerize each service.
  
## Firmware
The [Arduino IDE](https://www.arduino.cc/en/software/#app-lab-section) can be used to install the library, and C++ will be used to write the sensor and device drivers.

To program the RCP, you **must** use the XBee Grove Development Board and XCTU. Plug in with USB micro B and add device to XCTU. The following parameters are used:
| | Commissioner | Joiner |
| --- | --- | --- |
|**CE** Device Role | Form Network [1] | Join Network [0] |
| **ID** Extended PAN ID | 1234 | 1234| 
| **JV** Coordinator Verification | Enabled [1] | Disabled [0] |
| **JN** Join Notification | Disabled [0] | Enabled [1] |
| **NI** Node Identifier | SS_Comm | SS_x (0...) |
| **BD** UART Baud Rate | 115200 | 115200 |
| **AP** API Enable | API Mode With Escapes [2] | API Mode With Escapes [2] |
| **SM** Sleep Mode | No Sleep (Router) [0] | No Sleep (Router) [0] |
| **EE** Encryption Enable | Disabled [0] | Disabled [0] |

## References
 - [XBee Shield Hookup Guide](https://learn.sparkfun.com/tutorials/xbee-shield-hookup-guide/all#example-communication-test)
 - [Arduino Uno R4 docs](https://docs.arduino.cc/hardware/uno-r4-wifi/)
 - [Arduino API](https://docs.arduino.cc/learn/programming/reference/#structure)
 - [XBee Grove Development Board](https://docs.digi.com/resources/documentation/digidocs/pdfs/90001457-13.pdf)