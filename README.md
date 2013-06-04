j1850decoder
============

The source code for an Arduino based J1850 decoder.

For one of my other projects (mclogger) I needed to decode J1850/OBD2 messages for logging purposes. I used ebay BlueTooth OBD2 adapters and BlueTooth master modules until I abandoned the overhead of wireless readings. The J1850 protocol is a simple protocol to transfer a couple of bits so I decided to write my own decoder using an Arduino Pro Mini.

One digital input connected to the data stream on the J1850-bus and another pin with the encoded data delivered as ASCII hex messages at 115200 baud.

The task is to detect pulses with a duration of between 64 and 200 microseconds. 


Board: Arduino Pro Mini 5V 16MHz ATMega168