#include "lightuinoSourceDriver.h"
#include "wiring.h"

unsigned int reverse16bits(unsigned int x) {
 unsigned int h = 0;
 unsigned char i = 0;

 for(h = i = 0; i < 16; i++) {
  h = (h << 1) + (x & 1); 
  x >>= 1; 
 }

 return h;
}

LightuinoSourceDriver::LightuinoSourceDriver(unsigned char pclkPin, unsigned char pdataPin, unsigned char pstrobePin, unsigned char penaPin)
{
  clkPin    = pclkPin;  
  dataPin   = pdataPin;  
  strobePin =  pstrobePin,  
  enaPin    = penaPin; 

  pinMode(clkPin, OUTPUT);      // sets the digital pin as output
  pinMode(dataPin, OUTPUT);     // sets the digital pin as output
  pinMode(strobePin, OUTPUT);   // sets the digital pin as output
  pinMode(enaPin, OUTPUT);      // sets the digital pin as output

  digitalWrite(strobePin,HIGH);
  digitalWrite(enaPin,LOW);    // ON
}

void LightuinoSourceDriver::set(unsigned int bits)
{
  bits = reverse16bits(bits);
  digitalWrite(clkPin, LOW);
  digitalWrite(strobePin,LOW);
  for (char i=0;i<16;i++,bits>>=1)
    {
      digitalWrite(dataPin, bits&1);
      digitalWrite(clkPin, HIGH);
      digitalWrite(clkPin, LOW);      
    }

  //digitalWrite(enaPin,HIGH);    // OFF
  digitalWrite(strobePin,HIGH);
  //digitalWrite(enaPin,LOW);    // ON
}

void LightuinoSourceDriver::off()
{
  digitalWrite(enaPin,HIGH);    // OFF
}

void LightuinoSourceDriver::on()
{
  digitalWrite(enaPin,LOW);    // OFF
}


void LightuinoSourceDriver::shift(unsigned char bit)
{
  digitalWrite(clkPin, LOW);
  digitalWrite(strobePin,LOW);

  digitalWrite(dataPin, bit&1);
  digitalWrite(clkPin, HIGH);
  digitalWrite(clkPin, LOW);      

  //digitalWrite(enaPin,HIGH);    // OFF
  digitalWrite(strobePin,HIGH);
  //digitalWrite(enaPin,LOW);    // ON
}
