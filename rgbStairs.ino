#include <FastLED.h>
#include "stair.h"

Strip strip[2];
byte adr=10;
void setup() {
  Serial.begin(9600);
  
  strip[0].setPins(3,5,6);
  strip[1].setPins(9,10,11);
  
  pinMode(4,OUTPUT);
  digitalWrite(4,LOW);
  pinMode(8,OUTPUT);
  digitalWrite(8,LOW);
  strip[0].initOutput();
  strip[0].value.setRGB(10,10,10);
  strip[0].output();  

  strip[1].initOutput();
  strip[1].value.setRGB(10,10,10);
  strip[1].output();

  Wire.begin(adr);
  Wire.onReceive(receiveEvent);
  
  for(byte i =0;i<3;i++){Serial.print(strip[0].pin[i]);}
  Serial.println("");
  for(byte i =0;i<3;i++){Serial.print(strip[1].pin[i]);}
  Serial.println("");
  delay(2000);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()){
    byte val[3];
    byte ch = Serial.read()-48;
    delay(2);
    char com = Serial.read();
    delay(2);
    Serial.print(ch);Serial.print(",");
    Serial.print(com);Serial.print(",");
    if(com=='a'){ //rgb tester
      for(byte i=0; i<3; i++){
        val[i] = Serial.readStringUntil(',').toInt();
        delay(1);
        }
      unsigned int time = Serial.readStringUntil(',').toInt();
      
      strip[ch].fadeTo(val, time);
    }
    else if(com== 's'){ //second rgb tester 
      for(byte i=0; i<3; i++){
        val[i] = Serial.readStringUntil(',').toInt();
        delay(1);
        }
        unsigned int time = Serial.readStringUntil(',').toInt();
      strip[ch].fadeTo(val[0],val[1], val[2], time);
    }
    else if(com== 'l'){ //second rgb tester 
      for(byte i=0; i<3; i++){
        val[i] = Serial.readStringUntil(',').toInt();
        delay(1);
        }
        unsigned int time = Serial.readStringUntil(',').toInt();
      strip[ch].fadeLumaTo(val[0],val[1], val[2], time);
    }
    else if(com=='d'){ // single ch tester
      for(byte i=0; i<2; i++){
        val[i] = Serial.readStringUntil(',').toInt();
        delay(1);
        }
        unsigned int time = Serial.readStringUntil(',').toInt();
      strip[ch].fadeCh(val[0],val[1], time);
    }
    else if(com == 'c'){ //stop all
      for(byte i=0; i<3; i++)strip[0].updateCh[i]=&Strip::dummy2;
    }
    else if(com == 'l'){ //linear maping
      Serial.println("linear!");
      strip[ch].interpolate=&Strip::linear;
    }
    else if(com == 'n'){//approx cubic maping
      Serial.println("aprox!");
      strip[ch].interpolate=&Strip::Aprox;
    }
    else if(com == 'h'){//set hue
      for(byte i=0; i<3; i++){
        val[i] = Serial.readStringUntil(',').toInt();
        delay(1);
      }
      Serial.print(val[2]);
      Serial.print(", ");
      strip[ch].value.setHSV(val[0],val[1],val[2]);
      Serial.print(strip[0].value.getLuma());
      Serial.print(", ");
      Serial.println(strip[0].value.getAverageLight());
    }
    else if(com == 'j'){//fade hue
      for(byte i=0; i<3; i++){
        val[i] = Serial.readStringUntil(',').toInt();
        delay(1);
      }
      unsigned int time = Serial.readStringUntil(',').toInt();
      strip[ch].fadeToHSV(val[0],val[1],val[2],time);
    }
    else if(com == 'p'){//phazer
      for(byte i=0; i<3; i++){
        val[i] = Serial.readStringUntil(',').toInt();
        delay(1);
      }
      unsigned int time = Serial.readStringUntil(',').toInt();
      strip[ch].phaser(val[0],val[1],val[2],time);
      Serial.println(time);
    }
    Serial.readStringUntil('\n');
    for(byte i=0;i<3; i++){Serial.print(val[i]);Serial.print(",");}
    Serial.println();
  }
  strip[0].update();
  strip[1].update();
}

void receiveEvent(int howMany)
{
  while (1 < Wire.available()) // loop through all but the last
  {
    char c = Wire.read(); // receive byte as a character
    Serial.print(c);         // print the character
  }
  int x = Wire.read();    // receive byte as an integer
  Serial.println(x);         // print the integer
}
