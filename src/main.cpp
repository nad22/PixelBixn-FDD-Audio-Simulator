#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>
#include <Preferences.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <driver/i2s.h>
#include <math.h>
#include <esp_task_wdt.h>  // For watchdog control

// I2S Audio Output for MAX98357 amplifier
// Using ESP32 hardware I2S for proper audio output

// Audio pins for MAX98357 - corrected for your ESP32-C3 board
// Avoiding special function pins: 9=SCL, 8=SDA, 7=SS, 6=MOSI, 5=MISO, 20/21=RX/TX
#define I2S_BCLK 3   // Bit clock - GPIO 3
#define I2S_LRC 4    // Left/Right clock - GPIO 4
#define I2S_DOUT 2   // Data out - GPIO 2
#define I2S_SD 1     // Shutdown control - GPIO 1
#define I2S_GAIN 0   // Gain control - GPIO 0

// Button pin for trigger sound
#define BUTTON_PIN 7  // GPIO 7 - Button to GND (internal pull-up)

// I2S audio configuration
#define I2S_SAMPLE_RATE 44100     // 44.1kHz sample rate
#define I2S_BUFFER_SIZE 512       // I2S DMA buffer size
#ifndef PI
#define PI 3.14159265359
#endif

// Hardware pin definitions - Display bleibt fix verdrahtet!
// Display: 0.42" OLED with SSD1306 controller on I2C
#define OLED_SCL 6  // CONFIRMED WORKING - NICHT ÄNDERN!
#define OLED_SDA 5  // CONFIRMED WORKING - NICHT ÄNDERN!
#define OLED_WIDTH 72
#define OLED_HEIGHT 40
#define OLED_OFFSET_X 28  // From Amazon specs
#define OLED_OFFSET_Y 24  // From Amazon specs

// Control pins - updated after audio pin reassignment
#define TRIGGER_PIN 9    // Available GPIO
#define STATUS_LED 8     // Onboard LED

// Display object - WORKING U8G2 configuration for diymore ESP32-C3 Super Mini
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, OLED_SCL, OLED_SDA);

// Web server
AsyncWebServer server(80);

// I2S audio system
bool audioHardwareInitialized = false;
float currentPhase = 0.0;  // For phase-continuous audio

// SIMPLIFIED: No audio task, direct blocking playback
volatile bool audioStopRequested = false;
volatile bool isPlayingAudio = false;  // Track if audio is playing

// Removed mutex system - caused timeout issues

// Audio simulation variables for status display
bool audioSimulationActive = false;
unsigned long audioSimulationStart = 0;
unsigned long audioSimulationDuration = 0;
String currentAudioFile = "";

// Preferences for configuration storage
Preferences preferences;

// File caching to reduce SPIFFS access
bool fileCacheValid = false;
unsigned long lastFileScan = 0;
const unsigned long FILE_CACHE_TIMEOUT = 30000; // 30 seconds

// Configuration structure
struct Config {
    char ssid[32] = "FloppyDisk_AP";
    char password[32] = "";
    int volume = 50;
    bool randomPlay = true;
    char currentSound[64] = "default.wav";
    char startupSound[64] = "";  // Startup sound (empty = no sound)
    char triggerSound[64] = "";  // Button trigger sound (empty = random)
    bool highGain = false;  // false = 9dB, true = 15dB
    int delayDiskBoot = 0;      // Delay in ms before floppy animation starts for boot sound
    int delayDiskTrigger = 0;   // Delay in ms before floppy animation starts for trigger sound
    bool debugOutput = true;    // Enable/disable serial debug output
} config;

// Animation variables for random floppy blinking
unsigned long lastAnimUpdate = 0;
int animFrame = 0;
bool isPlaying = false;
unsigned long playStartTime = 0;
volatile bool showFloppy = false;           // Show floppy during playback
volatile bool floppyFilled = false;         // Current state: filled or outline
volatile bool floppyDisplayed = false;      // Track if floppy is currently displayed
volatile unsigned long floppyAnimStart = 0; // When animation started (after delay)
volatile unsigned long nextFloppyToggle = 0; // When to toggle next
volatile bool isBootSound = false;          // Track if current sound is boot sound

// Button handling variables
bool lastButtonState = HIGH;
unsigned long lastButtonPress = 0;
const unsigned long BUTTON_DEBOUNCE = 50;  // 50ms debounce

// Sound file list
std::vector<String> soundFiles;

// Debug log for web interface
String debugLog = "";
void addDebugLog(const String& msg) {
    debugLog += "[" + String(millis()/1000) + "s] " + msg + "<br>";
    // Keep only last 30 lines
    int lineCount = 0;
    int lastBr = debugLog.length();
    for(int i = debugLog.length() - 1; i >= 0; i--) {
        if(debugLog[i] == '>') {  // End of <br>
            lineCount++;
            if(lineCount > 30) {
                debugLog = debugLog.substring(i + 1);
                break;
            }
        }
    }
}

// Combined debug function - Serial + Web Log
void debugPrint(const String& msg) {
    if(config.debugOutput) {
        Serial.print(msg);
    }
    // Always add to web log (for web monitoring)
    addDebugLog(msg);
}

void debugPrintln(const String& msg) {
    if(config.debugOutput) {
        Serial.println(msg);
    }
    // Always add to web log (for web monitoring)
    addDebugLog(msg);
}

void debugPrintf(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    if(config.debugOutput) {
        Serial.print(buffer);
    }
    // Always add to web log (for web monitoring)
    addDebugLog(String(buffer));
}

// Function declarations
void initDisplay();
void initAudio();
void initWiFi();
void initWebServer();
void loadConfiguration();
void saveConfiguration();
void updateDisplay();


void playSound(const String& filename);
void stopSound();
void setGain(bool highGain);
void testAudioAmplifier();
void checkTriggerButton();
void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
void scanSoundFiles();
String getRandomSound();
void playWavFileBlocking(const String& filepath);  // Direct WAV player (blocking)

// Start floppy animation with delay
void startFloppyAnimation(bool isBoot) {
    showFloppy = true;
    isBootSound = isBoot;
    floppyAnimStart = millis();
    floppyDisplayed = false;  // Reset - will be set to true after delay
    
    debugPrintf("Floppy animation started (boot=%d, delay=%dms)\n", isBoot, isBoot ? config.delayDiskBoot : config.delayDiskTrigger);
}

// Stop floppy animation
void stopFloppyAnimation() {
    debugPrintf(">>> STOPPING ANIMATION: showFloppy WAS %d\n", showFloppy);
    showFloppy = false;
    floppyAnimStart = 0;
    floppyFilled = false;
    floppyDisplayed = false;  // CRITICAL: Reset display flag so next trigger works!
    nextFloppyToggle = 0;
    
    // Explicitly clear display NOW
    u8g2.clearBuffer();
    u8g2.sendBuffer();
    
    debugPrintln("Floppy animation stopped - display cleared");
}

// 'floppy', 72x40px - Detailed floppy disk bitmap
const unsigned char epd_bitmap_floppy [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0xc4, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0xc4, 0x07, 0x00, 
	0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x07, 0xc0, 0x0f, 0x07, 0x00, 0x00, 0x00, 0x00, 0xfc, 
	0x07, 0xc0, 0x0f, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x07, 0xc0, 0x0f, 0x1f, 0x00, 0x00, 0x00, 
	0x00, 0xfc, 0x07, 0xc0, 0x0f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x07, 0xc0, 0x0f, 0x3f, 0x00, 
	0x00, 0x00, 0x00, 0xfc, 0x07, 0xc0, 0x0f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x07, 0xc0, 0x0f, 
	0x3f, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x07, 0xc0, 0x0f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x07, 
	0xc0, 0x0f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x07, 0xc0, 0x0f, 0x3f, 0x00, 0x00, 0x00, 0x00, 
	0xfc, 0x07, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff, 0xff, 0x3f, 0x00, 0x00, 
	0x00, 0x00, 0xfc, 0xff, 0xff, 0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff, 0xff, 0x3f, 
	0x00, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff, 0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff, 
	0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xfc, 
	0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xf8, 0xff, 0x1f, 0x3f, 0x00, 0x00, 0x00, 
	0x00, 0xfc, 0xf8, 0xff, 0x0f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x3f, 0x00, 
	0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xf8, 0xff, 0x1f, 
	0x3f, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 
	0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x08, 0x02, 0x3f, 0x00, 0x00, 0x00, 0x00, 
	0xfc, 0xf8, 0xff, 0x1f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 
	0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x3f, 
	0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 
	0x80, 0x33, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff, 0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xfc, 
	0xff, 0xff, 0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Floppy disk icon drawing function using detailed bitmap
void drawFloppyDisk(int x, int y, bool filled) {
    // Draw the 72x40 pixel detailed floppy bitmap - centered on display
    u8g2.drawXBM(x, y, 72, 40, epd_bitmap_floppy);
}

void setup() {
    Serial.begin(115200);
    delay(100);
    debugPrintln("\n\n=== Floppy Disk Audio Simulator v1.0 ===");
    
    // Display initialization
    delay(100);
    u8g2.begin();
    u8g2.setDisplayRotation(U8G2_R0);
    
    // Clear display - completely blank on startup
    u8g2.clearBuffer();
    u8g2.sendBuffer();
    
    debugPrintln("Display initialized - blank screen");
    
    // Initialize SPIFFS filesystem
    debugPrintln("Initializing SPIFFS...");
    if(!SPIFFS.begin(true)) {
        debugPrintln("ERROR: SPIFFS Mount Failed!");
    } else {
        debugPrintln("SPIFFS mounted successfully");
    }
    
    // Load configuration
    debugPrintln("Loading configuration...");
    loadConfiguration();
    
    // Scan for WAV files
    debugPrintln("Scanning for WAV files...");
    scanSoundFiles();
    
    // WiFi initialization
    debugPrintln("Initializing WiFi...");
    initWiFi();
    
    // Webserver initialization
    debugPrintln("Initializing web server...");
    initWebServer();
    
    // Audio initialization
    debugPrintln("Initializing audio...");
    initAudio();
    
    if(audioHardwareInitialized) {
        debugPrintln("Audio initialized successfully!");
        
        // Play configured startup sound (if set) or beep
        if(strlen(config.startupSound) > 0) {
            String filepath = String(config.startupSound);
            if(SPIFFS.exists("/" + filepath)) {
                debugPrintln("Playing startup sound: " + filepath);
                // Start boot animation directly with correct flag
                startFloppyAnimation(true);
                playWavFileBlocking("/" + filepath);
                stopFloppyAnimation();
            } else {
                debugPrintln("Startup sound not found, playing beep");
                startFloppyAnimation(true);
                testAudioAmplifier();
                stopFloppyAnimation();
            }
        } else {
            // No startup sound configured, play beep
            startFloppyAnimation(true);
            testAudioAmplifier();
            stopFloppyAnimation();
        }
    } else {
        debugPrintln("ERROR: Audio init failed!");
    }
    
    // Setup button pin with internal pull-up
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    debugPrintln("Button pin initialized: GPIO " + String(BUTTON_PIN));
    
    // Clear display for clean state - floppy will only show during playback
    u8g2.clearBuffer();
    u8g2.sendBuffer();
    debugPrintln("Display cleared - ready for floppy animation");
    
    debugPrintln("=== READY ===");
}

void loop() {
    // Check trigger button
    checkTriggerButton();
    
    // Debug output every 5 seconds
    static unsigned long lastDebug = 0;
    unsigned long now = millis();
    
    if(now - lastDebug > 5000) {
        debugPrintf("DEBUG: showFloppy=%d, floppyDisplayed=%d\n", showFloppy, floppyDisplayed);
        lastDebug = now;
    }
    
    // Yield to system
    delay(10);
}

// ===== REST OF THE FILE - ALL FUNCTIONS REMAIN FOR COMPILATION =====

void initDisplay() {
    debugPrintln("Starting 0.42 inch OLED with U8G2...");
    debugPrintf("Pins: SCL=%d, SDA=%d, Offsets: X=%d, Y=%d\n", OLED_SCL, OLED_SDA, OLED_OFFSET_X, OLED_OFFSET_Y);
    
    // Initialize U8G2 display - this handles I2C automatically
    u8g2.begin();
    debugPrintln("U8G2 display initialized successfully!");
    
    // Show startup message with proper offsets
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    
    u8g2.drawStr(OLED_OFFSET_X, OLED_OFFSET_Y + 12, "FDD-SIM");
    u8g2.drawStr(OLED_OFFSET_X, OLED_OFFSET_Y + 28, "Starting...");
    
    u8g2.sendBuffer();
    debugPrintln("Startup message displayed");
    
    // Removed 2 second delay for faster startup
    
    debugPrintln("Display initialization complete");
}

void initAudio() {
    addDebugLog("Init audio pins...");
    // Initialize audio control pins for MAX98357 amplifier
    pinMode(I2S_SD, OUTPUT);
    pinMode(I2S_GAIN, OUTPUT);
    
    digitalWrite(I2S_SD, LOW);  // Start with amplifier OFF
    setGain(config.highGain);   // Set initial gain from config
    
    addDebugLog("Installing I2S driver...");
    // Setup I2S for MAX98357 amplifier
    debugPrintln("Initializing I2S audio output for MAX98357...");
    
    // I2S configuration - Stereo for MAX98357 compatibility
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,  // Stereo for MAX98357
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,  // Default interrupt
        .dma_buf_count = 2,     // Minimal buffers
        .dma_buf_len = 64,      // Small buffer for fast init
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };
    
    // I2S pin configuration
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCLK,
        .ws_io_num = I2S_LRC,
        .data_out_num = I2S_DOUT,
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    
    // Install and start I2S driver
    esp_err_t result = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (result != ESP_OK) {
        addDebugLog("I2S install FAILED: " + String(result));
        debugPrintf("I2S driver install FAILED: %d\n", result);
        return;
    }
    addDebugLog("I2S driver installed OK");
    
    result = i2s_set_pin(I2S_NUM_0, &pin_config);
    if (result != ESP_OK) {
        addDebugLog("I2S pin setup FAILED: " + String(result));
        debugPrintf("I2S pin setup FAILED: %d\n", result);
        return;
    }
    addDebugLog("I2S pins configured OK");
    
    audioHardwareInitialized = true;
    
    // ENABLE AMPLIFIER RIGHT AWAY
    addDebugLog("Enabling amplifier (I2S_SD=HIGH)");
    digitalWrite(I2S_SD, HIGH);
    delay(50);
    addDebugLog("Amplifier enabled");
    
    // Minimal logging for faster boot
    debugPrintln("I2S Audio ready - AMPLIFIER ENABLED");
    
    // SIMPLIFIED: No mutex, no task - direct blocking playback
    debugPrintln("Using direct blocking playback");
}

void initWiFi() {
    debugPrintln("Starting WiFi in AP mode...");
    
    // Set WiFi to AP-only mode for fastest startup
    WiFi.mode(WIFI_AP);
    
    // Ultra-low power WiFi for close-range stability (1-2 meters)
    WiFi.setTxPower(WIFI_POWER_5dBm);  // Even lower power for stability
    WiFi.setSleep(false);  // Disable WiFi sleep for consistency
    
    // Set specific channel to avoid interference
    uint8_t channel = 6;  // Channel 6 usually has less interference
    
    addDebugLog("WiFi power set to 5dBm for maximum stability");
    
    // Start AP mode immediately
    bool apStarted = false;
    if(strlen(config.password) == 0) {
        // Open AP (no password) with specific channel and low power
        apStarted = WiFi.softAP(config.ssid, "", channel, 0, 4);  // Max 4 connections
        addDebugLog("Starting open WiFi AP on channel " + String(channel));
    } else {
        // Protected AP with password, specific channel
        apStarted = WiFi.softAP(config.ssid, config.password, channel, 0, 4);
        addDebugLog("Starting protected WiFi AP on channel " + String(channel));
    }
    
    if(apStarted) {
        IPAddress IP = WiFi.softAPIP();
        
        // Additional stability settings
        WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
        
        addDebugLog("AP started! SSID: " + String(config.ssid));
        addDebugLog("AP IP: " + IP.toString() + " on channel " + String(channel));
        addDebugLog("Power: 5dBm, Max connections: 4");
        
        debugPrint("AP started! SSID: ");
        debugPrintln(config.ssid);
        debugPrint("AP IP: ");
        debugPrintln(IP.toString());
        debugPrintln("WiFi optimized for close-range stability");
    } else {
        addDebugLog("ERROR: Failed to start AP!");
        debugPrintln("ERROR: Failed to start AP!");
    }
}

void initWebServer() {
    // Complete HTML with all features
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Floppy Audio</title>"
            "<meta name='viewport' content='width=device-width'>"
            "<style>"
            "body{font-family:Arial;margin:20px;background:#f5f5f5}"
            "h1,h2{color:#333}"
            "button{padding:15px 30px;margin:10px;font-size:18px;border:none;border-radius:5px;cursor:pointer}"
            ".play{background:#4CAF50;color:white}"
            ".stop{background:#f44336;color:white}"
            ".upload{background:#2196F3;color:white}"
            ".delete{background:#ff9800;color:white;padding:5px 10px;font-size:14px}"
            "input[type=file],select{margin:10px 0;padding:10px;width:100%;max-width:400px}"
            "input[type=range]{width:100%;max-width:400px}"
            ".filelist{margin:20px 0;padding:10px;background:white;border-radius:5px}"
            ".file-item{display:flex;justify-content:space-between;align-items:center;padding:8px;border-bottom:1px solid #eee}"
            ".config-section{background:white;padding:15px;margin:20px 0;border-radius:5px}"
            "label{display:block;margin:10px 0;font-weight:bold}"
            "</style>"
            "</head><body>"
            "<h1>🎵 Floppy Audio Simulator</h1>"
            
            // Configuration
            "<div class='config-section'>"
            "<h2>⚙️ Configuration</h2>"
            "<form action='/config' method='post'>"
            
            "<label>Volume: <span id='volVal'>" + String(config.volume) + "</span>%</label>"
            "<input type='range' name='volume' min='0' max='100' value='" + String(config.volume) + "' "
            "oninput=\"document.getElementById('volVal').textContent=this.value\"><br>"
            
            "<label>Gain:</label>"
            "<select name='gain'>"
            "<option value='0'" + String(config.highGain ? "" : " selected") + ">Low (9dB)</option>"
            "<option value='1'" + String(config.highGain ? " selected" : "") + ">High (15dB)</option>"
            "</select><br>"
            
            "<label>Playback Mode:</label>"
            "<select name='randomPlay'>"
            "<option value='1'" + String(config.randomPlay ? " selected" : "") + ">Random</option>"
            "<option value='0'" + String(config.randomPlay ? "" : " selected") + ">Sequential</option>"
            "</select><br>"
            
            "<label>Startup Sound:</label>"
            "<select name='startupSound'>"
            "<option value=''>None</option>";
        
        for(const String& file : soundFiles) {
            String selected = (String(config.startupSound) == file) ? " selected" : "";
            html += "<option value='" + file + "'" + selected + ">" + file + "</option>";
        }
        
        html += "</select><br>"
            
            "<label>Button Trigger Sound:</label>"
            "<select name='triggerSound'>"
            "<option value=''" + String((strlen(config.triggerSound) == 0) ? " selected" : "") + ">Random</option>";
        
        for(const String& file : soundFiles) {
            String selected = (String(config.triggerSound) == file) ? " selected" : "";
            html += "<option value='" + file + "'" + selected + ">" + file + "</option>";
        }
        
        html += "</select><br><br>"
            
            "<label>Floppy Delay Boot Sound (ms): <span id='delayBootVal'>" + String(config.delayDiskBoot) + "</span></label>"
            "<input type='range' name='delayDiskBoot' min='0' max='10000' step='100' value='" + String(config.delayDiskBoot) + "' "
            "oninput=\"document.getElementById('delayBootVal').textContent=this.value\"><br>"
            
            "<label>Floppy Delay Trigger Sound (ms): <span id='delayTriggerVal'>" + String(config.delayDiskTrigger) + "</span></label>"
            "<input type='range' name='delayDiskTrigger' min='0' max='10000' step='100' value='" + String(config.delayDiskTrigger) + "' "
            "oninput=\"document.getElementById('delayTriggerVal').textContent=this.value\"><br><br>"
            
            "<label>Debug Output:</label>"
            "<select name='debugOutput'>"
            "<option value='1'" + String(config.debugOutput ? " selected" : "") + ">Enabled</option>"
            "<option value='0'" + String(config.debugOutput ? "" : " selected") + ">Disabled</option>"
            "</select><br><br>"
            
            "<button class='upload' type='submit'>💾 Save Config</button>"
            "</form>"
            "</div>"
            
            // Debug log section
            "<hr>"
            "<h2>🐛 Debug Log</h2>"
            "<div style='background:black;color:lime;padding:10px;font-family:monospace;font-size:12px;max-height:200px;overflow-y:auto'>"
            + debugLog +
            "</div>"
            
            // Upload section
            "<hr>"
            "<h2>Upload WAV File</h2>"
            "<form action='/upload' method='post' enctype='multipart/form-data'>"
            "<input type='file' name='file' accept='.wav' required><br>"
            "<button class='upload' type='submit'>📤 Upload</button>"
            "</form>"
            
            // File list
            "<hr>"
            "<h2>Files on Device (" + String(soundFiles.size()) + ")</h2>"
            "<div class='filelist'>";
        
        if(soundFiles.size() == 0) {
            html += "<p><em>No WAV files uploaded yet</em></p>";
        } else {
            for(const String& file : soundFiles) {
                html += "<div class='file-item'>"
                    "<span>🎵 " + file + "</span>"
                    "<div>"
                    "<form action='/playfile' method='post' style='display:inline;margin:0 5px'>"
                    "<input type='hidden' name='file' value='" + file + "'>"
                    "<button class='play' type='submit' style='padding:5px 10px;font-size:14px'>▶ Play</button>"
                    "</form>"
                    "<form action='/delete' method='post' style='display:inline;margin:0'>"
                    "<input type='hidden' name='file' value='" + file + "'>"
                    "<button class='delete' type='submit'>🗑️ Delete</button>"
                    "</form>"
                    "</div>"
                    "</div>";
            }
        }
        
        html += "</div></body></html>";
        request->send(200, "text/html", html);
    });

    // Config save endpoint
    server.on("/config", HTTP_POST, [](AsyncWebServerRequest *request){
        if(request->hasParam("volume", true)) {
            config.volume = request->getParam("volume", true)->value().toInt();
        }
        if(request->hasParam("gain", true)) {
            config.highGain = request->getParam("gain", true)->value().toInt() == 1;
            digitalWrite(I2S_GAIN, config.highGain ? HIGH : LOW);
        }
        if(request->hasParam("randomPlay", true)) {
            config.randomPlay = request->getParam("randomPlay", true)->value().toInt() == 1;
        }
        if(request->hasParam("startupSound", true)) {
            String startup = request->getParam("startupSound", true)->value();
            strncpy(config.startupSound, startup.c_str(), sizeof(config.startupSound) - 1);
        }
        if(request->hasParam("triggerSound", true)) {
            String trigger = request->getParam("triggerSound", true)->value();
            strncpy(config.triggerSound, trigger.c_str(), sizeof(config.triggerSound) - 1);
        }
        if(request->hasParam("delayDiskBoot", true)) {
            config.delayDiskBoot = request->getParam("delayDiskBoot", true)->value().toInt();
        }
        if(request->hasParam("delayDiskTrigger", true)) {
            config.delayDiskTrigger = request->getParam("delayDiskTrigger", true)->value().toInt();
        }
        if(request->hasParam("debugOutput", true)) {
            config.debugOutput = request->getParam("debugOutput", true)->value().toInt() == 1;
        }
        
        saveConfiguration();
        request->redirect("/");
    });

    // Delete file endpoint
    server.on("/delete", HTTP_POST, [](AsyncWebServerRequest *request){
        if(request->hasParam("file", true)) {
            String filename = request->getParam("file", true)->value();
            String filepath = "/" + filename;
            
            // Simple direct file deletion
            if(SPIFFS.exists(filepath)) {
                bool deleted = SPIFFS.remove(filepath);
                addDebugLog(deleted ? ("Deleted: " + filename) : ("Failed to delete: " + filename));
            } else {
                addDebugLog("File not found: " + filename);
            }
            
            // Invalidate cache and refresh
            fileCacheValid = false;
            delay(50); // Brief delay for filesystem
            scanSoundFiles();  // Refresh file list
        }
        request->redirect("/");
    });

    // Play specific file endpoint - CRITICAL: respond immediately, play asynchronously
    server.on("/playfile", HTTP_POST, [](AsyncWebServerRequest *request){
        String fileToPlay = "";
        if(request->hasParam("file", true)) {
            fileToPlay = request->getParam("file", true)->value();
        }
        
        // Send response IMMEDIATELY
        request->send(200, "text/html", 
            "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
            "<meta http-equiv='refresh' content='1;url=/'>"
            "<title>Playing...</title></head><body>"
            "<h2>▶ Playing: " + fileToPlay + "</h2>"
            "<p>Redirecting...</p>"
            "</body></html>");
        
        // NOW play the sound (after response sent)
        if(fileToPlay.length() > 0) {
            // Small delay to ensure response is sent
            delay(50);
            playSound(fileToPlay);
        }
    });
    
    // File upload handler - IMPORTANT: respond immediately to avoid timeout
    server.on("/upload", HTTP_POST, 
        [](AsyncWebServerRequest *request) {
            // Send response immediately to avoid timeout
            request->send(200, "text/html", 
                "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
                "<meta http-equiv='refresh' content='2;url=/'>"
                "<title>Upload Complete</title></head><body>"
                "<h2>✓ Upload Complete</h2>"
                "<p>File uploaded successfully! Redirecting...</p>"
                "</body></html>");
        }, 
        handleFileUpload
    );
    
    // Add connection monitoring
    server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "pong");
    });
    
    // Add WiFi status endpoint
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
        String status = "WiFi: " + String(WiFi.softAPgetStationNum()) + " clients, ";
        status += "Channel: " + String(WiFi.channel()) + ", ";
        status += "Power: 5dBm, ";
        status += "Free Heap: " + String(ESP.getFreeHeap()) + " bytes";
        request->send(200, "text/plain", status);
    });
    
    server.begin();
    addDebugLog("Web server started with stability optimizations");
    debugPrintln("Web server started with connection monitoring");
}

void loadConfiguration() {
    preferences.begin("floppy-sim", false);
    
    preferences.getString("ssid", config.ssid, sizeof(config.ssid));
    preferences.getString("password", config.password, sizeof(config.password));
    config.volume = preferences.getInt("volume", 50);
    config.randomPlay = preferences.getBool("randomPlay", true);

    config.highGain = preferences.getBool("highGain", false);
    preferences.getString("currentSound", config.currentSound, sizeof(config.currentSound));
    preferences.getString("startupSound", config.startupSound, sizeof(config.startupSound));
    preferences.getString("triggerSound", config.triggerSound, sizeof(config.triggerSound));
    config.delayDiskBoot = preferences.getInt("delayBoot", 0);
    config.delayDiskTrigger = preferences.getInt("delayTrigger", 0);
    config.debugOutput = preferences.getBool("debugOutput", true);
    
    preferences.end();
    
    // Debug-Ausgabe der geladenen Delay-Werte
    debugPrint("Loaded delayDiskBoot: ");
    debugPrintln(String(config.delayDiskBoot));
    debugPrint("Loaded delayDiskTrigger: ");
    debugPrintln(String(config.delayDiskTrigger));
}

void saveConfiguration() {
    preferences.begin("floppy-sim", false);
    
    preferences.putString("ssid", config.ssid);
    preferences.putString("password", config.password);
    preferences.putInt("volume", config.volume);
    preferences.putBool("randomPlay", config.randomPlay);

    preferences.putBool("highGain", config.highGain);
    preferences.putString("currentSound", config.currentSound);
    preferences.putString("startupSound", config.startupSound);
    preferences.putString("triggerSound", config.triggerSound);
    preferences.putInt("delayBoot", config.delayDiskBoot);
    preferences.putInt("delayTrigger", config.delayDiskTrigger);
    preferences.putBool("debugOutput", config.debugOutput);
    
    preferences.end();
}

void updateDisplay() {
    if(millis() - lastAnimUpdate < 500) return; // Update every 500ms for small display
    
    lastAnimUpdate = millis();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    
    if(isPlaying) {
        // Playing state - compact layout for 72x40
        u8g2.drawStr(OLED_OFFSET_X, OLED_OFFSET_Y + 10, "PLAY");
        
        // Show short filename
        String shortName = String(config.currentSound);
        if(shortName.length() > 10) {
            shortName = shortName.substring(0, 8) + "..";
        }
        u8g2.drawStr(OLED_OFFSET_X, OLED_OFFSET_Y + 22, shortName.c_str());
        
        // Simple animation
        animFrame++;
        if(animFrame > 3) animFrame = 0;
        
        char animStr[8] = "";
        for(int i = 0; i <= animFrame; i++) {
            strcat(animStr, ">");
        }
        u8g2.drawStr(OLED_OFFSET_X, OLED_OFFSET_Y + 34, animStr);
        
    } else {
        // Ready state
        u8g2.drawStr(OLED_OFFSET_X, OLED_OFFSET_Y + 10, "FDD-SIM");
        u8g2.drawStr(OLED_OFFSET_X, OLED_OFFSET_Y + 22, "Ready");
        
        // Show WiFi AP status
        if(WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
            u8g2.drawStr(OLED_OFFSET_X, OLED_OFFSET_Y + 34, "AP:ON");
        } else {
            u8g2.drawStr(OLED_OFFSET_X, OLED_OFFSET_Y + 34, "WiFi:OFF");
        }
    }
    
    u8g2.sendBuffer();
}

void playSound(const String& filename) {
    // Check if audio is ready
    if (!audioHardwareInitialized) {
        addDebugLog("ERROR: Audio not initialized!");
        debugPrintln("ERROR: Audio hardware not initialized!");
        return;
    }
    
    addDebugLog("Playing: " + filename);
    debugPrintln("Audio hardware is ready, starting playback...");
    
    // Amplifier should already be enabled from initAudio()
    // But ensure it's on and set gain
    digitalWrite(I2S_SD, HIGH);
    digitalWrite(I2S_GAIN, config.highGain ? HIGH : LOW);
    
    // SIMPLIFIED: Direct blocking playback
    audioStopRequested = false;
    
    // Update status for UI feedback
    currentAudioFile = filename;
    audioSimulationActive = true;
    audioSimulationStart = millis();
    isPlaying = true;
    playStartTime = millis();
    
    debugPrintf("Playing (blocking): %s\n", filename.c_str());
    
    // Start floppy animation - trigger sounds always use trigger delay
    startFloppyAnimation(false);
    
    // Play directly (will block until done)
    String filepath = "/" + filename;
    playWavFileBlocking(filepath);
    
    // Stop floppy animation
    stopFloppyAnimation();
    
    // Cleanup after playback
    audioSimulationActive = false;
    isPlaying = false;
    currentAudioFile = "";
    debugPrintln("Playback finished");
}

// Direct WAV file player (blocking - will freeze UI during playback)
void playWavFileBlocking(const String& filepath) {
    // Simplified filesystem access - no mutex
    isPlayingAudio = true;  // Mark that we're playing
    
    if(!SPIFFS.exists(filepath)) {
        addDebugLog("WAV file not found: " + filepath);
        isPlayingAudio = false;
        return;
    }
    
    // Open WAV file
    File file = SPIFFS.open(filepath, "r");
    if(!file) {
        addDebugLog("Failed to open WAV file: " + filepath);
        isPlayingAudio = false;
        return;
    }
    
    size_t fileSize = file.size();
    debugPrintf("Playing WAV file: %s (%d bytes)\n", filepath.c_str(), fileSize);
    
    // Read WAV header (44 bytes for standard WAV)
    uint8_t header[44];
    if(file.read(header, 44) != 44) {
        addDebugLog("Failed to read WAV header");
        file.close();
        isPlayingAudio = false;
        return;
    }
    
    // Parse WAV header
    // Check for "RIFF" signature
    if(memcmp(header, "RIFF", 4) != 0) {
        addDebugLog("Invalid WAV file - missing RIFF header");
        file.close();
        isPlayingAudio = false;
        return;
    }
    
    // Check for "WAVE" format
    if(memcmp(header + 8, "WAVE", 4) != 0) {
        addDebugLog("Invalid WAV file - not WAVE format");
        file.close();
        isPlayingAudio = false;
        return;
    }
    
    // Extract audio parameters
    uint16_t audioFormat = header[20] | (header[21] << 8);
    uint16_t numChannels = header[22] | (header[23] << 8);
    uint32_t sampleRate = header[24] | (header[25] << 8) | (header[26] << 16) | (header[27] << 24);
    uint16_t bitsPerSample = header[34] | (header[35] << 8);
    
    addDebugLog("WAV: " + String(audioFormat) + " format, " + String(numChannels) + "ch, " + 
                String(sampleRate) + "Hz, " + String(bitsPerSample) + "bit");
    
    // Validate format (only support PCM 16-bit)
    if(audioFormat != 1) {
        addDebugLog("ERROR: Unsupported audio format - only PCM supported");
        file.close();
        isPlayingAudio = false;
        return;
    }
    
    if(bitsPerSample != 16) {
        addDebugLog("ERROR: Unsupported bit depth - only 16-bit supported");
        file.close();
        isPlayingAudio = false;
        return;
    }
    
    // CRITICAL: Reconfigure I2S for correct sample rate!
    if(sampleRate != I2S_SAMPLE_RATE) {
        addDebugLog("Adjusting I2S from " + String(I2S_SAMPLE_RATE) + "Hz to " + String(sampleRate) + "Hz");
        
        // Stop current I2S
        i2s_stop(I2S_NUM_0);
        
        // Set new sample rate
        i2s_set_sample_rates(I2S_NUM_0, sampleRate);
        
        // Restart I2S
        i2s_start(I2S_NUM_0);
        
        delay(100);  // Let I2S stabilize with new rate
    }
    
    // Enable audio amplifier
    digitalWrite(I2S_SD, HIGH);
    delay(50);  // Let amplifier stabilize
    
    // Read and play audio data in chunks
    // REDUCED buffer size to avoid heap fragmentation and OOM crashes
    const size_t CHUNK_SIZE = 256;  // 256 samples = 512 bytes (reduced from 512)
    int16_t* buffer = (int16_t*)malloc(CHUNK_SIZE * sizeof(int16_t));
    
    if(!buffer) {
        debugPrintln("Failed to allocate audio buffer");
        addDebugLog("ERROR: Out of memory for audio buffer!");
        file.close();
        digitalWrite(I2S_SD, LOW);
        isPlayingAudio = false;
        return;
    }
    
    debugPrintf("Audio buffer allocated: %d bytes (free heap: %d)\n", 
                  CHUNK_SIZE * sizeof(int16_t), ESP.getFreeHeap());
    
    size_t totalSamplesPlayed = 0;
    size_t bytesRead;
    
    while((bytesRead = file.read((uint8_t*)buffer, CHUNK_SIZE * sizeof(int16_t))) > 0) {
        // Check for stop request
        if(audioStopRequested) {
            debugPrintln("Playback stopped by user");
            break;
        }
        
        size_t samplesRead = bytesRead / sizeof(int16_t);
        
        // Simple volume control - no format conversion needed
        // Apply volume control to mono samples
        for(size_t i = 0; i < samplesRead; i++) {
            buffer[i] = (int16_t)(buffer[i] * config.volume / 100);
        }
        
        // Write same mono data to both stereo channels separately (no buffer manipulation)
        size_t bytesWritten;
        for(size_t i = 0; i < samplesRead; i++) {
            int16_t stereoSample[2] = {buffer[i], buffer[i]}; // Left = Right
            i2s_write(I2S_NUM_0, stereoSample, sizeof(stereoSample), &bytesWritten, portMAX_DELAY);
        }
        
        totalSamplesPlayed += samplesRead;
        
        // Yield to other tasks periodically AND feed watchdog
        if(totalSamplesPlayed % 2048 == 0) {  // Every 2048 samples
            delay(5);
            yield();
            
            // CRITICAL: Reset watchdog timer to prevent abort
            esp_task_wdt_reset();
            
            // *** CHECK IF WE SHOULD SHOW FLOPPY (only once after delay) ***
            if(showFloppy && !floppyDisplayed) {
                unsigned long now = millis();
                unsigned long delayTime = isBootSound ? config.delayDiskBoot : config.delayDiskTrigger;
                
                debugPrintf("DELAY CHECK: isBoot=%d, delayTime=%lu, elapsed=%lu\n", isBootSound, delayTime, now - floppyAnimStart);
                
                if((now - floppyAnimStart) >= delayTime) {
                    // Show floppy now - centered on 128x64 display
                    // X: (128-72)/2 = 28, Y: (64-40)/2 = 12
                    u8g2.clearBuffer();
                    drawFloppyDisk(28, 23, false);
                    u8g2.sendBuffer();
                    floppyDisplayed = true;
                    debugPrintf(">>> FLOPPY DISPLAYED (isBoot=%d, delay=%lu)\n", isBootSound, delayTime);
                }
            }
            
            // Check heap status to prevent crashes
            if(ESP.getFreeHeap() < 10000) {
                debugPrintln("WARNING: Low heap during playback: " + String(ESP.getFreeHeap()));
            }
        }
    }
    
    free(buffer);
    file.close();
    
    debugPrintf("Playback complete: %d samples, free heap: %d bytes\n", 
                  totalSamplesPlayed, ESP.getFreeHeap());
    
    // Cleanup after audio playback
    isPlayingAudio = false;
    
    // Clear I2S buffer
    i2s_zero_dma_buffer(I2S_NUM_0);
    
    // Disable amplifier to save power
    digitalWrite(I2S_SD, LOW);
    
    debugPrintf("WAV playback complete - played %d samples (filesystem unlocked)\n", totalSamplesPlayed);
}

void stopSound() {
    // SIMPLIFIED: Direct stop (no mutex)
    audioStopRequested = true;
    audioSimulationActive = false;
    isPlaying = false;
    currentAudioFile = "";
    strcpy(config.currentSound, "");
    
    debugPrintln("Audio stop requested");
}

void testAudioAmplifier() {
    addDebugLog("=== STARTUP BEEP (3kHz, 10% vol) ===");
    debugPrintln("=== STARTUP BEEP (3000 Hz, 10% volume) ===");
    
    if (!audioHardwareInitialized) {
        addDebugLog("ERROR: Audio hardware not ready!");
        debugPrintln("Error: Audio hardware not initialized!");
        return;
    }
    
    // Enable amplifier
    addDebugLog("Enabling amp + low gain...");
    debugPrintln("Enabling amplifier (I2S_SD = HIGH)...");
    digitalWrite(I2S_SD, HIGH);
    digitalWrite(I2S_GAIN, LOW);  // Low gain for startup beep
    delay(100); // Let amplifier stabilize
    addDebugLog("Amp ready, generating sine wave...");
    debugPrintln("Amplifier enabled, generating beep...");
    
    // Generate 1kHz beep (500ms, 10% volume) - longer and lower for easier hearing
    int sampleCount = I2S_SAMPLE_RATE * 50 / 100;  // 0.5 seconds
    int16_t* samples = (int16_t*)malloc(sampleCount * sizeof(int16_t));
    
    if (!samples) {
        addDebugLog("ERROR: Buffer allocation failed!");
        debugPrintln("ERROR: Failed to allocate audio buffer!");
        return;
    }
    
    addDebugLog("Allocated " + String(sampleCount) + " samples");
    debugPrintf("Generating %d samples...\n", sampleCount);
    
    float phaseIncrement = 2.0 * PI * 1000 / I2S_SAMPLE_RATE;  // 1kHz (lower frequency)
    float phase = 0.0;
    
    // Generate sine wave with envelope (fade in/out to avoid click)
    for (int i = 0; i < sampleCount; i++) {
        float sineValue = sin(phase);
        
        // Simple envelope: fade in first 10%, fade out last 10%
        float envelope = 1.0;
        if (i < sampleCount / 10) {
            envelope = (float)i / (sampleCount / 10);
        } else if (i > sampleCount * 9 / 10) {
            envelope = (float)(sampleCount - i) / (sampleCount / 10);
        }
        
        samples[i] = (int16_t)(sineValue * envelope * 3000);  // 30% volume (much louder!)
        phase += phaseIncrement;
        if (phase >= 2.0 * PI) phase -= 2.0 * PI;
    }
    
    // Play via I2S
    addDebugLog("Writing " + String(sampleCount) + " samples to I2S...");
    debugPrintln("Writing to I2S...");
    size_t bytesWritten;
    esp_err_t result = i2s_write(I2S_NUM_0, samples, sampleCount * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
    addDebugLog("I2S result: " + String(result) + ", wrote: " + String(bytesWritten) + " bytes");
    debugPrintf("I2S write result: %d, bytes written: %d\n", result, bytesWritten);
    
    free(samples);
    
    // Keep amplifier enabled for future playback
    addDebugLog("Startup beep completed!");
    debugPrintln("Startup beep complete - audio system ready!");
}

void setGain(bool highGain) {
    config.highGain = highGain;
    digitalWrite(I2S_GAIN, highGain ? HIGH : LOW);
    debugPrintf("Audio gain set to %s (%ddB)\n", 
                  highGain ? "HIGH" : "LOW", 
                  highGain ? 15 : 9);
    saveConfiguration();  // Save to preferences
}


void checkTriggerButton() {
    bool currentButtonState = digitalRead(BUTTON_PIN);
    unsigned long currentTime = millis();
    
    // Button pressed (HIGH to LOW transition with debounce)
    if (lastButtonState == HIGH && currentButtonState == LOW && 
        (currentTime - lastButtonPress) > BUTTON_DEBOUNCE) {
        
        lastButtonPress = currentTime;
        addDebugLog("Button pressed - trigger sound");
        
        // Play configured trigger sound or random
        if(soundFiles.size() > 0) {
            String soundToPlay;
            
            if(strlen(config.triggerSound) > 0) {
                // Check if configured trigger sound exists
                bool soundExists = false;
                for(const String& file : soundFiles) {
                    if(file == String(config.triggerSound)) {
                        soundExists = true;
                        soundToPlay = file;
                        break;
                    }
                }
                if(!soundExists) {
                    soundToPlay = getRandomSound();
                    addDebugLog("Trigger sound not found, playing random");
                }
            } else {
                soundToPlay = getRandomSound();
                addDebugLog("Playing random trigger sound");
            }
            
            if(soundToPlay.length() > 0) {
                // Don't call startFloppyAnimation here - playSound() will do it
                playSound(soundToPlay);
                // Don't call stopFloppyAnimation here - playSound() will do it
            }
        } else {
            addDebugLog("No sound files available");
        }
    }
    
    lastButtonState = currentButtonState;
}

void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    static File uploadFile;
    static String currentUploadFile;
    
    if(!index) {
        // Start of upload
        debugPrintln("Upload start: " + filename);
        currentUploadFile = filename;
        
        // Open file for writing
        String filepath = "/" + filename;
        uploadFile = SPIFFS.open(filepath, "w");
        if(!uploadFile) {
            debugPrintln("ERROR: Failed to open file for writing: " + filepath);
            return;
        }
        debugPrintln("File opened: " + filepath);
    }
    
    // Write data chunk
    if(uploadFile && len > 0) {
        size_t written = uploadFile.write(data, len);
        if(written != len) {
            debugPrintln("WARNING: Write incomplete " + String(written) + "/" + String(len));
        }
        yield(); // Give system time to breathe
    }
    
    if(final) {
        // End of upload
        if(uploadFile) {
            uploadFile.close();
            debugPrintln("Upload complete: " + currentUploadFile + " (" + String(index + len) + " bytes)");
            
            // Quick refresh of file list
            fileCacheValid = false;
            scanSoundFiles();
        } else {
            debugPrintln("ERROR: Upload file was null on final");
        }
        currentUploadFile = "";
    }
}

void scanSoundFiles() {
    // Check if we need to refresh the cache
    unsigned long now = millis();
    if (fileCacheValid && (now - lastFileScan) < FILE_CACHE_TIMEOUT) {
        addDebugLog("Using cached file list (" + String(soundFiles.size()) + " files)");
        return;
    }
    
    // Fast filesystem scan with caching
    soundFiles.clear();
    
    File root = SPIFFS.open("/");
    if(!root) {
        addDebugLog("Failed to open root directory");
        return;
    }
    
    File file = root.openNextFile();
    while(file) {
        String filename = file.name();
        filename.toLowerCase(); // Make case-insensitive
        if(filename.endsWith(".wav") || filename.endsWith(".mp3")) {
            soundFiles.push_back(file.name()); // Keep original case for filename
        }
        file = root.openNextFile();
    }
    
    fileCacheValid = true;
    lastFileScan = now;
    addDebugLog("Scanned filesystem: " + String(soundFiles.size()) + " sound files");
}

String getRandomSound() {
    if(soundFiles.size() == 0) return "";
    int index = random(0, soundFiles.size());
    return soundFiles[index];
}

