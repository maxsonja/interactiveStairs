#include "Arduino.h"
//---------------------------
struct vectRGB {
  union {
    byte red;
    byte r;
  };
  union {
    byte green;
    byte g;
  };
  union {
    byte blue;
    byte b;
  };
  inline byte operator[](byte i)
  {
    switch (i)
    {
      default :
      case 0:
        return this->red;
      case 1:
        return this->green;
      case 2:
        return this->blue;
    }
  }
};

struct chInfo {
  byte startVal;
  byte endVal;
  unsigned int phaseTime;
  bool phase = false;
  union {
    unsigned long startTime;
    unsigned long lastTime;
  };
  union {
    unsigned int duration;
    unsigned int time;
  };
};
struct adjInfo {
  byte adj=255;
  byte start;
  byte end;
  byte startTime;
  byte duration;
};

//---init stuff ---------
class Strip {
  typedef bool(Strip::*funcPoint)(byte i);
  typedef fract8(Strip::*interpolateFunc)(byte& i);
  
  public:
    CRGB value;
    CRGB pin;
    CRGB tweak;
    CHSV hsv;
    adjInfo luma[3];    
    chInfo info[6];
    funcPoint updateCh[6];
    interpolateFunc interpolate;    

    Strip();
    Strip(byte pR, byte pG, byte pB);
    void setTweak(byte tR, byte tG, byte tB);
    void setPins(byte pR, byte pG, byte pB);
    void setPins(int pR, int pG, int pB);
    void initOutput();
    void output();
    void update();
    bool dummy(byte);
    bool dummy2(byte i);

    fract8 linear(byte& i);
    fract8 Aprox(byte& i);

    unsigned long deltaT(unsigned long& start);
    
    void fadeTo(byte val[], unsigned int time);
    void fadeTo(byte red, byte green, byte blue, unsigned int time);
    void fadeToHSV(byte val[], unsigned long time);
    void fadeToHSV(byte hue, byte sat, byte val, unsigned long time);
    void fadeLumaTo(byte lumaR, byte lumaG, byte lumaB, unsigned int time);
    void fadeLumaTo(byte luma, unsigned int time);
    void fadeCh(byte ch, byte val, unsigned int time);
    bool fadeUpdate(byte ch);
    void phaser(byte startHue, byte sat, byte val, unsigned int time );
    void phaserRaw(byte startHue, byte sat, byte val, unsigned int time );
    bool phaserUpdate(byte ch);
    bool phaserCal(byte ch);
    
    

};

Strip::Strip()
{
  value.setRGB(0,0,0);
  interpolate = &Strip::Aprox;
  for(byte i=3; i<6;i++)value[i]=255;
  for(byte i=0; i<6;i++)updateCh[i]=&Strip::dummy;
}
Strip::Strip(byte pR, byte pG, byte pB)
{
  value.setRGB(0,0,0);
  setPins(pR, pG, pB);
  initOutput();
  interpolate = &Strip::Aprox;
  for(byte i=3; i<6;i++)value[i]=255;
  for(byte i=0; i<6; i++){updateCh[i]=&Strip::dummy;}
}
void Strip::setTweak(byte tR, byte tG, byte tB)
{
  this->tweak.r = tR;
  this->tweak.g = tG;
  this->tweak.b = tB;
}
void Strip::setPins(byte pR, byte pG, byte pB)
{
  this->pin.r = pR;
  this->pin.g = pG;
  this->pin.b = pB;
}
void Strip::setPins(int pR, int pG, int pB)
{
  this->pin.r = byte(pR);
  this->pin.g = byte(pG);
  this->pin.b = byte(pB);
}
void Strip::initOutput(){
  for(byte i =0; i<3; i++) pinMode(pin[i], OUTPUT);
}

void Strip::output(){
  for(byte i =0; i<3; i++) analogWrite(pin[i], map(value[i], 0,255,0,value[i+3] ));
}

void Strip::update(){
  for(byte i=0; i<6; i++){(this->*this->updateCh[i])(i);}
  output();
}

bool Strip::dummy(byte i){}
bool Strip::dummy2(byte i){
  Serial.println("dummy2!");
  updateCh[i]=&Strip::dummy;
}
//-------controll stuff ----
unsigned long Strip::deltaT(unsigned long& start)
{
  unsigned long time = millis();
  if(start<=time){
    return time-start;
  }
  else {
    return (0xffffffff-start)+time;
  }
}

void Strip::fadeTo(byte val[], unsigned int time)
{
  for(byte i=0;i<3;i++){
    fadeCh(i, val[i],time);
  }
}

void Strip::fadeTo(byte red, byte green, byte blue, unsigned int time)
{
  fadeCh(0, red, time);
  fadeCh(1, green, time);
  fadeCh(2, blue, time); 
}

void Strip::fadeToHSV(byte hue, byte sat, byte val, unsigned long time)
{
  CRGB tmp = CHSV(hue,sat,val);
  fadeCh(0, tmp.red, time);
  fadeCh(1, tmp.green, time);
  fadeCh(2, tmp.blue, time);
}
void Strip::fadeToHSV(byte val[], unsigned long time)
{
  CRGB tmp = CHSV(val[0],val[1],val[2]);
  fadeCh(0, tmp.red, time);
  fadeCh(1, tmp.green, time);
  fadeCh(2, tmp.blue, time);
}

void Strip::fadeLumaTo(byte lumaR, byte lumaG, byte lumaB, unsigned int time)
{
  fadeCh(3, lumaR, time);
  fadeCh(4, lumaG, time);
  fadeCh(5, lumaB, time);
}

void Strip::fadeLumaTo(byte luma, unsigned int time)
{
  for(byte i=3;i<6;i++)fadeCh(i, luma, time);
}

void Strip::fadeCh(byte ch, byte val, unsigned int time)
{
  info[ch].startTime = millis();
  info[ch].startVal = value[ch];
  info[ch].endVal=val;
  info[ch].duration=time;
  updateCh[ch]=&Strip::fadeUpdate;
}

bool Strip::fadeUpdate(byte ch)
{
  unsigned long time = deltaT(info[ch].startTime);
  if(time>info[ch].duration) time = info[ch].duration;
  fract8 position = map(time, 0, info[ch].duration, 0, 0xff );
  position = (this->*this->interpolate)(position);
  value[ch]=map(position,0,0xff,info[ch].startVal,info[ch].endVal);
  if(value[ch] == info[ch].endVal){updateCh[ch] = &Strip::dummy; return true;}
  return false;
}

void Strip::phaser(byte startHue, byte sat, byte val, unsigned int time )
{
  info[0].phaseTime=time;
  info[0].phase=false;
  hsv.setHSV(startHue,sat,val);
  CRGB tmp;
  tmp.setHSV(startHue,sat,val);
  for(byte i=0; i<3;i++){
    info[i].startTime = millis();
    info[i].startVal = value[i];
    info[i].endVal=tmp[i];
    info[i].duration=1000;
  }
  updateCh[0]=&Strip::phaserUpdate;
  updateCh[1]=&Strip::dummy;
  updateCh[2]=&Strip::dummy;
  
}

void Strip::phaserRaw(byte startHue, byte sat, byte val, unsigned int time )
{
  info[0].phaseTime=time;
  hsv.setHSV(startHue,sat,val);
  for(byte i=0; i<3;i++){
    info[i].startTime = millis();
    info[i].startVal = value[i];
  }
  updateCh[0]=&Strip::phaserCal;
  updateCh[1]=&Strip::dummy;
  updateCh[2]=&Strip::dummy;
  
}
bool Strip::phaserUpdate(byte ch)
{
  if(!info[0].phase){
    info[0].phase = (this->fadeUpdate)(0);
    (this->fadeUpdate)(1);
    (this->fadeUpdate)(2);   
    if(info[0].phase){
      info[0].startTime = millis();
      updateCh[0]=&Strip::phaserUpdate;
    }
  }
  else{
    (this->phaserCal)(0);
  }
}

bool Strip::phaserCal(byte ch)
{
  unsigned long dTime = deltaT(info[0].startTime);
  if(dTime>info[0].phaseTime){
    while(dTime>info[0].phaseTime)dTime=dTime-info[0].phaseTime;
    info[0].startTime = millis() - dTime;
  }
  byte hue = map(dTime, 0,info[0].phaseTime,0,255);
  hue+=hsv.hue;
  value.setHSV(hue,hsv.sat,hsv.val);
}

fract8 Strip::linear(byte& i){
  return i;
}
fract8 Strip::Aprox(byte& i){
  i=ease8InOutApprox(i);
  return i;
}
//---------------------------
