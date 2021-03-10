// MIXMASTER FIRMWARE version 1.0.0

// ### INCLUDES ###
#include <Wire.h>
#include <AccelStepper.h> // AccelStepper for Motors
#include <LiquidCrystal_I2C.h> // LCD
#include <Rotary.h> //Rotary Encoder
#include <Adafruit_NeoPixel.h> // Pixel Ring
#ifdef __AVR__
  #include <avr/power.h>
#endif

// ### DEFINES ###
#define MotorInterfaceType 4 // Define the AccelStepper interface type:


//variables for 16 channel relay
const int COCACOLA = 22;
const int SPRITE = 23;
const int relayPins[] = {COCACOLA, SPRITE};

//variables for motors 
AccelStepper xStepper = AccelStepper(MotorInterfaceType, 8, 9, 10, 11); // Initialize  a new instance of AccelStepper on pins 8 through 11:
AccelStepper yStepper = AccelStepper(MotorInterfaceType, 17, 16, 15, 14);
AccelStepper zStepper = AccelStepper(MotorInterfaceType, 4, 5, 6, 7); 

MultiStepper stepperController; //Steppers controller

#define PIN 24
#define NUMPIXELS 24
LiquidCrystal_I2C lcd(0x27, 20, 4);  // LCD

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800); //pixel ring

//end stops
const int xLeft = 52;
const int xRight = 54;
volatile boolean isHome = false;

//Rotary Encoder = Rotary(DT,CHK);
const int RE_DT = A1;
const int RE_CHK = A0;
const int RE_SW = A2;
Rotary r = Rotary(RE_DT,RE_CHK);
volatile int pos = 0;   //index for drink array
unsigned char result;   //knob turn direction

char* drinkItems[] = { "Rum and Coke","Whiskey and Coke","Screwdriver" };
int numberOfItems = 3;

void setup() {
  Serial.begin(9600);
  
  xStepper.setMaxSpeed(1000);
  yStepper.setMaxSpeed(500);
  
  zStepper.setMaxSpeed(100);  // Set the maximum steps per second
  zStepper.setAcceleration(100); // Set the maximum acceleration in steps per second^2

  stepperController.addStepper(xStepper);
  stepperController.addStepper(yStepper);
  
  
  for (int i = 0; i < 2 ; i ++){ //set up the relay pins
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH);
  }

  //set up limit switches
  pinMode(xLeft, INPUT_PULLUP);
  pinMode(xRight, INPUT_PULLUP);

  //read pin for rotary encoder
  pinMode(RE_SW, INPUT);

  //setup for pixel ring
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
  #endif
  pixels.begin();
  pixels.setBrightness(1); //max 255
  pixels.clear();
  
  //setup for LCD
  lcd.init(); 
  lcd.backlight();
  lcd.clear();
  printWelcomeMessage();
  
  //more pixel ring startup routuine
  runPixelStartup();
  flashOnce(0,0,255);
  flashOnce(0,0,255);
  flashOnce(0,0,255);
  setColor(0, 0, 255);
  

  //print menu
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Select a drink:");
  delay(1000);
  lcd.setCursor(3, 2);
  lcd.print(drinkItems[pos]);


}


void loop() { 
  
    result = r.process();
    if (digitalRead(RE_SW) == LOW) {
      //if (!isHome) { goHome(); }
      makeDrink();
    }
    
    if (result) {
      switch (result) {
        case DIR_CW:          
             if (pos < numberOfItems - 1) { //sets a scroll limit
              pos += 1;
             }
             else {
              pos = 0;
             }
             lcd.clear();
             lcd.setCursor(3, 0);
             lcd.print("SELECT A DRINK");
             lcd.setCursor(3 ,2);
             lcd.print(drinkItems[pos]);
             break;
        case DIR_CCW:
             if (pos > 0) { //sets a scroll limit
              pos -= 1;
             }
             else {
              pos = numberOfItems - 1;
             }
             lcd.clear();
             lcd.setCursor(3,0);
             lcd.print("SELECT A DRINK");
             lcd.setCursor(3,2);
             lcd.print(drinkItems[pos]);
             break;
        case DIR_NONE:
             break;
        default:
             break;
      }//end switch
    }//end if 
    
    
}


void makeDrink() { 
    switch(pos) {
      case 0: 
        lcd.clear();
        lcd.setCursor(0,2);
        lcd.print("Making");
        lcd.setCursor(2,0);
        lcd.print("Rum and Coke...");
        rumCoke();
        //goHome();
        flashOnce(0, 255, 0);
        flashOnce(0, 255, 0);
        flashOnce(0, 255, 0);
        break;
      case 1:
        //make whiskey coke
        break;
      case 2:
        //make screwdriver
        break;
      default:
        break;
    }   
}

/*
void goHome(currentXPositon, currentYPosition) {
  for (int i = currentXPositon -1; i >0; i--) {
    xStepper.moveTo(i); // Set position
    xStepper.runToPosition(); // Run to position with set speed and acceleration
    yStepper.moveTo(currentYPosition - 1);
    yStepper.runToPosition();
  }
}
*/

void moveXToPosition(int position) {
  xStepper.moveTo(position); // Set position
  xStepper.runToPosition(); // Run to position with set speed and acceleration:
}

void moveYToPosition(int position) {
  yStepper.moveTo(position); // Set position
  yStepper.runToPosition(); // Run to position with set speed and acceleration:
}

void moveZToPosition(int position) {
  zStepper.moveTo(position); // Set position
  zStepper.runToPosition(); // Run to position with set speed and acceleration:
}

void pumpMixer(int mixer, long pumpTime) {
   digitalWrite(mixer, LOW);
   delay(pumpTime);
   digitalWrite(mixer, HIGH);
   delay(500); // delay for possible dripage?
}

void dispenseSpirit() {
  moveZToPosition(100);
  delay(1000);
  moveZToPosition(0);
}

void rumCoke() {

    pumpMixer(COCACOLA, 3000);
    //goHome();
}


void printWelcomeMessage() {
  //print initial message
  lcd.setCursor(3, 0);
  lcd.print("--------------");
  lcd.setCursor(3, 1);
  lcd.print("WELCOME TO THE");
  lcd.setCursor(3, 2);
  lcd.print("MIXMASTER 3000");
  lcd.setCursor(3, 3);
  lcd.print("--------------");
  delay(3000);
}

void runPixelStartup() {

   int delayTime = 50;
   for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 255, 255)); //white
    pixels.show();
    delay(delayTime);
  }
 
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 0, 0)); //red
    pixels.show();
    delay(delayTime);
  }

  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(150, 50, 150)); //purple
    pixels.show();
    delay(delayTime);
  }

  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 255)); //blue
    pixels.show();
    delay(delayTime);
  }

  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(255,255,0)); //yellow
    pixels.show();
    delay(delayTime);
  }
  
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 255, 0)); //green
    pixels.show();
    delay(delayTime);
  }
}

void flashOnce(int r, int g,int b) {
  int flashDelayTime = 200;

  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(r, g, b)); //on
  }
  pixels.show();
  delay(flashDelayTime);
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0)); //off
  }
  pixels.show();
  delay(flashDelayTime);
}

void setColor(int r, int g, int b){
  for(int i=0; i<NUMPIXELS; i++) {
   pixels.setPixelColor(i, pixels.Color(r, g, b)); //on
  }
  pixels.show();
}


//RGB
//RED (255, 0, 0)
//BLUE (0, 0, 255)
//GREEN (0, 255, 0)
//YELLOW 
//PURPLE (255, 0, 255)
//WHITE (255, 255,255)
//OFF(0 ,0, 0)
