//
// J1850 decoder
// Mats Ekberg (c) 2013
// 
// This software decodes the binary data stream from a J1850 compliant 
// OBD2 device. Using just one digital input for the data stream and
// transmitting the decoded data on the UART.
//
// Chip:  Arduino Mini Pro w ATmega168 at 16MHz
//


// I/O and interrupts. Pin/Int
// UNO:   2/0, 3/1. 
// Mega:  2/0, 3/1, 21/2, 20/3, 19/4, 18/5
#define J1850_PIN  2
#define J1850_INT  0

#define LED_PIN 13

// Timing for start of frame
#define SOF_TIME      200
#define SOF_DEV        18

// Timing for end of frame
#define EOF_TIME      200
#define EOF_DEV        18

// Timing for a one-bit
#define BITONE_TIME   128
#define BITONE_DEV     16

// Timing for a zero-bit
#define BITZERO_TIME   64
#define BITZERO_DEV    15

// forward decl
void j1850_interrupt(void);
enum state_t {
  idle, header, data};

// Storage
volatile uint8_t buf[15];
volatile uint8_t bufIdx;
volatile uint8_t msgLen = 0;

volatile long lastInt = 0;
volatile uint8_t bitCnt;
volatile state_t state;

//
// Initialization
//
void setup(void) 
{  
  pinMode(J1850_PIN, INPUT_PULLUP);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  delay(2000);
  digitalWrite(LED_PIN, LOW);
  Serial.begin(38400);
  Serial.println(F("j1850.v1"));

  state = idle;

  attachInterrupt(J1850_INT, j1850_interrupt, CHANGE);
}


//
// Background loop - print message when available
//
void loop(void)
{
  if (msgLen > 0) {
    digitalWrite(LED_PIN, HIGH);
    Serial.print(">");
    for (int i = 0; i < msgLen; i++) {
      Serial.print(buf[i], HEX);
    }
    Serial.println("<");
    msgLen = 0;
    digitalWrite(LED_PIN, LOW);
  } 
  else {
    if (millis() % 2000 < 20)
      digitalWrite(LED_PIN, HIGH);
    else  
      digitalWrite(LED_PIN, LOW);
  }
}


//
// Interrupt routine for changes on j1850 data pin
//
void j1850_interrupt(void)
{
  uint8_t pin = digitalRead(J1850_PIN);
  long tstamp = micros();
  long delta = tstamp - lastInt;

  switch (state)
  {

  case idle:
    if (pin == 0 && abs(delta - SOF_TIME) < SOF_DEV)
    {
      // found SOF, start header sampling
      state = header;
      bitCnt = 0;
      msgLen = 0;
      bufIdx = 0;
    }
    break;

  case header:

    if (abs(delta - BITONE_TIME) < BITONE_DEV) {
      // found a high bit
      buf[0] = (buf[0] << 1) | 0x01;
      bitCnt++;

    } 
    else if (abs(delta - BITZERO_TIME) < BITZERO_DEV) {
      // found a low bit
      buf[0] = (buf[0] << 1);
      bitCnt++;

    } 
    else {
      // unknown bit, reset
      state = idle;
      break;
    }

    if (bitCnt == 8) {
      // 8 bits of header received
      bufIdx = 1;
      bitCnt = 0;
      state = data;
    }
    break;

  case data:

    if (abs(delta - BITONE_TIME) < BITONE_DEV) {
      // found a high bit
      buf[bufIdx] = (buf[bufIdx] << 1) | 0x01;
      bitCnt++;

    } 
    else if (abs(delta - BITZERO_TIME) < BITZERO_DEV) {
      // found a low bit
      buf[bufIdx] = (buf[bufIdx] << 1);
      bitCnt++;

    } 
    else if (abs(delta - EOF_TIME) < EOF_DEV) {
      // found EOF
      msgLen = bufIdx;
      state = idle;

    } 
    else {
      // unknown bit, reset
      state = idle;
    }

    if (bitCnt == 8) {
      // 8 bits of data byte received
      bufIdx++;
      bitCnt = 0;
      if (bufIdx >= sizeof(buf)) {
        // too many data bytes, error
        state = idle;
      }
    }
    break;

  };

  lastInt = tstamp;

}



