//
// J1850 decoder, v1.2
// Mats Ekberg (c) 2013
// 
// This software decodes the binary data stream from a J1850 compliant 
// OBD2 device. Using just one digital input for the data stream and
// transmitting the decoded data on the UART.
//
// http://www.systemconnection.com/downloads/PDFs/OBD-IIJ1850Whitepaper.pdf
//
// Chip:  Arduino Mini Pro w ATmega168 at 16MHz
//


// I/O and interrupts. Pin/Int
// UNO:   2/0, 3/1. 
// Mega:  2/0, 3/1, 21/2, 20/3, 19/4, 18/5
#define J1850_PIN  3
#define J1850_INT  1

// this port is connected to the ordinary J1850 port
#define J1850_PIN2  9
#define J1850_BITS2  PINB
#define J1850_MASK2  _BV(1)

#define LED_PIN 13

#define DBG_PIN  7

// Timing for start of frame
#define SOF_TIME      200
#define SOF_DEV        18

// Timing for end of frame
#define EOF_TIME      200
#define EOF_DEV        18

// Timing for a long bit pulse
#define LONGBIT_TIME   128
#define LONGBIT_DEV     16

// Timing for short bit pulse
#define SHORTBIT_TIME   64
#define SHORTBIT_DEV    15

// timeout after 250 microsec
#define  TMR_PRELOAD (65536 - (EOF_TIME*16))

#define TMROVF_INT_OFF   TIMSK1 &= (~_BV(TOIE1)) 
#define TMROVF_INT_ON    TIMSK1 |= _BV(TOIE1) 
#define TMROVF_INT_CLR   TIFR1 &= _BV(TOV1)


// forward decl
void j1850_interrupt(void);

volatile boolean idle = true;
// Storage, max 11 data bytes + CRC
#define BUFSIZE 12
volatile uint8_t msgbuf[BUFSIZE];
volatile uint8_t msgLen;


//
// Initialization
//
void setup(void) 
{  
  pinMode(J1850_PIN, INPUT_PULLUP);
  //pinMode(J1850_PIN2, INPUT_PULLUP);

  pinMode(DBG_PIN, OUTPUT);
  digitalWrite(DBG_PIN, LOW);


  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  Serial.begin(38400);
  delay(2000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
  Serial.println(F("j1850decoder/v1.2"));

  TMROVF_INT_OFF;
  TCCR1A = 0;
  TCNT1 = TMR_PRELOAD;  // preload timer 65536-16MHz/256/2Hz
  TCCR1B = _BV(CS10);  // no prescaler, start timer 

  idle = true;
  msgLen = 0;
  attachInterrupt(J1850_INT, j1850_interrupt, CHANGE);
  interrupts();
}


//
// Background loop - print message when available
//
void loop(void)
{
  if (msgLen > 0) {
    /*
    digitalWrite(LED_PIN, HIGH);
     delay(20);
     digitalWrite(LED_PIN, LOW);
     */
    Serial.print(F(">"));
    for (int i = 0; i < msgLen; i++) {
      if (msgbuf[i] < 16) Serial.print("0");
      Serial.print(msgbuf[i], HEX);
    }
    Serial.println();
    msgLen = 0;
    digitalWrite(LED_PIN, LOW);
  }
}


//
// Interrupt routine for changes on j1850 data pin
//

volatile unsigned long lastInt = 0;
volatile uint8_t bitCnt;
volatile long delta;
volatile unsigned long tstamp;
volatile uint8_t aByte;
volatile uint8_t buf[BUFSIZE];
volatile uint8_t bufIdx;


void j1850_interrupt(void)
{
  tstamp = micros();

  //uint8_t pin = digitalRead(J1850_PIN);
  uint8_t pin = J1850_BITS2 & J1850_MASK2;

  // reload the overflow timer with EOF timeout
  TCNT1 = TMR_PRELOAD;           

  delta = tstamp - lastInt;
  long longbit, shortbit;

  if (idle)
  {
    if (pin == 0) 
    {
      longbit = delta - SOF_TIME;
      if (abs(longbit) < SOF_DEV)
      {
        // found SOF, start header/data sampling
        idle = false;
        bitCnt = 0;
        bufIdx = 0;
        aByte = 0;
        digitalWrite(LED_PIN, LOW);
      }
    }
  } 
  else
  {
    shortbit = delta - SHORTBIT_TIME;
    longbit = delta - LONGBIT_TIME;

    if (abs(shortbit) < SHORTBIT_DEV) {
      // short pulse
      if (pin == 0)
        // short pulse & pulse was high => active "1"
        aByte = (aByte << 1) | 0x01;
      else
        // short pulse & pulse was low => passive "0"
        aByte = (aByte << 1) & 0xFE;
      bitCnt++;

    } 
    else if (abs(longbit) < LONGBIT_DEV) {
      // long pulse
      if (pin == 0)
        // long pulse & pulse was high => active "0"
        aByte = (aByte << 1) & 0xFE;
      else
        // long pulse & pulse was low => passive "1"
        aByte = (aByte << 1) | 0x01;
      bitCnt++;

    } 
    else {
      // unknown bit, reset
      TMROVF_INT_OFF; 
      idle = true;
      lastInt = tstamp;
      return;
    }

    if (bitCnt >= 8) {

      buf[bufIdx++] = aByte;
      bitCnt = 0;
      if (bufIdx >= sizeof(buf)) {
        // too many data bytes, error
        TMROVF_INT_OFF; 
        idle = true;
      } 
      else {
        // if all is ok, start the EOF timeout
        TMROVF_INT_CLR; 
        TMROVF_INT_ON; 
      }
    }
  }
  lastInt = tstamp;
  digitalWrite(DBG_PIN, LOW);
}

//
// Timer overlflow interrupt
// Occurs when the EOF pulse times out the timer
//
ISR(TIMER1_OVF_vect)  
{
  TCNT1 = TMR_PRELOAD;  
  TMROVF_INT_OFF; 
  digitalWrite(LED_PIN, HIGH);

  // copy the data so that we can start to fill the buffer again
  // but only if the buffer has been consumed in the background
  if (bufIdx > 0 && msgLen == 0)
  {
    memcpy((void*)msgbuf, (const void*)buf, bufIdx);
    msgLen = bufIdx;
  }
  idle = true;
}





















