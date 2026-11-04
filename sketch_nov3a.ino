#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>
#include <Keyboard.h>

#define TFT_CS    17
#define TFT_DC    20
#define TFT_RST   21
#define TOUCH_CS  14
#define TOUCH_IRQ 10
#define BACKLIGHT 22
#define SLIDER    27

const int ROWS[2] = {0, 1};
const int COLS[3] = {2, 3, 4};

const uint8_t KEYMAP[2][3] = {
  {KEY_F13, KEY_F14, KEY_F15},
  {KEY_F16, KEY_F17, KEY_F18}
};
const char* LABELS[6] = {"F13", "F14", "F15", "F16", "F17", "F18"};

Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

bool keyDown[2][3] = {};
int lastSlider = -1;

void drawKey(int r, int c, bool pressed) {
  int w = 100, h = 70;
  int x = 10 + c * (w + 5);
  int y = 10 + r * (h + 5);
  tft.fillRoundRect(x, y, w, h, 8, pressed ? ILI9341_CYAN : ILI9341_DARKGREY);
  tft.setTextColor(pressed ? ILI9341_BLACK : ILI9341_WHITE);
  tft.setTextSize(3);
  tft.setCursor(x + 22, y + 24);
  tft.print(LABELS[r * 3 + c]);
}

void drawSlider(int percent) {
  tft.fillRect(10, 190, 300, 30, ILI9341_BLACK);
  tft.drawRect(10, 190, 300, 30, ILI9341_WHITE);
  tft.fillRect(12, 192, percent * 296 / 100, 26, ILI9341_GREEN);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(120, 160);
  tft.print("Bright ");
  tft.print(percent);
  tft.print("%  ");
}

void pressKey(int r, int c, bool down) {
  if (down == keyDown[r][c]) return;
  keyDown[r][c] = down;
  if (down) Keyboard.press(KEYMAP[r][c]);
  else      Keyboard.release(KEYMAP[r][c]);
  drawKey(r, c, down);
}

void setup() {
  for (int c = 0; c < 3; c++) pinMode(COLS[c], INPUT_PULLUP);
  for (int r = 0; r < 2; r++) pinMode(ROWS[r], INPUT);
  pinMode(BACKLIGHT, OUTPUT);

  Keyboard.begin();
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  touch.begin();
  touch.setRotation(1);

  for (int r = 0; r < 2; r++)
    for (int c = 0; c < 3; c++) drawKey(r, c, false);
}

void loop() {
  bool now[2][3] = {};

  for (int r = 0; r < 2; r++) {
    pinMode(ROWS[r], OUTPUT);
    digitalWrite(ROWS[r], LOW);
    delayMicroseconds(10);
    for (int c = 0; c < 3; c++) now[r][c] = (digitalRead(COLS[c]) == LOW);
    pinMode(ROWS[r], INPUT);
  }

  if (touch.touched()) {
    TS_Point p = touch.getPoint();
    int x = map(p.x, 200, 3900, 0, 320);
    int y = map(p.y, 200, 3900, 0, 240);
    int c = (x - 10) / 105, r = (y - 10) / 75;
    if (r >= 0 && r < 2 && c >= 0 && c < 3) now[r][c] = true;
  }

  for (int r = 0; r < 2; r++)
    for (int c = 0; c < 3; c++) pressKey(r, c, now[r][c]);

  int percent = analogRead(SLIDER) * 100 / 1023;
  if (abs(percent - lastSlider) > 1) {
    analogWrite(BACKLIGHT, 255 - percent * 255 / 100);
    drawSlider(percent);
    lastSlider = percent;
  }

  delay(10);
}