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
#include <SPI.h>
#include <MFRC522.h>
#include <driver/i2s.h>
#include <math.h>

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

// NFC pins for PN532 - adjusted for available pins
#define NFC_RST 2    // Available GPIO
#define NFC_CS 10    // Available GPIO

// Control pins - updated after audio pin reassignment
#define TRIGGER_PIN 9    // Available GPIO
#define STATUS_LED 8     // Onboard LED

// Display object - WORKING U8G2 configuration for diymore ESP32-C3 Super Mini
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, OLED_SCL, OLED_SDA);

// NFC Reader
MFRC522 nfc(NFC_CS, NFC_RST);

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
    bool nfcEnabled = true;
    char currentSound[64] = "default.wav";
    char startupSound[64] = "";  // Startup sound (empty = no sound)
    char triggerSound[64] = "";  // Button trigger sound (empty = random)
    bool highGain = false;  // false = 9dB, true = 15dB
} config;

// Animation variables
unsigned long lastAnimUpdate = 0;
int animFrame = 0;
bool isPlaying = false;
unsigned long playStartTime = 0;

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
    // Keep only last 20 lines
    int lineCount = 0;
    int lastBr = debugLog.length();
    for(int i = debugLog.length() - 1; i >= 0; i--) {
        if(debugLog[i] == '>') {  // End of <br>
            lineCount++;
            if(lineCount > 20) {
                debugLog = debugLog.substring(i + 1);
                break;
            }
        }
    }
}

// Function declarations
void initDisplay();
void initAudio();
void initNFC();
void initWiFi();
void initWebServer();
void loadConfiguration();
void saveConfiguration();
void updateDisplay();


void playSound(const String& filename);
void stopSound();
void setGain(bool highGain);
void testAudioAmplifier();
void checkNFC();
void checkTriggerButton();
void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
void scanSoundFiles();
String getRandomSound();
void playWavFileBlocking(const String& filepath);  // Direct WAV player (blocking)

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n=== Floppy Audio Simulator v1.0 ===");
    
    // Initialize pins
    pinMode(STATUS_LED, OUTPUT);
    digitalWrite(STATUS_LED, HIGH);
    
    // Initialize button pin (pull-up, button connects to GND)
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    addDebugLog("Button pin initialized (GPIO " + String(BUTTON_PIN) + ")");
    
    // Initialize SPIFFS - faster than LittleFS
    Serial.println("Initializing SPIFFS...");
    if(!SPIFFS.begin(true)) {
        Serial.println("ERROR: SPIFFS Mount Failed!");
    } else {
        Serial.println("SPIFFS mounted successfully");
    }
    
    // Filesystem access simplified - no mutex needed
    
    // Load configuration
    Serial.println("Loading configuration...");
    loadConfiguration();
    
    // Scan for WAV files
    Serial.println("Scanning for WAV files...");
    scanSoundFiles();
    
    // Initialize WiFi
    Serial.println("Initializing WiFi...");
    initWiFi();
    
    // Initialize Web server
    Serial.println("Initializing web server...");
    initWebServer();
    
    // Initialize Audio directly (like it worked before)
    Serial.println("Initializing audio...");
    addDebugLog("Initializing audio hardware...");
    initAudio();
    
    if(audioHardwareInitialized) {
        addDebugLog("Audio initialized successfully!");
        
        // Play configured startup sound or default beep
        if(strlen(config.startupSound) > 0 && soundFiles.size() > 0) {
            // Check if configured startup sound exists
            bool soundExists = false;
            for(const String& file : soundFiles) {
                if(file == String(config.startupSound)) {
                    soundExists = true;
                    break;
                }
            }
            if(soundExists) {
                addDebugLog("Playing startup sound: " + String(config.startupSound));
                playSound(String(config.startupSound));
            } else {
                addDebugLog("Startup sound not found, playing beep");
                testAudioAmplifier();
            }
        } else {
            Serial.println("Playing startup beep...");
            addDebugLog("Playing startup beep (no custom sound configured)");
            testAudioAmplifier();
        }
        addDebugLog("Startup audio complete");
    } else {
        addDebugLog("ERROR: Audio init failed!");
    }
    
    addDebugLog("System ready!");
    Serial.println("=== READY ===");
    digitalWrite(STATUS_LED, LOW);
}

void loop() {
    // Check button for trigger sound
    checkTriggerButton();
    
    // WiFi stability monitoring every 30 seconds
    static unsigned long lastWiFiCheck = 0;
    if (millis() - lastWiFiCheck > 30000) {
        lastWiFiCheck = millis();
        
        // Log WiFi status for debugging
        uint8_t clients = WiFi.softAPgetStationNum();
        if (clients > 0) {
            addDebugLog("WiFi: " + String(clients) + " client(s) connected");
        }
        
        // Memory check
        uint32_t freeHeap = ESP.getFreeHeap();
        if (freeHeap < 10000) {  // Less than 10KB free
            addDebugLog("WARNING: Low memory: " + String(freeHeap) + " bytes");
        }
    }
    
    // Yield to WiFi stack and watchdog
    yield();
    delay(10);  // Optimized for button responsiveness and WiFi stability
}

void initDisplay() {
    Serial.println("Starting 0.42 inch OLED with U8G2...");
    Serial.printf("Pins: SCL=%d, SDA=%d, Offsets: X=%d, Y=%d\n", OLED_SCL, OLED_SDA, OLED_OFFSET_X, OLED_OFFSET_Y);
    
    // Initialize U8G2 display - this handles I2C automatically
    u8g2.begin();
    Serial.println("U8G2 display initialized successfully!");
    
    // Show startup message with proper offsets
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    
    u8g2.drawStr(OLED_OFFSET_X, OLED_OFFSET_Y + 12, "FDD-SIM");
    u8g2.drawStr(OLED_OFFSET_X, OLED_OFFSET_Y + 28, "Starting...");
    
    u8g2.sendBuffer();
    Serial.println("Startup message displayed");
    
    // Removed 2 second delay for faster startup
    
    Serial.println("Display initialization complete");
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
    Serial.println("Initializing I2S audio output for MAX98357...");
    
    // I2S configuration - MINIMAL for fast init
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
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
        Serial.printf("I2S driver install FAILED: %d\n", result);
        return;
    }
    addDebugLog("I2S driver installed OK");
    
    result = i2s_set_pin(I2S_NUM_0, &pin_config);
    if (result != ESP_OK) {
        addDebugLog("I2S pin setup FAILED: " + String(result));
        Serial.printf("I2S pin setup FAILED: %d\n", result);
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
    Serial.println("I2S Audio ready - AMPLIFIER ENABLED");
    
    // SIMPLIFIED: No mutex, no task - direct blocking playback
    Serial.println("Using direct blocking playback");
}

void initNFC() {
    SPI.begin();
    nfc.PCD_Init();
    
    // Check if NFC reader is connected
    byte version = nfc.PCD_ReadRegister(nfc.VersionReg);
    if(version == 0x00 || version == 0xFF) {
        Serial.println("NFC reader not found - disabling NFC features");
        config.nfcEnabled = false;
        return;
    }
    
    Serial.print("NFC reader found, version: 0x");
    Serial.println(version, HEX);
}

void initWiFi() {
    Serial.println("Starting WiFi in AP mode...");
    
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
        
        Serial.print("AP started! SSID: ");
        Serial.println(config.ssid);
        Serial.print("AP IP: ");
        Serial.println(IP);
        Serial.println("WiFi optimized for close-range stability");
    } else {
        addDebugLog("ERROR: Failed to start AP!");
        Serial.println("ERROR: Failed to start AP!");
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
        
        html += "</select><br>"
            
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

    // Play specific file endpoint
    server.on("/playfile", HTTP_POST, [](AsyncWebServerRequest *request){
        if(request->hasParam("file", true)) {
            String filename = request->getParam("file", true)->value();
            playSound(filename);
        }
        request->redirect("/");
    });
    
    // File upload handler with better error handling
    server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
        // This response is sent after the upload handler completes
        String response = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Upload Result</title></head><body>";
        response += "<h2>Upload Complete</h2>";
        response += "<p>File uploaded successfully!</p>";
        response += "<script>setTimeout(function(){window.location.href='/';}, 2000);</script>";
        response += "</body></html>";
        request->send(200, "text/html", response);
    }, handleFileUpload);
    
    // Configure server for stability
    server.onFileUpload(handleFileUpload);
    
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
    Serial.println("Web server started with connection monitoring");
}

void loadConfiguration() {
    preferences.begin("floppy-sim", false);
    
    preferences.getString("ssid", config.ssid, sizeof(config.ssid));
    preferences.getString("password", config.password, sizeof(config.password));
    config.volume = preferences.getInt("volume", 50);
    config.randomPlay = preferences.getBool("randomPlay", true);
    config.nfcEnabled = preferences.getBool("nfcEnabled", true);
    config.highGain = preferences.getBool("highGain", false);
    preferences.getString("currentSound", config.currentSound, sizeof(config.currentSound));
    preferences.getString("startupSound", config.startupSound, sizeof(config.startupSound));
    preferences.getString("triggerSound", config.triggerSound, sizeof(config.triggerSound));
    
    preferences.end();
}

void saveConfiguration() {
    preferences.begin("floppy-sim", false);
    
    preferences.putString("ssid", config.ssid);
    preferences.putString("password", config.password);
    preferences.putInt("volume", config.volume);
    preferences.putBool("randomPlay", config.randomPlay);
    preferences.putBool("nfcEnabled", config.nfcEnabled);
    preferences.putBool("highGain", config.highGain);
    preferences.putString("currentSound", config.currentSound);
    preferences.putString("startupSound", config.startupSound);
    preferences.putString("triggerSound", config.triggerSound);
    
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
        Serial.println("ERROR: Audio hardware not initialized!");
        return;
    }
    
    addDebugLog("Playing: " + filename);
    Serial.println("Audio hardware is ready, starting playback...");
    
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
    
    Serial.printf("Playing (blocking): %s\n", filename.c_str());
    
    // Play directly (will block until done)
    String filepath = "/" + filename;
    playWavFileBlocking(filepath);
    
    // Cleanup after playback
    audioSimulationActive = false;
    isPlaying = false;
    currentAudioFile = "";
    Serial.println("Playback finished");
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
    Serial.printf("Playing WAV file: %s (%d bytes)\n", filepath.c_str(), fileSize);
    
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
    
    Serial.printf("WAV Format: %d, Channels: %d, SampleRate: %d, BitsPerSample: %d\n", 
                  audioFormat, numChannels, sampleRate, bitsPerSample);
    
    // Validate format (only support PCM 16-bit)
    if(audioFormat != 1) {
        addDebugLog("Unsupported audio format - only PCM supported");
        file.close();
        isPlayingAudio = false;
        return;
    }
    
    if(bitsPerSample != 16) {
        addDebugLog("Unsupported bit depth - only 16-bit supported");
        file.close();
        isPlayingAudio = false;
        return;
    }
    
    // Enable audio amplifier
    digitalWrite(I2S_SD, HIGH);
    vTaskDelay(pdMS_TO_TICKS(50));  // Let amplifier stabilize
    
    // Read and play audio data in chunks
    const size_t CHUNK_SIZE = 512;  // 512 samples = 1024 bytes
    int16_t* buffer = (int16_t*)malloc(CHUNK_SIZE * sizeof(int16_t));
    
    if(!buffer) {
        Serial.println("Failed to allocate audio buffer");
        file.close();
        digitalWrite(I2S_SD, LOW);
        isPlayingAudio = false;
        return;
    }
    
    size_t totalSamplesPlayed = 0;
    size_t bytesRead;
    
    while((bytesRead = file.read((uint8_t*)buffer, CHUNK_SIZE * sizeof(int16_t))) > 0) {
        // Check for stop request
        if(audioStopRequested) {
            Serial.println("Playback stopped by user");
            break;
        }
        
        size_t samplesRead = bytesRead / sizeof(int16_t);
        
        // Handle volume scaling
        for(size_t i = 0; i < samplesRead; i++) {
            buffer[i] = (int16_t)(buffer[i] * config.volume / 100);
        }
        
        // Convert stereo to mono if needed
        if(numChannels == 2) {
            for(size_t i = 0; i < samplesRead / 2; i++) {
                buffer[i] = (buffer[i * 2] + buffer[i * 2 + 1]) / 2;
            }
            samplesRead /= 2;
        }
        
        // Write to I2S
        size_t bytesWritten;
        i2s_write(I2S_NUM_0, buffer, samplesRead * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
        
        totalSamplesPlayed += samplesRead;
        
        // Yield to other tasks periodically
        if(totalSamplesPlayed % 4096 == 0) {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
    
    free(buffer);
    file.close();
    
    // Cleanup after audio playback
    isPlayingAudio = false;
    
    // Clear I2S buffer
    i2s_zero_dma_buffer(I2S_NUM_0);
    
    // Disable amplifier to save power
    digitalWrite(I2S_SD, LOW);
    
    Serial.printf("WAV playback complete - played %d samples (filesystem unlocked)\n", totalSamplesPlayed);
}

void stopSound() {
    // SIMPLIFIED: Direct stop (no mutex)
    audioStopRequested = true;
    audioSimulationActive = false;
    isPlaying = false;
    currentAudioFile = "";
    strcpy(config.currentSound, "");
    
    Serial.println("Audio stop requested");
}

void testAudioAmplifier() {
    addDebugLog("=== STARTUP BEEP (3kHz, 10% vol) ===");
    Serial.println("=== STARTUP BEEP (3000 Hz, 10% volume) ===");
    
    if (!audioHardwareInitialized) {
        addDebugLog("ERROR: Audio hardware not ready!");
        Serial.println("Error: Audio hardware not initialized!");
        return;
    }
    
    // Enable amplifier
    addDebugLog("Enabling amp + low gain...");
    Serial.println("Enabling amplifier (I2S_SD = HIGH)...");
    digitalWrite(I2S_SD, HIGH);
    digitalWrite(I2S_GAIN, LOW);  // Low gain for startup beep
    delay(100); // Let amplifier stabilize
    addDebugLog("Amp ready, generating sine wave...");
    Serial.println("Amplifier enabled, generating beep...");
    
    // Generate 1kHz beep (500ms, 10% volume) - longer and lower for easier hearing
    int sampleCount = I2S_SAMPLE_RATE * 50 / 100;  // 0.5 seconds
    int16_t* samples = (int16_t*)malloc(sampleCount * sizeof(int16_t));
    
    if (!samples) {
        addDebugLog("ERROR: Buffer allocation failed!");
        Serial.println("ERROR: Failed to allocate audio buffer!");
        return;
    }
    
    addDebugLog("Allocated " + String(sampleCount) + " samples");
    Serial.printf("Generating %d samples...\n", sampleCount);
    
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
    Serial.println("Writing to I2S...");
    size_t bytesWritten;
    esp_err_t result = i2s_write(I2S_NUM_0, samples, sampleCount * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
    addDebugLog("I2S result: " + String(result) + ", wrote: " + String(bytesWritten) + " bytes");
    Serial.printf("I2S write result: %d, bytes written: %d\n", result, bytesWritten);
    
    free(samples);
    
    // Keep amplifier enabled for future playback
    addDebugLog("Startup beep completed!");
    Serial.println("Startup beep complete - audio system ready!");
}

void setGain(bool highGain) {
    config.highGain = highGain;
    digitalWrite(I2S_GAIN, highGain ? HIGH : LOW);
    Serial.printf("Audio gain set to %s (%ddB)\n", 
                  highGain ? "HIGH" : "LOW", 
                  highGain ? 15 : 9);
    saveConfiguration();  // Save to preferences
}

void checkNFC() {
    if(!nfc.PICC_IsNewCardPresent() || !nfc.PICC_ReadCardSerial()) {
        return;
    }
    
    Serial.print("NFC card detected: ");
    for(byte i = 0; i < nfc.uid.size; i++) {
        Serial.print(nfc.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(nfc.uid.uidByte[i], HEX);
    }
    Serial.println();
    
    // Trigger sound based on card
    if(config.randomPlay && soundFiles.size() > 0) {
        String randomFile = getRandomSound();
        playSound(randomFile);
    }
    
    nfc.PICC_HaltA();
    nfc.PCD_StopCrypto1();
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
                playSound(soundToPlay);
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
        addDebugLog("Upload start: " + filename);
        currentUploadFile = filename;
        
        // Simple filesystem access
        String filepath = "/" + filename;
        uploadFile = SPIFFS.open(filepath, "w");
        if(!uploadFile) {
            addDebugLog("ERROR: Failed to open file for writing: " + filepath);
            return;
        }
        addDebugLog("File opened for upload: " + filepath);
    }
    
    if(uploadFile && len > 0) {
        size_t written = uploadFile.write(data, len);
        if(written != len) {
            addDebugLog("WARNING: Write incomplete " + String(written) + "/" + String(len));
        }
    }
    
    if(final) {
        if(uploadFile) {
            uploadFile.flush(); // Ensure data is written
            uploadFile.close();
            
            addDebugLog("Upload complete: " + currentUploadFile + " (" + String(index + len) + " bytes)");
            
            // Invalidate cache and refresh after upload
            fileCacheValid = false;
            delay(50); // Reduced delay
            scanSoundFiles(); // Refresh file list
        } else {
            addDebugLog("ERROR: Upload file was null on final");
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

