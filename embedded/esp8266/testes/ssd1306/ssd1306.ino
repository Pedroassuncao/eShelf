#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "libraries/wemosd1mini_pins.h"

#define OLED_RESET D4
Adafruit_SSD1306 display(OLED_RESET);

#define DISPLAY_ADDR 0x3C

void setup(){
    display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDR);

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(4);
    display.setCursor(0, 0);
    display.print("Test");
    display.display();
}

void loop(){
    
}