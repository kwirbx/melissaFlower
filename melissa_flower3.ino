


#include <Arduino.h>
#include "BasicStepperDriver.h"
#include "digitalbutton.h"
#include "PinChangeInt.h"

#define BUTTON_UP A2
#define BUTTON_DOWN A3
#define BUTTON_ENTER A4
#define MIC_PIN A0

#define MICROSTEPS 32
#define LIMIT_SWITCH 10
#define MAXSTEPS 4000

// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
#define STEPSHUND 90  //move .01
#define STEPSTENS 900
//move .1
#define STEPPER_SPEED 400
#define CUT_SPEED 200

// All the wires needed for full functionality
#define DIR 12
#define STEP 11
// Since microstepping is set externally, make sure this matches the selected mode
// 1=full step, 2=half step etc.

#define FPS 30
#define BUTTON_OFF_TIME 1100

// 2-wire basic config, microstepping is hardwired on the driver
BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);



DigitalButton btn_up = buttonCreate(BUTTON_UP);
DigitalButton btn_down = buttonCreate(BUTTON_DOWN);
DigitalButton btn_enter = buttonCreate(BUTTON_ENTER);


unsigned long last;

uint8_t tickRate = 0;

uint16_t lastStep = 0;
int16_t stepMove = 0;

volatile bool stopMove = false;

void onButtonUp(){

  stepper.move(20*MICROSTEPS);

}

void onButtonDown(){

  stepper.move(-20*MICROSTEPS);
  

  
}

void onButtonHoldUp(){
  stepper.move(1000);
stepper.move(4*MICROSTEPS);
}

void onButtonHoldDown(){
  stepper.move(-4*MICROSTEPS);
}



void homing()
{
  //supposedly stop homing
  while(stopMove != true)
  {
  stepper.move(MAXSTEPS);
  }
}

void onButtonEnter(){
  homing();
}


void stopHoming(){
  //detachPinChangeInterrupt(LIMIT_SWITCH);
  stepper.move(0);
}

void setStopMove(){
    stopMove = true;
}


void setup() {  
  //Serial.begin(9600);
  pinMode(LIMIT_SWITCH, INPUT_PULLUP);
 
  digitalWrite(LIMIT_SWITCH, HIGH);
  attachPinChangeInterrupt(LIMIT_SWITCH, setStopMove, RISING);

 stepper.begin(STEPPER_SPEED, MICROSTEPS);
 stepper.enable();
 
  last = millis();
  // calculate tick rate in ms
  tickRate = (1 / FPS) * 1000;
  
  

  homing();
  //stepper.move(-MAXSTEPS);

  buttonOnTap(btn_up, onButtonUp, 100);
  buttonOnTap(btn_down, onButtonDown, 100);
 // buttonOnHold(btn_up, onButtonHoldUp, BUTTON_OFF_TIME);
 // buttonOnHold(btn_down, onButtonHoldDown, BUTTON_OFF_TIME);
  buttonOnTap(btn_enter, onButtonEnter, 100);
  
  //buttonOnTap(lim_switch, onLimit, 100);
  //buttonOnHold(btn, onButtonHold, BUTTON_OFF_TIME);
  
}



void loop() {

     audioDraw();
    // if its time for the next tick, do
    unsigned long curr = millis();
    if(curr - last > tickRate){

         last = curr;

        // tick button too i guess
        digitalButtonTick(btn_up);
        digitalButtonTick(btn_down);
        digitalButtonTick(btn_enter);         
    }
}


     //pulse sort of
void audioDraw() {
    //don't have zones you have lots to play with
    //MAXSTEPS
    
    uint16_t stepPeak = 0;
    uint16_t currentStepPeak = 0;

    uint16_t sampleWindow = 200; // Sample window width in mS (50 mS = 20Hz)
    uint16_t sample;


    uint64_t startMillis= millis();  // Start of sample window
    uint16_t peakToPeak = 0;   // peak-to-peak level

    uint16_t signalMax = 0;
    uint16_t signalMin = 1024;

   //code i didn't write
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(MIC_PIN); 
      if (sample < 1024)  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;  // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  // save just the min levels
         }
      }
   }
   peakToPeak = signalMax - signalMin;



   // map 1v p-p level to the max scale of the display
   stepPeak = map(peakToPeak, 0, 100, 0, MAXSTEPS);
   currentStepPeak = constrain(stepPeak, 0, MAXSTEPS);

     //get them stepper paths based on the last level
     //you basically wanna hop levels instead of starting from 0 
     stepMove = lastStep - currentStepPeak;

     stepper.move(stepMove); 

    /*
     Serial.print(lastStep);
     Serial.print(" ");
     Serial.print(currentStepPeak);
     Serial.print(" ");
     Serial.println(stepMove);*/
     
     
   //  Serial.println("hi");
     lastStep= currentStepPeak ;

}


