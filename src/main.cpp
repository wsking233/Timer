#include <Arduino.h>
#include <FastLED.h>
#include <FastLED_NeoMatrix.h>

// define the matrix PIN, width and height
#define DATA_PIN 4 // 连接到DIN引脚
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 8

// define the button PIN
#define START_BUTTON 12   // start button
#define SETTING_BUTTON 11 // setting button
#define TIMER_PLUS 10     // timer plus button
#define TIMER_MINUS 9     // timer minus button
#define BUZZER 7          // buzzer

CRGB leds[MATRIX_WIDTH * MATRIX_HEIGHT];
FastLED_NeoMatrix *matrix;

bool settingModel; // 设置模式
bool buzzerOn;

int timerDuration; // 用于存储计时器持续时间
int timerMin;
int timerSec;

int pressedTimes; // 用于存储按钮按下次数

char timeText[6]; // 用于存储格式化后的时间文本

void setTimer(int times)
{
  timerDuration = times * 10;
  timerMin = timerDuration / 60;
  timerSec = timerDuration % 60;
}

void UpdateTimer()
{
  // logic for the timer
  if (timerDuration > 0)
  {
    timerDuration--;
    timerSec--;
    if (timerSec < 0)
    {
      timerSec = 59;
      if (timerMin > 0)
      {
        timerMin--;
      }
    }
  }
  else
  {
    timerDuration = 0;
    timerMin = 0;
    timerSec = 0;
  }
}

void printTimer()
{
  // 使用 snprintf 函数将分钟和秒格式化为 "00:00" 形式.
  snprintf(timeText, sizeof(timeText), "%02d:%02d", timerMin, timerSec);
  matrix->clear();                // 清除屏幕
  matrix->setCursor(2, 0);        // 设置光标位置
  matrix->print(timeText);
  matrix->show();
}

void startTimer() // call setTimer() before calling this function
{
  UpdateTimer();
  matrix->setTextColor(CRGB::Blue); // 设置字体颜色为蓝色
  printTimer();
  delay(1000);
  matrix->clear();
}

void printText(String text, int speed)
{
  int textLength = text.length();
  int displayWidth = MATRIX_WIDTH; // 显示窗口的宽度等于点阵屏的宽度
  int xPos = displayWidth;         // 初始 x 坐标在显示窗口的右边界

  while (xPos >= -textLength * 6)
  {                  // 循环直到文本完全移出显示窗口
    matrix->clear(); // 清除屏幕

    // 使用 setCursor 设置文本的位置，y 坐标可以根据需要调整
    matrix->setCursor(xPos, 0);

    // 使用 print 函数将文本显示在屏幕上
    matrix->print(text);

    matrix->show(); // 刷新屏幕

    delay(speed); // 控制滚动速度

    xPos--; // 减小 x 坐标，实现滚动效果
  }
}

void setup()
{
  FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, MATRIX_WIDTH * MATRIX_HEIGHT);

  matrix = new FastLED_NeoMatrix(leds, MATRIX_WIDTH, MATRIX_HEIGHT,
                                 NEO_MATRIX_TOP + NEO_MATRIX_LEFT +
                                     NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG);

  matrix->begin();
  matrix->setTextWrap(false);
  matrix->setBrightness(5); // 设置亮度（可以根据需要调整）
  matrix->clear();          // 清除屏幕
  matrix->setCursor(1, 0);  // 设置光标位置
  matrix->print("TIMER");
  matrix->show(); // 显示屏幕

  timerDuration = 0; // 设置计时器持续时间
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

  if (settingModel) // 如果在设置模式中
  {
    if (digitalRead(TIMER_PLUS) == LOW) // 检测加号按钮
    {
      pressedTimes++;
      delay(200);
    }
    if (digitalRead(TIMER_MINUS) == LOW) // 检测减号按钮
    {
      if (pressedTimes > 0)
      {
        pressedTimes--;
      }
      delay(200);
    }
    if (digitalRead(START_BUTTON) == LOW) // 检测开始按钮
    {
      settingModel = false; // 退出设置模式
      delay(200);
    }
    setTimer(pressedTimes);
    matrix->setTextColor(CRGB::LightPink); // 设置字体颜色为红色
    printTimer();  
  }
  else // 如果不在设置模式中
  {

    if (timerDuration <= 0)
    { // 如果时间到了

      if (buzzerOn)
      {
        tone(BUZZER, 1000);
        buzzerOn = false;
      }
    
      // 检测任意按钮按下，关闭buzzer
      // digitalRead(SETTING_BUTTON) == LOW ||
      if (digitalRead(TIMER_PLUS) == LOW ||
          digitalRead(TIMER_MINUS) == LOW ||
          digitalRead(START_BUTTON) == LOW)
      {
        // 关闭buzzer
        noTone(BUZZER);
        buzzerOn = false;
        delay(200);
      }

      // printText("TIME UP", 100); // 显示 TIME UP
      matrix->clear();                // 清除屏幕
      matrix->setTextSize(1);         // 设置字体大小
      matrix->setTextColor(CRGB::Green); // 设置字体颜色为绿色
      matrix->setCursor(0, 0);        // 设置光标位置
      matrix->print("READY");
      matrix->show();
      delay(500);
      matrix->clear();          // 清除屏幕
      matrix->setCursor(12, 0); // 设置光标位置
      matrix->print("UP");
      matrix->show();
      delay(500);
    }
    else
    { // 如果时间没到
      startTimer();
    }

    // 检测是否按下设置按钮
    if (digitalRead(SETTING_BUTTON) == LOW)
    {
      settingModel = true;
      pressedTimes = 0;
      buzzerOn = true;
      delay(200);
    }
  }
}
