# include <U8glib.h>
#include <TimerOne.h>
// This code is working great lolololol
//**************************************************
// Change this constructor to match your display!!!
U8GLIB_SH1106_128X64 u8g(13, 11, 10, 9, 8);
//**************************************************

#define NO_PRESS 0
#define SHORT_PRESS 1 
#define LONG_PRESS 2
#define HALL_SENSOR_THRESHOLD 600
typedef enum enum_deviceStatus { 
  START = 10,
  IDLE = 11,
  SCREEN_ON = 12,
  CHECK_SAFETY = 13,
  READY = 14,
  RUN = 15,
  CANCEL = 16,
  DONE = 17, 
  NOT_SAFE_STATE = 18
}deviceStatus;

deviceStatus state = START; 

double timePressed = 0, timeReleased = 0;
int switchLED = 6;
int switchPin = 3;
int ledPin = 7;
int hallSensor = A2;
int switchPin_Dig = 3; 
int isStarted = 0;
int lightIntensity = 0;
volatile bool onState = false;
long minsElapsed = 0;
long secElapsed = 0;


void setup() {
  Serial.begin(9600);
  pinMode(switchLED, OUTPUT);
  pinMode(switchPin, INPUT);
  pinMode(switchPin_Dig, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(hallSensor, INPUT);
  attachInterrupt(digitalPinToInterrupt(switchPin_Dig), toggle, HIGH);
  Timer1.initialize(100000); 
  u8g.setFont(u8g_font_unifont);
  u8g.setColorIndex(1); // Instructs the display to draw with a pixel on. 
}
void loop() {

    switch(state){
    case START:        break;
    case IDLE:         if(ButtonPress() == SHORT_PRESS) state = CHECK_SAFETY;
                       break;
    case CHECK_SAFETY: if (analogRead(hallSensor)>550)
                         state = SCREEN_ON;
                       else
                         state = NOT_SAFE_STATE;
                         break;
    case NOT_SAFE_STATE: break;
    case SCREEN_ON:  break;
    case READY:      if (ButtonPress() == SHORT_PRESS)
                        state = RUN;
                     else if (ButtonPress() == LONG_PRESS)
                        state = CANCEL; 
                     break;
    case RUN:        if (ButtonPress() == LONG_PRESS) state = CANCEL; 
                     else if(checktime()) state = DONE; break;  // if time >= time_set
    case CANCEL:     if (ButtonPress() == SHORT_PRESS)  state = IDLE; break;
    case DONE:       if (ButtonPress() == LONG_PRESS)  state = IDLE; break;
    default:         state = IDLE;
  }
  if (state == START)
  {
     digitalWrite(ledPin, LOW);
     u8g.firstPage();
      do {  
        wakeUp();
      } while( u8g.nextPage() );
      delay(10);
    for (lightIntensity; lightIntensity < 250; lightIntensity++) {
        if(state == START){
        analogWrite(switchLED, lightIntensity);
        delay(5);
        }
        else break;
      }
    for (lightIntensity; lightIntensity > 10; lightIntensity = lightIntensity - 3) {
      if(state == START){
      analogWrite(switchLED, lightIntensity);
      delay(10);
      }
      else break;
      }
     
      
  }
  else if (state == IDLE)
  {
    lightIntensity = 0;   
    analogWrite(switchLED, lightIntensity);
    digitalWrite(ledPin, LOW);    
    Serial.print("The state is : IDLE\n"); 
    u8g.firstPage();
    do {  
      draw_idle();
    } while( u8g.nextPage() );
    delay(10);  
  }
  else if (state == NOT_SAFE_STATE)
  {
    digitalWrite(ledPin, LOW);
    u8g.firstPage();
    do {  
      warningMessage();
    } while( u8g.nextPage() );
    delay(3000);
    state = IDLE;
  }
  else if (state == SCREEN_ON){
    lightIntensity = 0;   
    analogWrite(switchLED, lightIntensity);
    // TODO: set everything to not work
    u8g.firstPage();
    do {  
      draw_on();
    } while( u8g.nextPage() );
    delay(1000); 
    state = READY;
  }
  else if(state == READY)
  {
    digitalWrite(ledPin, LOW);
    u8g.firstPage();
    do {  
      draw_ready();
    } while( u8g.nextPage() );
    delay(5); 
  }
  else if (state == RUN){
    lightIntensity = 0;   
    Timer1.attachInterrupt(checkSafety); 

    analogWrite(switchLED, lightIntensity);
    Serial.print(analogRead(hallSensor));
    Serial.print("\n");
    // TODO: set everything running
    digitalWrite(ledPin, HIGH);
    u8g.firstPage();
    do {  
      draw_run();
    } while( u8g.nextPage() );
    delay(10);      
  }

  else if (state == CANCEL){
    lightIntensity = 0;   
    analogWrite(switchLED, lightIntensity);
    // TODO: set cancel mode
      digitalWrite(ledPin, LOW);
      Serial.print("The state is : CANCEL\n"); 
      u8g.firstPage();
      do {  
        draw_cancel();
      } while( u8g.nextPage() );
      delay(10);
  }
  else if (state == DONE){
    lightIntensity = 0;   
    analogWrite(switchLED, lightIntensity);
    // TODO: set done mode
     Serial.print("The state is : DONE\n"); 
  }
  else
  {
    Serial.print("The state is : UNKNOWN\n"); 
  }
}

int ButtonPress(){
  timePressed = millis();
  while(analogRead(switchPin) > 1000)
  {
     if (millis() - timePressed > 750)
      return LONG_PRESS;
  }
  timeReleased = millis();
  if (timeReleased - timePressed > 750)
    return LONG_PRESS;
  else if (timeReleased - timePressed < 10)
    return NO_PRESS;
  else
    return SHORT_PRESS;
  }
bool checktime()
{
  if(minsElapsed >= 10)
    return true;
  else 
    return false;
}
void draw_run(){
  u8g.drawCircle(64, 32, 30);
  u8g.drawStr( 37, 37, "Running");
  u8g.setRot180();
}
void draw_idle(){
  u8g.drawCircle(64, 32, 30);
  u8g.drawStr( 45, 37, "IDLE");
  u8g.setRot180();
  
}
void draw_on(){
 // u8g.drawCircle(64, 32, 30);
  u8g.drawStr( 40, 37, "Launching");
//  u8g.drawStr( 0, 50, "Press to Start");
  u8g.setRot180();
}
void draw_ready(){
  u8g.drawCircle(64, 32, 30);
  u8g.drawStr( 40, 37, "Ready");
//  u8g.drawStr( 0, 50, "Press to Start");
  u8g.setRot180();
}
void draw_cancel(){
  u8g.drawCircle(64, 32, 30);
  u8g.drawStr( 37, 37, "Cancel");
  u8g.setRot180();
}
void sleep(){
  u8g.sleepOn();
}
void wakeUp(){
  u8g.sleepOff();
  u8g.drawStr( 40, 37, "Start");
  u8g.drawCircle(64, 32, 30);
  u8g.setRot180();
}

void warningMessage()
{
 u8g.drawStr( 10, 37, "Close the Lid"); 
 u8g.setRot180();
}
void toggle ()
{ 
  Serial.print("Toggle triggered \n ");    
  if(state == START)
  {
    state = IDLE;
  }
  detachInterrupt(digitalPinToInterrupt(switchPin_Dig));
}
void checkSafety()
{
  if(analogRead(hallSensor)<HALL_SENSOR_THRESHOLD)
  {
    state = NOT_SAFE_STATE;
    Timer1.detachInterrupt();
  }
}
