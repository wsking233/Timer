#include <Arduino.h>
#include <FastLED.h>
#include <FastLED_NeoMatrix.h>

// define the matrix PIN, width and height
#define DATA_PIN 4  //led matrix data pin
#define MATRIX_WIDTH 32 //led matrix width
#define MATRIX_HEIGHT 8 //led matrix height

// define the button PIN
#define START_BUTTON 12   // start button
#define SETTING_BUTTON 11 // setting button
#define TIMER_PLUS 10     // timer plus button
#define TIMER_MINUS 9     // timer minus button
#define BUZZER 7          // buzzer

CRGB leds[MATRIX_WIDTH * MATRIX_HEIGHT];
FastLED_NeoMatrix *matrix;

bool settingModel; //to check if the timer is in setting model
bool buzzerOn;

int timerDuration; //to store the timer duration in seconds
//only used for printing the timer in "00:00" format
int timerMin; //to store the timer in minutes 
int timerSec; //to store the timer in seconds

int pressedTimes; //to store how many times the timer button is pressed

char timeText[6]; //to store the timer in "00:00" format

void setTimer(int times)
{
  timerDuration = times * 10; //each press will add 10 seconds to the timer
  timerMin = timerDuration / 60;  // get the minutes by dividing the duration by 60
  timerSec = timerDuration % 60;  // get the seconds by getting the remainder of the duration divided by 60
}

void UpdateTimer()
{
  // logic for the timer
  if (timerDuration > 0)  //when the timer is not time up
  {
    timerDuration--;  
    timerSec--;
    if (timerSec < 0)
    {
      timerSec = 59;  //when the seconds is less than 0 and the timer is not time up
      if (timerMin > 0)
      {
        timerMin--;
      }
    }
  }
  else  //when the timer is time up
  { // reset the timer
    timerDuration = 0;
    timerMin = 0;
    timerSec = 0;
  }
}

void printTimer()
{  // print the timer in "00:00" format
  snprintf(timeText, sizeof(timeText), "%02d:%02d", timerMin, timerSec);  //update the timeText
  matrix->clear();                
  matrix->setCursor(2, 0);  //adjust the position on the matrix
  matrix->print(timeText);
  matrix->show();
}

void startTimer() // call setTimer() before calling this function
{
  UpdateTimer();
  matrix->setTextColor(CRGB::Blue); //blue while the timer is counting down
  printTimer();
  delay(1000);
  matrix->clear();
}

void printText(String text, int speed)
{
  int textLength = text.length();
  int displayWidth = MATRIX_WIDTH;
  int xPos = displayWidth; 

  while (xPos >= -textLength * 6) //6 is the width of each character
  {
    matrix->clear();
    matrix->setCursor(xPos, 0);
    matrix->print(text);
    matrix->show();
    delay(speed); //control the speed of the text scrolling

    xPos--; //move the text to the left
  }
}

void setup()
{
  //initialise the matrix
  FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, MATRIX_WIDTH * MATRIX_HEIGHT); //initialise the leds

  matrix = new FastLED_NeoMatrix(leds, MATRIX_WIDTH, MATRIX_HEIGHT,
                                 NEO_MATRIX_TOP + NEO_MATRIX_LEFT +
                                     NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG); //initialise the matrix

  matrix->begin();
  matrix->setTextWrap(false); 
  matrix->setBrightness(5);
  matrix->clear();
  matrix->setCursor(1, 0);
  matrix->print("TIMER");
  matrix->show();

  //initialise the timer
  timerDuration = 0; 
  timerMin = 0;
  timerSec = 0;
  timeText[0] = '0';
  timeText[1] = '0';
  timeText[2] = ':';
  timeText[3] = '0';
  timeText[4] = '0';
  timeText[5] = '\0'; // add null terminator to end of string

  pressedTimes = 0;
  settingModel = true;
  buzzerOn = true;

  //initialise the buttons
  pinMode(START_BUTTON, INPUT_PULLUP);
  pinMode(SETTING_BUTTON, INPUT_PULLUP);
  pinMode(TIMER_PLUS, INPUT_PULLUP);
  pinMode(TIMER_MINUS, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  delay(2000);
  matrix->clear();
}

void loop()
{

  if (settingModel) 
  {   //setting model
    if (digitalRead(TIMER_PLUS) == LOW) //detect the timer plus button
    {
      pressedTimes++;
      delay(200); //delay to avoid the button been pressed multiple times
    }
    if (digitalRead(TIMER_MINUS) == LOW) //detect the timer minus button
    {
      if (pressedTimes > 0) //the timer cannot be less than 0
      {
        pressedTimes--;
      }
      delay(200);
    }
    if (digitalRead(START_BUTTON) == LOW) //detect the start button
    {
      settingModel = false; //exit the setting model
      delay(200);
    }
    setTimer(pressedTimes);
    matrix->setTextColor(CRGB::LightPink); //pink in the setting model
    printTimer();  
  }
  else //counting down model
  {

    if (timerDuration <= 0)
    { //timer time up

      if (buzzerOn) //buzzer on when the timer is time up
      {
        tone(BUZZER, 1000);
        buzzerOn = false; //change the buzzer status to off, incase the buzzer been turned on again by next loop
      }
    
      //turn off the buzzer when any button but the setting button is pressed
      // digitalRead(SETTING_BUTTON) == LOW || 
      if (digitalRead(TIMER_PLUS) == LOW ||
          digitalRead(TIMER_MINUS) == LOW ||
          digitalRead(START_BUTTON) == LOW)
      {
        
        noTone(BUZZER);
        buzzerOn = false;
        delay(200); //delay to avoid the button been pressed multiple times
      }

      // printText("TIME UP", 100); // 显示 TIME UP
      matrix->clear();
      matrix->setTextSize(1);
      matrix->setTextColor(CRGB::Green);  //green when the timer is time up
      matrix->setCursor(0, 0);
      matrix->print("READY");
      matrix->show();
      delay(500);
      matrix->clear();
      matrix->setCursor(12, 0);
      matrix->print("UP");
      matrix->show();
      delay(500);
    }
    else
    { 
      startTimer();
    }

      //detect the setting button to enter the setting model
    if (digitalRead(SETTING_BUTTON) == LOW)
    {
      //reset the buzzer status to on and the pressedTimes to 0
      settingModel = true;
      buzzerOn = true;
      pressedTimes = 0;
      delay(200);
    }
  }
}
