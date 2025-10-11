#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// Display pins - CONFIRMED WORKING
#define OLED_SCL 6
#define OLED_SDA 5
#define OLED_OFFSET_X 28
#define OLED_OFFSET_Y 24

// Display object - Amazon specification
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, OLED_SCL, OLED_SDA);

void setup() {
    delay(100);
    
    // Initialize display ONLY
    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(OLED_OFFSET_X + 2, OLED_OFFSET_Y + 15, "MINIMAL!");
    u8g2.sendBuffer();
    
    delay(2000);
}

void loop() {
    static unsigned long lastUpdate = 0;
    static int counter = 0;
    
    if(millis() - lastUpdate > 1000) {  // Every 1 second
        lastUpdate = millis();
        counter++;
        
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x10_tf);
        
        String text = "C: " + String(counter);
        u8g2.drawStr(OLED_OFFSET_X + 2, OLED_OFFSET_Y + 15, text.c_str());
        
        u8g2.sendBuffer();
    }
    
    delay(10);
}
