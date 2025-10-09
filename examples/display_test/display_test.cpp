# Display Test Example

This example demonstrates how to test the built-in OLED display independently of the main project code.

## Purpose
- Verify OLED display connections and functionality
- Test different display modes and animations
- Debug I2C communication issues
- Validate display performance

## Hardware Requirements
- ESP32-C3 development board with built-in 0.42" OLED display
- The display is typically already connected via I2C

## Built-in Display Specifications
- **Size**: 0.42 inch (72x40 pixels)
- **Technology**: OLED (Organic LED)
- **Interface**: I2C 
- **Address**: Usually 0x3C
- **Pins**: SDA (GPIO 8), SCL (GPIO 9)

## Code

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Display configuration for ESP32-C3 with built-in OLED
#define SCREEN_WIDTH    72    // Actual width of 0.42" display
#define SCREEN_HEIGHT   40    // Actual height of 0.42" display
#define OLED_RESET      -1    // Reset pin (not used)
#define OLED_SDA        8     // I2C Data pin
#define OLED_SCL        9     // I2C Clock pin
#define OLED_ADDRESS    0x3C  // I2C address

// Create display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Animation variables
int animFrame = 0;
unsigned long lastUpdate = 0;
const int animDelay = 100; // milliseconds

void setup() {
    Serial.begin(115200);
    Serial.println("OLED Display Test Starting...");
    
    // Initialize I2C with custom pins
    Wire.begin(OLED_SDA, OLED_SCL);
    
    // Initialize display
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println("SSD1306 allocation failed");
        Serial.println("Check I2C connections and address");
        while (1) {
            delay(1000);
            Serial.println("Display initialization failed - check wiring");
        }
    }
    
    Serial.println("Display initialized successfully!");
    Serial.print("Display size: ");
    Serial.print(SCREEN_WIDTH);
    Serial.print("x");
    Serial.println(SCREEN_HEIGHT);
    
    // Run display tests
    runDisplayTests();
}

void loop() {
    // Continuous animation demo
    floppyDiskAnimation();
    delay(50);
}

void runDisplayTests() {
    Serial.println("\n=== Display Test Sequence ===");
    
    // Test 1: Basic display test
    Serial.println("Test 1: Basic display and text");
    testBasicDisplay();
    delay(2000);
    
    // Test 2: Pixel test
    Serial.println("Test 2: Pixel manipulation test");
    testPixels();
    delay(2000);
    
    // Test 3: Graphics test
    Serial.println("Test 3: Basic graphics shapes");
    testGraphics();
    delay(2000);
    
    // Test 4: Text formatting test
    Serial.println("Test 4: Text formatting");
    testTextFormatting();
    delay(2000);
    
    // Test 5: Animation test
    Serial.println("Test 5: Simple animation");
    testAnimation();
    delay(2000);
    
    // Test 6: Floppy disk simulation
    Serial.println("Test 6: Floppy disk animation demo");
    for (int i = 0; i < 50; i++) {
        floppyDiskAnimation();
        delay(100);
    }
    
    Serial.println("Display tests completed!");
    Serial.println("Starting continuous animation...");
}

void testBasicDisplay() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Floppy");
    display.println("Audio");
    display.println("Sim");
    display.display();
}

void testPixels() {
    display.clearDisplay();
    
    // Draw individual pixels in a pattern
    for (int x = 0; x < SCREEN_WIDTH; x += 4) {
        for (int y = 0; y < SCREEN_HEIGHT; y += 4) {
            display.drawPixel(x, y, SSD1306_WHITE);
        }
    }
    
    display.display();
}

void testGraphics() {
    display.clearDisplay();
    
    // Draw rectangles
    display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);  // Border
    display.fillRect(5, 5, 10, 8, SSD1306_WHITE);  // Filled rectangle
    
    // Draw circles (if they fit)
    if (SCREEN_WIDTH >= 20 && SCREEN_HEIGHT >= 20) {
        display.drawCircle(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 8, SSD1306_WHITE);
    }
    
    // Draw lines
    display.drawLine(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1, SSD1306_WHITE);
    display.drawLine(SCREEN_WIDTH-1, 0, 0, SCREEN_HEIGHT-1, SSD1306_WHITE);
    
    display.display();
}

void testTextFormatting() {
    display.clearDisplay();
    
    // Test different text sizes and positions
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    // Small text at top
    display.setCursor(0, 0);
    display.println("Size 1");
    
    // Try size 2 if display is large enough
    if (SCREEN_WIDTH >= 48) {
        display.setTextSize(2);
        display.setCursor(0, 12);
        display.println("S2");
    }
    
    // Reset to size 1 for bottom text
    display.setTextSize(1);
    display.setCursor(0, SCREEN_HEIGHT - 8);
    display.println("Bottom");
    
    display.display();
}

void testAnimation() {
    // Simple bouncing dot animation
    int x = 0, y = SCREEN_HEIGHT / 2;
    int dx = 1, dy = 1;
    
    for (int frame = 0; frame < 100; frame++) {
        display.clearDisplay();
        
        // Draw bouncing dot
        display.fillCircle(x, y, 2, SSD1306_WHITE);
        
        // Update position
        x += dx;
        y += dy;
        
        // Bounce off edges
        if (x <= 2 || x >= SCREEN_WIDTH - 3) dx = -dx;
        if (y <= 2 || y >= SCREEN_HEIGHT - 3) dy = -dy;
        
        // Add frame counter
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.printf("%d", frame);
        
        display.display();
        delay(50);
    }
}

void floppyDiskAnimation() {
    if (millis() - lastUpdate < animDelay) return;
    lastUpdate = millis();
    
    display.clearDisplay();
    
    // Title
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Floppy Sim");
    
    // Draw floppy disk representation
    int diskX = SCREEN_WIDTH / 2 - 8;
    int diskY = 12;
    int diskW = 16;
    int diskH = 12;
    
    // Outer floppy case
    display.drawRect(diskX, diskY, diskW, diskH, SSD1306_WHITE);
    display.drawRect(diskX + 1, diskY + 1, diskW - 2, diskH - 2, SSD1306_WHITE);
    
    // Label area
    display.fillRect(diskX + 2, diskY + 2, diskW - 4, 4, SSD1306_WHITE);
    
    // Disk access animation
    animFrame++;
    if (animFrame > 8) animFrame = 0;
    
    // Animate read/write head
    int headX = diskX + 2 + (animFrame % 4) * 2;
    display.drawLine(headX, diskY + 8, headX + 1, diskY + 8, SSD1306_WHITE);
    
    // Activity indicator dots
    display.setCursor(0, SCREEN_HEIGHT - 8);
    display.print("ACT:");
    for (int i = 0; i < (animFrame % 4) + 1; i++) {
        display.print("*");
    }
    
    // Status text that changes
    const char* statusTexts[] = {
        "Reading...",
        "Seeking...",
        "Writing...",
        "Ready"
    };
    
    display.setCursor(0, 26);
    display.print(statusTexts[animFrame % 4]);
    
    display.display();
}

// Additional utility functions for advanced testing
void displaySystemInfo() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    
    display.printf("Free:");
    display.println(ESP.getFreeHeap());
    display.printf("CPU:");
    display.printf("%dMHz", ESP.getCpuFreqMHz());
    
    display.display();
}

void displayNetworkInfo() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    
    display.println("Network:");
    display.println("AP Mode");
    display.println("192.168.4.1");
    
    display.display();
}

void displayScrollingText(const char* text) {
    int textWidth = strlen(text) * 6; // Approximate character width
    
    for (int offset = SCREEN_WIDTH; offset > -textWidth; offset -= 2) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(offset, SCREEN_HEIGHT / 2 - 4);
        display.print(text);
        display.display();
        delay(50);
    }
}

void displayBargraph(int value, int maxValue) {
    display.clearDisplay();
    
    // Draw bargraph frame
    display.drawRect(0, SCREEN_HEIGHT - 12, SCREEN_WIDTH, 10, SSD1306_WHITE);
    
    // Calculate filled width
    int fillWidth = map(value, 0, maxValue, 0, SCREEN_WIDTH - 2);
    display.fillRect(1, SCREEN_HEIGHT - 11, fillWidth, 8, SSD1306_WHITE);
    
    // Display value
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.printf("%d/%d", value, maxValue);
    
    display.display();
}

void displayMemoryUsage() {
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t usedHeap = totalHeap - freeHeap;
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    
    display.println("Memory:");
    display.printf("Used: %dk", usedHeap / 1024);
    display.setCursor(0, 16);
    display.printf("Free: %dk", freeHeap / 1024);
    
    // Memory usage bar
    int usagePercent = (usedHeap * 100) / totalHeap;
    displayBargraph(usagePercent, 100);
    
    display.display();
}
```

## Usage Instructions

1. **Upload the code** to your ESP32-C3 board with built-in OLED
2. **Open serial monitor** at 115200 baud to see test progress
3. **Watch the display** go through various test patterns
4. **Observe the final animation** showing floppy disk activity

## Expected Display Sequence

1. **Basic Text**: Simple text display with "Floppy Audio Sim"
2. **Pixel Pattern**: Dots in a regular pattern across the screen
3. **Graphics Shapes**: Rectangles, lines, and borders
4. **Text Formatting**: Different text sizes and positions
5. **Bouncing Animation**: Moving dot with frame counter
6. **Floppy Animation**: Continuous floppy disk activity simulation

## Troubleshooting

### Display Not Working

**Check these items:**
```
SSD1306 allocation failed
Check I2C connections and address
```

**Solutions:**
- Verify I2C address (try 0x3C or 0x3D)
- Check if display is already initialized elsewhere
- Ensure proper power supply to display
- Verify I2C pins are correct for your board

### Garbled Display

**Possible causes:**
- Wrong display resolution settings
- I2C communication errors
- Power supply instability
- Incorrect pin assignments

### Partial Display Issues

**Solutions:**
- Adjust `SCREEN_WIDTH` and `SCREEN_HEIGHT` constants
- Check if display has different pixel mapping
- Verify display driver compatibility
- Test with simpler graphics first

### I2C Communication Errors

**Debug steps:**
```cpp
// Add I2C scanner to find device address
void scanI2C() {
    Serial.println("Scanning I2C devices...");
    for (byte addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("Device found at 0x%02X\n", addr);
        }
    }
}
```

## Advanced Testing

### Display Performance
```cpp
// Measure display update speed
unsigned long startTime = millis();
display.display();
unsigned long updateTime = millis() - startTime;
Serial.printf("Display update: %lu ms\n", updateTime);
```

### Custom Bitmaps
```cpp
// Define custom floppy disk bitmap
const unsigned char floppyBitmap[] PROGMEM = {
    0x7F, 0xFE, 0x80, 0x01, 0x9F, 0xF9, 0x90, 0x09,
    0x90, 0x09, 0x9F, 0xF9, 0x80, 0x01, 0x7F, 0xFE
};

display.drawBitmap(x, y, floppyBitmap, 16, 8, SSD1306_WHITE);
```

### Power Consumption Testing
```cpp
// Test different brightness levels (if supported)
void testBrightness() {
    for (int brightness = 0; brightness <= 255; brightness += 32) {
        display.ssd1306_command(SSD1306_SETCONTRAST);
        display.ssd1306_command(brightness);
        delay(1000);
    }
}
```

## Display Optimizations

### Memory Usage
- Use `PROGMEM` for static bitmaps and text
- Clear only necessary areas instead of full screen
- Use partial updates when possible

### Performance
- Minimize `display.display()` calls
- Batch multiple drawing operations
- Use hardware scrolling features if available

### Power Saving
- Turn off display when not needed
- Reduce update frequency
- Use lower contrast settings

## Integration with Main Project

### Display Manager Class
```cpp
class DisplayManager {
private:
    Adafruit_SSD1306& display;
    unsigned long lastUpdate;
    int currentScreen;
    
public:
    void updateStatus(bool playing, const String& filename);
    void showNetworkInfo(IPAddress ip);
    void showVolumeLevel(int volume);
};
```

### Thread Safety
- Ensure display updates don't conflict with other tasks
- Use semaphores or mutexes if needed
- Coordinate with WiFi and audio operations

---

*This display test helps ensure your OLED is working correctly before integrating with the full floppy disk audio simulator.*