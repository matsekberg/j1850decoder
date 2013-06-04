//
// J1850 decoder, v1
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

#define HDR_PIN  9
#define DATA_PIN 8
#define DBG_PIN  7

// Timing for start of frame
#define SOF_TIME      200
#define SOF_DEV        30 //18

// Timing for end of frame
#define EOF_TIME      200
#define EOF_DEV        18

// Timing for a one-bit
#define BITONE_TIME   128
#define BITONE_DEV     25 //16

// Timing for a zero-bit
#define BITZERO_TIME   64
#define BITZERO_DEV    20 //15

// timeout after 250 microsec
#define  TMR_PRELOAD 65535 - 250

// forward decl
void j1850_interrupt(void);
enum state_t {
  idle, header, data};

// Storage
volatile uint8_t buf[15];
volatile uint8_t bufIdx;
volatile uint8_t msgLen = 0;

volatile unsigned long lastInt = 0;
volatile uint8_t bitCnt;
volatile state_t state;
volatile long delta;
volatile unsigned long tstamp;
volatile uint8_t aByte;

//
// Initialization
//
void setup(void) 
{  
  pinMode(J1850_PIN, INPUT_PULLUP);
  pinMode(HDR_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(DBG_PIN, OUTPUT);
  digitalWrite(HDR_PIN, LOW);
  digitalWrite(DATA_PIN, LOW);
  digitalWrite(DBG_PIN, LOW);


  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  delay(2000);
  digitalWrite(LED_PIN, LOW);
  Serial.begin(115200);
  Serial.println(F("j1850.v1"));

  state = idle;

  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = TMR_PRELOAD;   // preload timer 

  attachInterrupt(J1850_INT, j1850_interrupt, CHANGE);
}


//
// Background loop - print message when available
//
void loop(void)
{
  if (msgLen > 0) {
    TCCR1B &= 0xF8;      // no clock source, kill timer

    //noInterrupts();
    //digitalWrite(LED_PIN, HIGH);
    Serial.print(F(">"));
    for (int i = 0; i < msgLen; i++) {
      Serial.print(buf[i], HEX);
      Serial.print(F(","));
    }
    Serial.println(F("<"));
    msgLen = 0;
    //interrupts();
    //digitalWrite(LED_PIN, LOW);
  }
  /*
  Serial.print(state, DEC);
   Serial.print(bitCnt, DEC);
   Serial.println(bufIdx, DEC);
   */

  /* noInterrupts(); 
   if (delta > 160 && delta < 400)
   Serial.println(abs(delta - SOF_TIME), DEC);
   interrupts();
   
   else {
   if (millis() % 2000 < 20)
   digitalWrite(LED_PIN, HIGH);
   else  
   digitalWrite(LED_PIN, LOW);
   }
   */
}


//
// Interrupt routine for changes on j1850 data pin
//
void j1850_interrupt(void)
{
  tstamp = micros();
  if (msgLen == 0) {

    uint8_t pin = digitalRead(J1850_PIN);
    TCNT1 = TMR_PRELOAD;            // preload timer 65536-16MHz/256/2Hz
    TCCR1B |= (1 << CS10);    // no prescaler, start timer 
    TIMSK1 |= (1 << TOIE1); 

    digitalWrite(LED_PIN, LOW);

    delta = tstamp - lastInt;
    long err1, err2, err3;

    switch (state)
    {

    case idle:

      err1 = delta - SOF_TIME;
      if (pin == LOW && abs(err1) < SOF_DEV)
      {
        // found SOF, start header sampling
        state = header;
        bitCnt = 0;
        msgLen = 0;
        bufIdx = 0;
        aByte = 0;
      }
      break;

    case header:

      err1 = delta - BITONE_TIME;
      err2 = delta - BITZERO_TIME;
      if (abs(err1) < BITONE_DEV) {
        // found a high bit
        aByte = (aByte << 1) | 0x01;
        bitCnt++;

      } 
      else if (abs(err2) < BITZERO_DEV) {
        // found a low bit
        aByte = (aByte << 1) & 0xFE;
        bitCnt++;
      } 
      else {

        // unknown bit, reset
        state = idle;
        break;
      }

      if (bitCnt >= 8) {
        // 8 bits of header received
        buf[bufIdx++] = aByte;
        aByte = 0;
        bitCnt = 0;
        state = data;
      }
      break;

    case data:
      err1 = delta - BITONE_TIME;
      err2 = delta - BITZERO_TIME;
      err3 = delta - EOF_TIME;

      if (abs(err1) < BITONE_DEV) {
        // found a high bit
        aByte = (aByte << 1) | 0x01;
        bitCnt++;

      } 
      else if (abs(err2) < BITZERO_DEV) {
        // found a low bit
        aByte = (aByte << 1) & 0xFE;
        bitCnt++;

      } 
      else {
        // unknown bit, reset
        state = idle;
        break;
      }

      if (bitCnt >= 8) {
        // 8 bits of data byte received
        buf[bufIdx++] = aByte;
        bitCnt = 0;
        if (bufIdx >= sizeof(buf)) {
          // too many data bytes, error
          state = idle;
        }
      }
      break;

    };

  }
  lastInt = tstamp;
  digitalWrite(DBG_PIN, LOW);
}

// interrupt service routine that wraps a user defined function supplied by attachInterrupt
ISR(TIMER1_OVF_vect)  
{
  noInterrupts();
  TCNT1 = TMR_PRELOAD;   // preload timer 
  TCCR1B &= 0xF8;      // no clock source, kill timer
  if (state == data) {
    msgLen = bufIdx;
    state = idle;
    digitalWrite(LED_PIN, HIGH);
  }
  interrupts();

}










