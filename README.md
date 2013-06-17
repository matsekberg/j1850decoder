j1850decoder
============

The source code for an Arduino based J1850 decoder.

For one of my other projects (mclogger) I needed to decode J1850/OBD2 messages for logging purposes. I used ebay BlueTooth OBD2 adapters and BlueTooth master modules until I abandoned the overhead of wireless readings. The J1850 protocol is a simple protocol to transfer a couple of bits so I decided to write my own decoder using an Arduino Pro Mini. The task is to detect pulses with a duration of between 64 and 200 microseconds. 

One digital input connected to the data stream on the J1850-bus and another pin with the encoded data delivered as ASCII hex messages at 115200 baud.

Board: Arduino Pro Mini 5V 16MHz ATMega168

###Below is a recording from my Harley Sportster:

	Unique frames ------------------
     [281B10 020000 D5] [P1 H3 YF Z0 T1B S10] Status: RPM 0 (8 Function Command/Status)
     [482910 020000 56] [P2 H3 YF Z0 T29 S10] Status: Speed 0 (8 Function Command/Status)
     [A86910 860000 D7] [P5 H3 YF Z0 T69 S10] Status: ODO last 0.0 (8 Function Command/Status)
     [688810 83 62] [P3 H3 YF Z0 T88 S10] Command: Tell tales Check engine  = T (8 Function Command/Status)
     [C88810 0E BA] [P6 H3 YF Z0 T88 S10] Command: Tell tales Low voltage (8 Function Command/Status)
     [299210 01 60] [P1 H3 YF Z1 T92 S10] Command: ? Vehicle security (9 Function Request/Query)
     [289310 0200 8C] [P1 H3 YF Z0 T93 S10] Status: ? Vehicle security (8 Function Command/Status)
     [489240 AAFFFF 5B] [P2 H3 YF Z0 T92 S40] Command: ? Vehicle security (8 Function Command/Status)
     [289340 019B3B CA] [P1 H3 YF Z0 T93 S40] Status: ? Vehicle security (8 Function Command/Status)
     [48DA40 3902 4A] [P2 H3 YF Z0 TDA S40] Command: Turn signal Right (8 Function Command/Status)
	Frames from sources ------------------
  Source = 10
    [281B10 020000 D5] [P1 H3 YF Z0 T1B S10] Status: RPM 0 (8 Function Command/Status)
    [482910 020000 56] [P2 H3 YF Z0 T29 S10] Status: Speed 0 (8 Function Command/Status)
    [A86910 860000 D7] [P5 H3 YF Z0 T69 S10] Status: ODO last 0.0 (8 Function Command/Status)
    [688810 83 62] [P3 H3 YF Z0 T88 S10] Command: Tell tales Check engine  = T (8 Function Command/Status)
    [C88810 0E BA] [P6 H3 YF Z0 T88 S10] Command: Tell tales Low voltage (8 Function Command/Status)
    [299210 01 60] [P1 H3 YF Z1 T92 S10] Command: ? Vehicle security (9 Function Request/Query)
    [289310 0200 8C] [P1 H3 YF Z0 T93 S10] Status: ? Vehicle security (8 Function Command/Status)
  Source = 40
    [489240 AAFFFF 5B] [P2 H3 YF Z0 T92 S40] Command: ? Vehicle security (8 Function Command/Status)
    [289340 019B3B CA] [P1 H3 YF Z0 T93 S40] Status: ? Vehicle security (8 Function Command/Status)
    [48DA40 3902 4A] [P2 H3 YF Z0 TDA S40] Command: Turn signal Right (8 Function Command/Status)

