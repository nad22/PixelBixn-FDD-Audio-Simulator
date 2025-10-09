#include <Arduino.h>
#include <U8g2lib.h>

// diymore ESP32-C3 Super Mini exact configuration from Amazon
// SSD1306 controller, SCL=6, SDA=5, 72x40 display with offsets
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 6, 5);

int width = 72;
int height = 40;  
int xOffset = 28;
int yOffset = 24;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("=== diymore ESP32-C3 Super Mini U8G2 Test ===");
    Serial.println("Using exact Amazon specs: SCL=6, SDA=5, offsets=28,24");
    
    // Initialize U8G2 display
    u8g2.begin();
    Serial.println("U8G2 display initialized");
    
    // Clear and test display
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    
    // Apply offsets and draw within the 72x40 area
    u8g2.drawStr(xOffset, yOffset + 10, "TEST");
    u8g2.drawStr(xOffset, yOffset + 25, "WORKS!");
    u8g2.drawStr(xOffset, yOffset + 40, "72x40");
    
    u8g2.sendBuffer();
    Serial.println("Test pattern sent to display with offsets");
    
    delay(2000);
    
    // Test 2: Show technical info
    u8g2.clearBuffer();
    u8g2.drawStr(xOffset, yOffset + 10, "diymore");  
    u8g2.drawStr(xOffset, yOffset + 25, "ESP32-C3");
    u8g2.drawStr(xOffset, yOffset + 40, "SCL:6 SDA:5");
    u8g2.sendBuffer();
    
    Serial.println("Technical info sent to display");
}

void loop() {
    static int counter = 0;
    
    delay(2000);
    counter++;
    
    // Animated counter test
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(xOffset, yOffset + 10, "COUNTER:");
    
    char countStr[10];
    sprintf(countStr, "%d", counter);
    u8g2.drawStr(xOffset, yOffset + 25, countStr);
    
    // Simple animation
    for(int i = 0; i < (counter % 4); i++) {
        u8g2.drawStr(xOffset + i*8, yOffset + 40, ">");
    }
    
    u8g2.sendBuffer();
    
    Serial.printf("Counter: %d displayed\n", counter);
}