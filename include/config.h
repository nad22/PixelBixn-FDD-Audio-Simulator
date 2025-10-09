// Configuration constants for Floppy Disk Audio Simulator
#ifndef CONFIG_H
#define CONFIG_H

// Hardware Configuration
#define DEVICE_NAME "Floppy Disk Audio Simulator"
#define FIRMWARE_VERSION "1.0.0"
#define HARDWARE_VERSION "1.0"

// ESP32-C3 Pin Definitions
// OLED Display (I2C)
#define OLED_SDA_PIN        8
#define OLED_SCL_PIN        9
#define OLED_WIDTH          128
#define OLED_HEIGHT         64
#define OLED_RESET_PIN      -1
#define OLED_I2C_ADDRESS    0x3C

// Audio Output (I2S to MAX98357)
#define I2S_BCLK_PIN        4
#define I2S_LRC_PIN         5
#define I2S_DOUT_PIN        6
#define I2S_SAMPLE_RATE     44100
#define I2S_BITS_PER_SAMPLE 16

// NFC Reader (SPI to PN532)
#define NFC_RST_PIN         2
#define NFC_CS_PIN          10
#define NFC_MOSI_PIN        7    // Default SPI MOSI
#define NFC_MISO_PIN        5    // Default SPI MISO (shared with I2S_LRC)
#define NFC_SCK_PIN         6    // Default SPI SCK (shared with I2S_DOUT)

// Control Pins
#define TRIGGER_BUTTON_PIN  3
#define STATUS_LED_PIN      7

// System Configuration
#define SERIAL_BAUD_RATE    115200
#define WATCHDOG_TIMEOUT    8000    // milliseconds

// WiFi Configuration
#define DEFAULT_AP_SSID     "FloppyDisk_AP"
#define DEFAULT_AP_PASSWORD "floppy123"
#define AP_CHANNEL          6
#define AP_MAX_CLIENTS      4
#define WIFI_CONNECT_TIMEOUT 10000  // milliseconds
#define WIFI_RETRY_DELAY    500     // milliseconds

// Web Server Configuration
#define WEB_SERVER_PORT     80
#define WEBSOCKET_PORT      81
#define MAX_UPLOAD_SIZE     (2 * 1024 * 1024)  // 2MB
#define API_RATE_LIMIT      60      // requests per minute
#define CORS_ENABLED        true

// Audio Configuration
#define DEFAULT_VOLUME      50      // 0-100
#define MIN_VOLUME          0
#define MAX_VOLUME          100
#define VOLUME_STEP         5
#define AUDIO_BUFFER_SIZE   2048
#define MAX_AUDIO_DURATION  30000   // milliseconds
#define SUPPORTED_FORMATS   "wav,mp3"

// File System Configuration
#define FS_PARTITION_LABEL  "littlefs"
#define MAX_FILENAME_LENGTH 64
#define MAX_FILES           100
#define RESERVED_SPACE      (512 * 1024)  // 512KB reserved

// Display Configuration
#define DISPLAY_UPDATE_RATE 100     // milliseconds
#define ANIMATION_FRAMES    8
#define DISPLAY_TIMEOUT     30000   // milliseconds until screensaver
#define BRIGHTNESS_DEFAULT  255
#define CONTRAST_DEFAULT    128

// NFC Configuration
#define NFC_UPDATE_RATE     250     // milliseconds
#define NFC_CARD_TIMEOUT    5000    // milliseconds
#define NFC_MAX_UID_SIZE    10      // bytes
#define NFC_DEBOUNCE_TIME   1000    // milliseconds

// Button Configuration
#define BUTTON_DEBOUNCE     50      // milliseconds
#define BUTTON_LONG_PRESS   2000    // milliseconds
#define BUTTON_REPEAT_RATE  500     // milliseconds

// Network Configuration  
#define HOSTNAME            "floppy-simulator"
#define MDNS_SERVICE        "_http"
#define MDNS_PROTO          "_tcp"
#define MDNS_PORT           80
#define OTA_PORT            3232
#define OTA_PASSWORD        "floppy-ota"

// Memory Management
#define MIN_FREE_HEAP       10000   // bytes
#define HEAP_WARNING_LEVEL  20000   // bytes
#define STACK_SIZE          8192    // bytes
#define TASK_PRIORITY       1

// Timing Configuration
#define MAIN_LOOP_DELAY     10      // milliseconds
#define STATUS_UPDATE_RATE  2000    // milliseconds
#define CONFIG_SAVE_DELAY   5000    // milliseconds
#define HEARTBEAT_INTERVAL  30000   // milliseconds

// Debug Configuration
#ifdef DEBUG
    #define DEBUG_LEVEL         3   // 0=Error, 1=Warning, 2=Info, 3=Debug
    #define SERIAL_DEBUG        true
    #define WEB_DEBUG           true
    #define MEMORY_DEBUG        true
#else
    #define DEBUG_LEVEL         1
    #define SERIAL_DEBUG        false
    #define WEB_DEBUG           false
    #define MEMORY_DEBUG        false
#endif

// Error Codes
#define ERROR_NONE          0
#define ERROR_INIT_FAILED   1
#define ERROR_WIFI_FAILED   2
#define ERROR_FS_FAILED     3
#define ERROR_AUDIO_FAILED  4
#define ERROR_NFC_FAILED    5
#define ERROR_DISPLAY_FAILED 6
#define ERROR_CONFIG_FAILED 7
#define ERROR_MEMORY_LOW    8
#define ERROR_FILE_NOT_FOUND 9
#define ERROR_INVALID_FORMAT 10

// Feature Flags
#define FEATURE_NFC         true
#define FEATURE_WEBSOCKET   true
#define FEATURE_OTA         true
#define FEATURE_MDNS        true
#define FEATURE_ANIMATIONS  true
#define FEATURE_API_AUTH    false   // Future feature
#define FEATURE_ENCRYPTION  false   // Future feature
#define FEATURE_LOGGING     true

// Audio Processing
#define AUDIO_FADE_TIME     500     // milliseconds
#define CROSSFADE_ENABLED   false
#define EQUALIZER_ENABLED   false
#define VOLUME_CURVE        "logarithmic"  // linear, logarithmic

// Power Management
#define SLEEP_ENABLED       false
#define SLEEP_TIMEOUT       300000  // milliseconds (5 minutes)
#define LIGHT_SLEEP_MODE    true
#define CPU_FREQ_LOW        80      // MHz for low power mode
#define CPU_FREQ_HIGH       160     // MHz for normal operation

// Security Configuration
#define MAX_LOGIN_ATTEMPTS  5
#define LOCKOUT_TIME        300000  // milliseconds (5 minutes)
#define SESSION_TIMEOUT     3600000 // milliseconds (1 hour)
#define CSRF_PROTECTION     false   // Future feature

// Logging Configuration
#define LOG_LEVEL           2       // 0=Error, 1=Warning, 2=Info, 3=Debug
#define LOG_TO_SERIAL       true
#define LOG_TO_FILE         false   // Future feature
#define LOG_MAX_SIZE        (100 * 1024)  // 100KB
#define LOG_ROTATION        true

// Validation Macros
#define VALIDATE_PIN(pin)   ((pin >= 0) && (pin <= 21))
#define VALIDATE_VOLUME(vol) ((vol >= MIN_VOLUME) && (vol <= MAX_VOLUME))
#define VALIDATE_FILENAME(name) ((strlen(name) > 0) && (strlen(name) < MAX_FILENAME_LENGTH))

// Utility Macros
#define ARRAY_SIZE(arr)     (sizeof(arr) / sizeof(arr[0]))
#define MIN(a, b)           ((a) < (b) ? (a) : (b))
#define MAX(a, b)           ((a) > (b) ? (a) : (b))
#define CLAMP(val, min, max) (MIN(MAX(val, min), max))

// String Constants
#define MIME_TYPE_JSON      "application/json"
#define MIME_TYPE_HTML      "text/html"
#define MIME_TYPE_CSS       "text/css"
#define MIME_TYPE_JS        "application/javascript"
#define MIME_TYPE_WAV       "audio/wav"
#define MIME_TYPE_MP3       "audio/mpeg"

// Default Sounds (built-in)
#define DEFAULT_SOUNDS      { \
    "floppy_seek.wav", \
    "floppy_motor.wav", \
    "floppy_read.wav", \
    "floppy_write.wav", \
    "floppy_error.wav" \
}

// HTML Templates
#define HTML_HEAD_TEMPLATE  "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>%s</title></head><body>"
#define HTML_FOOT_TEMPLATE  "</body></html>"

// CSS Classes
#define CSS_CONTAINER       "container"
#define CSS_CARD            "card"
#define CSS_BUTTON          "btn"
#define CSS_INPUT           "input"
#define CSS_LABEL           "label"

// JavaScript Events
#define JS_EVENT_PLAY       "play"
#define JS_EVENT_STOP       "stop"
#define JS_EVENT_VOLUME     "volume"
#define JS_EVENT_STATUS     "status"

#endif // CONFIG_H