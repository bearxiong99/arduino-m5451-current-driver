/*
 * Constant-Current Shield and M5451 LED driver
 * Version 1.0 Author: G. Andrew Stone
 * Created March, 2009
 * Released under the MIT license (http://www.opensource.org/licenses/mit-license.php)
 *
 * This is an Arduino Library, to install see: http://www.arduino.cc/en/Hacking/LibraryTutorial
 * Quick Installation instructions:
 * Copy this file and the CCShield.h file to a new directory called
 * "CCShield" in the hardware\libraries directory in your arduino installation.  
 * Then restart the Arduino IDE.
 *
 * For example: 
 * mkdir c:\arduino-0012\hardware\libraries\CCShield
 * copy *.* c:\arduino-0012\hardware\libraries\CCShield
 */
#include <avr/pgmspace.h>
#include "WProgram.h"

#include "wiring.h"
//extern void analogWrite(uint8_t,int);

#include "lightuinoSink.h"

#define M5451_CLK 1


void setbit(unsigned char offset,unsigned long int* a, unsigned long int* b, unsigned char* c)
{
  if (offset < 32) *a |= 1UL<<offset;
  else if (offset < 64) *b |= 1UL<<(offset-32);
  else if (offset < Lightuino_NUMOUTS) *c |= 1UL<<(offset-64);
}

void clearbit(unsigned char offset,unsigned long int* a, unsigned long int* b, unsigned char* c)
{
  if (offset < 32) *a &= ~(1UL<<offset);
  else if (offset < 64) *b &= ~(1UL<<(offset-32));
  else if (offset < Lightuino_NUMOUTS) *c &= ~(1UL<<(offset-64));
}

// Sets the brightness of ALL LEDs using the M5451 brightness control
void LightuinoSink::setBrightness(uint8_t b)
{
  if (brightPin<0xff)
    analogWrite(brightPin,b);
}



LightuinoSink::LightuinoSink(uint8_t clkPin, uint8_t dataPin1, uint8_t dataPin2, uint8_t brightnessPin)
{
  int i;
  flags = Lightuino_FASTSET;
  clockPin = clkPin;
  serDataPin[0] = dataPin1;
  serDataPin[1] = dataPin2;
  brightPin = brightnessPin;
  finishReq=false;
  
  pinMode(clkPin, OUTPUT);      // sets the digital pin as output
  pinMode(serDataPin[0], OUTPUT);      // sets the digital pin as output
  pinMode(serDataPin[1], OUTPUT);      // sets the digital pin as output
  pinMode(brightPin,OUTPUT);

  set(0UL,0UL,0UL);            // Clear out any random settings caused by power up
  analogWrite(brightPin,255);  // Turn brightness fully on by default -- helps initial adopters

#if 0
  // Clear out the device so we can clock in items
  digitalWrite(serDataPin[0],LOW);  
  digitalWrite(serDataPin[1],LOW);  
  for (i=0;i<M5451_NUMOUTS+2;i++)
    {
    mydelay(M5451_CLK);
    digitalWrite(clockPin,HIGH);
    mydelay(M5451_CLK);
    digitalWrite(clockPin,LOW);    
    }
#endif
}

int LightuinoSink::mydelay(unsigned long int clk)
{
  unsigned long int i;
  unsigned long int j=0;
  for (i=0;i<clk;i++) 
    {
      j+=3*i;
      i+=1;
      j-=i*j/7;
    }
  //delay(1);
  return j;
}

void LightuinoSink::set(unsigned long int a, unsigned long int b, unsigned long int c)
{
  unsigned long int data[3];
  data[0] = a; data[1]=b; data[2]=c;
  if (flags&Lightuino_FASTSET)
    {
    if (flags&Lightuino_BY32) fastSetBy32(data);
    else fastSet(data);
    }
  else safeSet(data);
}

void LightuinoSink::set(unsigned char* a)
{
  unsigned long int data[3];
  data[0] = *((unsigned long int*) a); 
  data[1]=*((unsigned long int*) (a+4)); 
  data[2]= a[8];
  if (flags&Lightuino_FASTSET)
    {
    if (flags&Lightuino_BY32) fastSetBy32(data);
    else fastSet(data);
    }
  else safeSet(data);
}


void LightuinoSink::set(unsigned long int a[3])
{
  if (flags&Lightuino_FASTSET) 
    {
    if (flags&Lightuino_BY32) fastSetBy32(a);
    else fastSet(a);
    }
  else safeSet(a);

}

void LightuinoSink::fastSet(unsigned long int a, unsigned long int b, unsigned long int c)
{
  unsigned long int data[3];
  data[0] = a; data[1]=b; data[2]=c;
  fastSet(data);
}

void LightuinoSink::fastSetBy32(unsigned long int left, unsigned long int right, unsigned long int overflow)
{
  unsigned long int data[3];
  data[0] = left; data[1]=right; data[2]=overflow;
  fastSetBy32(data);
}


void LightuinoSink::fastSet(unsigned long int a[3])
{
  uint8_t i;

  // Write the initial "start" signal
  digitalWrite(clockPin,LOW);
  digitalWrite(serDataPin[0],LOW);
  digitalWrite(serDataPin[1],LOW);

  // Remember the low values
  uint8_t dcurval  = PORTD;
  uint8_t bcurval = PORTB;

  //mydelay(M5451_CLK);
  digitalWrite(clockPin,HIGH);
  //mydelay(M5451_CLK);
  digitalWrite(clockPin,LOW);
  //mydelay(M5451_CLK);
  digitalWrite(serDataPin[0],HIGH);
  digitalWrite(serDataPin[1],HIGH);
  //mydelay(M5451_CLK);
  digitalWrite(clockPin,HIGH);

  uint8_t hiclk = PORTD;

  //mydelay(M5451_CLK);
  digitalWrite(clockPin,LOW);
 
  unsigned long int b[2];
  b[0] = (a[1]>>3 | (a[2] << 29));
  b[1] = (a[2]>>3);

#if 0
  if (serDataPin[0]<8) curval &= ~(1<<serDataPin[0]);
  if (serDataPin[1]<8) curval &= ~(1<<serDataPin[1]);
  if (serDataPin[0]>=8) bcurval &= ~(1<<(serDataPin[0]-8));
  if (serDataPin[1]>=8) bcurval &= ~(1<<(serDataPin[1]-8));
#endif  
  
  // Set up all possible values for PORTD
  uint8_t dpOn[4];
  dpOn[0] = dcurval;
  //digitalPinToBitMask(serDataPin[0]);
  if (serDataPin[0]<8) dpOn[1] = dcurval | (1<<serDataPin[0]);
  else dpOn[1] = dcurval;  
  if (serDataPin[1]<8) dpOn[2] = dcurval | (1<<serDataPin[1]);
  else dpOn[2] = dcurval;
  dpOn[3] = dcurval | (dpOn[1] | dpOn[2]);

  // Set up all possible values for PORTB
  uint8_t bpOn[4];
  bpOn[0] = bcurval;
  if (serDataPin[0]>=8) bpOn[1] = bcurval | (1<<(serDataPin[0]-8));
  else bpOn[1] = bcurval;
  if (serDataPin[1]>=8) bpOn[2] = bcurval | (1<<(serDataPin[1]-8));
  else bpOn[2] = bcurval;
  bpOn[3] = bcurval | (bpOn[1] | bpOn[2]);
  
  
  for (i=0;i<32;i+=2)
  {
    uint8_t lkup = (a[0]&1)+((b[0]&1)<<1);
    
    PORTD = dpOn[lkup];
    PORTB = bpOn[lkup];
    //mydelay(M5451_CLK);
    PORTD |= hiclk;
    //mydelay(M5451_CLK);
    lkup = ((a[0]&2)>>1)+(b[0]&2);
    
    PORTD = dpOn[lkup];
    PORTB = bpOn[lkup];
    //mydelay(M5451_CLK);
    PORTD |= hiclk;    
    a[0]>>=2; b[0]>>=2;   
  }
  
    for (i=0;i<M5451_NUMOUTS-33;i++)
  {
    uint8_t lkup = (a[1]&1)+((b[1]&1)<<1);    
    a[1]>>=1; b[1]>>=1;    
    PORTD = dpOn[lkup];
    PORTB = bpOn[lkup];
    //mydelay(M5451_CLK);
    PORTD |= hiclk; 
    //mydelay(M5451_CLK);
  }
  
  uint8_t lkup = (a[1]&1)+((b[1]&1)<<1);    
  a[1]>>=1; b[1]>>=1;    
  PORTD = dpOn[lkup];
  PORTB = bpOn[lkup];
    //mydelay(M5451_CLK);
  if (!finishReq) PORTD |= hiclk; 
    //mydelay(M5451_CLK);  
  
  
}

void LightuinoSink::finish()
{
  digitalWrite(clockPin,HIGH);
  //PORTD |= hiclk;
}

void LightuinoSink::fastSetBy32(unsigned long int input[3])
{
  uint8_t i;

  // Write the initial "start" signal
  digitalWrite(clockPin,LOW);
  digitalWrite(serDataPin[0],LOW);
  digitalWrite(serDataPin[1],LOW);

  // Remember the low values
  uint8_t dcurval  = PORTD;
  uint8_t bcurval = PORTB;

  //mydelay(M5451_CLK);
  digitalWrite(clockPin,HIGH);
  //mydelay(M5451_CLK);
  digitalWrite(clockPin,LOW);
  //mydelay(M5451_CLK);
  digitalWrite(serDataPin[0],HIGH);
  digitalWrite(serDataPin[1],HIGH);
  //mydelay(M5451_CLK);
  digitalWrite(clockPin,HIGH);

  uint8_t hiclk = PORTD;

  //mydelay(M5451_CLK);
  digitalWrite(clockPin,LOW);
 
  // Make "a" contain what should be programmed to one of the chips
  // and "b" contain what to program to the other chip 
  unsigned long int a[2];
  unsigned long int b[2];
  a[0] = input[0];
  b[0] = input[1];
  b[1] = input[2]>>3;
  a[1] = input[2];
  
  // Set up all possible values for PORTD
  uint8_t dpOn[4];
  dpOn[0] = dcurval;
  //digitalPinToBitMask(serDataPin[0]);
  if (serDataPin[0]<8) dpOn[1] = dcurval | (1<<serDataPin[0]);
  else dpOn[1] = dcurval;  
  if (serDataPin[1]<8) dpOn[2] = dcurval | (1<<serDataPin[1]);
  else dpOn[2] = dcurval;
  dpOn[3] = dcurval | (dpOn[1] | dpOn[2]);

  // Set up all possible values for PORTB
  uint8_t bpOn[4];
  bpOn[0] = bcurval;
  if (serDataPin[0]>=8) bpOn[1] = bcurval | (1<<(serDataPin[0]-8));
  else bpOn[1] = bcurval;
  if (serDataPin[1]>=8) bpOn[2] = bcurval | (1<<(serDataPin[1]-8));
  else bpOn[2] = bcurval;
  bpOn[3] = bcurval | (bpOn[1] | bpOn[2]);
  
  
  for (i=0;i<32;i+=2)
  {
    uint8_t lkup = (a[0]&1)+((b[0]&1)<<1);
    
    PORTD = dpOn[lkup];
    PORTB = bpOn[lkup];
    //mydelay(M5451_CLK);
    PORTD |= hiclk;
    //mydelay(M5451_CLK);
    lkup = ((a[0]&2)>>1)+(b[0]&2);
    
    PORTD = dpOn[lkup];
    PORTB = bpOn[lkup];
    //mydelay(M5451_CLK);
    PORTD |= hiclk;    
    a[0]>>=2; b[0]>>=2;   
  }
  
    for (i=0;i<M5451_NUMOUTS-32;i++)
  {
    uint8_t lkup = (a[1]&1)+((b[1]&1)<<1);    
    a[1]>>=1; b[1]>>=1;    
    PORTD = dpOn[lkup];
    PORTB = bpOn[lkup];
    //mydelay(M5451_CLK);
    PORTD |= hiclk; 
    //mydelay(M5451_CLK);
  }
}


void LightuinoSink::safeSet(unsigned long int a, unsigned long int b, unsigned long int c)
{
  unsigned long int data[3];
  data[0] = a; data[1]=b; data[2]=c;
  safeSet(data);
}

void LightuinoSink::safeSet(unsigned long int a[3])
{
  int i;

  // Write the initial "start" signal
  digitalWrite(clockPin,LOW);
  digitalWrite(serDataPin[0],LOW);
  digitalWrite(serDataPin[1],LOW);

  mydelay(M5451_CLK);
  digitalWrite(clockPin,HIGH);
  mydelay(M5451_CLK);
  digitalWrite(clockPin,LOW);
  mydelay(M5451_CLK/2);
  digitalWrite(serDataPin[0],HIGH);
  digitalWrite(serDataPin[1],HIGH);
  mydelay(M5451_CLK/2);
  digitalWrite(clockPin,HIGH);

  mydelay(M5451_CLK);
  digitalWrite(clockPin,LOW);
 
  unsigned long int d2[2];
  d2[0] = a[1]>>3 | (a[2] << 29);
  d2[1] = a[2]>>3;
  // Write the bits
    
  
  for (i=0;i<M5451_NUMOUTS;i++)
  {
    int serDataVal[2];
    if (i<32) { serDataVal[0] = (a[0]&1); a[0]>>=1;}
    else { serDataVal[0] = (a[1]&1); a[1]>>=1;}
    if (i<32) { serDataVal[1] = (d2[0]&1); d2[0]>>=1;}
    else { serDataVal[1] = (d2[1]&1); d2[1]>>=1;}
    
    mydelay(M5451_CLK/2);
    digitalWrite(serDataPin[0],serDataVal[0]);
    digitalWrite(serDataPin[1],serDataVal[1]);
    mydelay(M5451_CLK/2);
    digitalWrite(clockPin,HIGH);
    mydelay(M5451_CLK);
    digitalWrite(clockPin,LOW);
  }
}

void AniPattern::next(void)
{
  unsigned long int bits[3];

  bits[0] = pgm_read_dword_near(ani + curFrame);
  bits[1] = pgm_read_dword_near(ani + curFrame+4);
  bits[2] = pgm_read_byte_near(ani + curFrame+8);
  shield.set(bits);
  int d = pgm_read_word_near(delays + curDelay);
  delay(d);
      
  curFrame+=(anidir*9);
  curDelay+=anidir;

  if (curDelay >= numFrames) 
    { 
    if (flags&1) { anidir = -1; curDelay=numFrames-2; curFrame = (numFrames-2)*9; }
    else { curFrame=0; curDelay=0; }    
    }
  else if (curDelay < 0)
  {
    if (flags&1) { anidir = 1; curDelay=1; curFrame = 9; }
    else { curDelay=numFrames-1; curFrame=(numFrames-1)*9; }    
  }
}



  

