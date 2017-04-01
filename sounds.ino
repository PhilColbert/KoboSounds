
#include "ESP8266WiFi.h"
extern "C" {
#include <user_interface.h>
#include "Esp.h"
}


/* ==== Defines ==== */
#define SERIAL_BAUD 57600
#define GFPALARM_LENGTH 200
#define GFPALARM_BASE_FREQUENCY 200
#define GFPALARM_SCALE 13
/* ==== END Defines ==== */


int speakerPin = 5; // D1


bool validSoundLine=false;
int incomingByte = 0;
char incomingChar;
String out;
int tuneIs[20][2]={0};
int noTones=0;
int currentTone=0;

void play_welcome_beep()                 //play only once welcome beep after turning on arduino vario
{
  ///for (int i=300; i<=800; i=i+100)
 // {
//    tone(speakerPin, i,200);
//    delay(75);
 // }

   tone(speakerPin, 300,500);
   delay(500);
   tone(speakerPin, 400,100);
   delay(100);
   tone(speakerPin, 700,100);
   delay(100);
}


bool isItNumeric ( String testString )
{

    for ( int i=0;i<testString.length();i++)
    {
      if (testString.charAt(i)=='0' || 
          testString.charAt(i)=='1' ||
          testString.charAt(i)=='2' ||
          testString.charAt(i)=='3' ||
          testString.charAt(i)=='4' ||
          testString.charAt(i)=='5' ||
          testString.charAt(i)=='6' ||
          testString.charAt(i)=='7' ||
          testString.charAt(i)=='8' ||
          testString.charAt(i)=='9')
      {
      }
      else
      {
        return false;
      }
    }
  return true;  
}

void setup()
{
  Serial.begin(SERIAL_BAUD);
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  wifi_set_sleep_type(MODEM_SLEEP_T);
  play_welcome_beep();
}


void loop()
{


// Serial.println("tone");
// tone(speakerPin, 300,1000);
 
 if (currentTone>=noTones)
 {

   validSoundLine=false;
  
   if (  millis() > tuneIs[currentTone][1])
   {
    // Serial.println("tone off");
     tuneIs[currentTone][1]=0;
     noTones=0;
     currentTone=0;
     noTone(speakerPin);
     validSoundLine=false;
   }
 }

 if ( noTones>0 && validSoundLine)
 {
  if ( millis()>tuneIs[currentTone][1])
  {
    currentTone++;
    tone(speakerPin, tuneIs[currentTone][0]);
   }
 }
 
 if (Serial.available() > 0 )
 {
   incomingChar = Serial.read();
     
   if ( incomingChar == '\n' )
   {

     String line=out.substring(0,out.indexOf("*"));

     if ( line.substring(0,9)=="$GFPALARM" )
     {

       if (isItNumeric(line.substring(10)))
       {
         int toneNumber=line.substring(10).toInt();
         validSoundLine=true;
         toneNumber=(toneNumber*GFPALARM_SCALE)+GFPALARM_BASE_FREQUENCY;

         tuneIs[0][0]=toneNumber;
         tuneIs[0][1]=GFPALARM_LENGTH+millis(); 
      // start playing the first.
         tone(speakerPin, tuneIs[0][0]);
         currentTone=0;
         noTones=0; 
       }
       else
       {
        validSoundLine=false;
       }
       
     }
     if (line.substring(0,4)=="$BSD")
     {

       String bsdLine=line.substring(5);

       Serial.println(bsdLine);
       int currentChar=0;

       int tuneLength=0;
       int maxTones=10;

       int toneReading=0;
       int spaceIndex=0;

       validSoundLine=true;
       
       while( spaceIndex>=0 && currentTone<maxTones && validSoundLine)
       {
        
         spaceIndex=bsdLine.indexOf(" ",currentChar);

         String currS=bsdLine.substring(currentChar,spaceIndex);

         if (isItNumeric(currS))
         {
           // time to play tone
           if ( toneReading == 1 )
           {
             tuneIs[currentTone][toneReading]=tuneLength+currS.toInt()+millis(); 
             tuneLength=tuneLength+currS.toInt();
           }
           else
           {
            tuneIs[currentTone][toneReading]=currS.toInt();
           }
         
           currentChar=spaceIndex+1;
         
           if (toneReading==0)
           {
             toneReading=1;
             noTones++;
          }
           else
           {
             toneReading=0;
             currentTone++;
           }
         }
         else
         {
           validSoundLine=false;
         }
       }

      if (validSoundLine)
      {
      // start playing first tone and then check at top of loop for the next tone to change to.
        tone(speakerPin, tuneIs[0][0]);
     
        currentTone=0;
      // playing the first.
        noTones--;
      }
    }
     
     out="";
   }
   else
   {
     out=out+String(incomingChar);
   }
 }

 yield();
 // delay in to stop the board looping too fast and crashing.
 delay(5);
}

