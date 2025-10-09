# Floppy Disk Audio Simulator

A sophisticated ESP32-C3 based audio simulator that recreates authentic floppy disk drive sounds with modern features including web-based configuration, NFC triggering, and animated OLED display.

## 🎯 Features

- **Authentic Floppy Sounds**: Play realistic floppy disk drive sounds (seek, read, write, motor)
- **Web Interface**: Beautiful Material Design web interface for configuration and control
- **Multiple Triggers**: NFC card reader, physical button, and web interface triggering
- **High-Quality Audio**: MAX98357 Class D amplifier for crisp sound reproduction
- **Visual Feedback**: 0.42" OLED display with animated floppy disk access indicators
- **File Management**: Upload, download, delete, and organize sound files via web interface
- **WiFi Configuration**: Both AP mode and station mode support
- **Volume Control**: Adjustable volume with real-time updates
- **Random/Sequential Play**: Configurable playback modes
- **OTA Updates**: Over-the-air firmware updates support

## 🛠️ Hardware Requirements

### Main Components
- **ESP32-C3 Development Board** with 0.42" OLED display (like the diymore board)
- **MAX98357 I2S Class D Audio Amplifier**
- **PN532 V2.0 NFC RFID Module** (optional)
- **Speaker** (4-8 ohms, 3-5 watts recommended)
- **Push Button** for manual triggering
- **Breadboard/PCB** for connections

### Pin Configuration (ESP32-C3)
```
OLED Display (I2C):
- SDA: GPIO 8
- SCL: GPIO 9

MAX98357 Audio (I2S):
- BCLK: GPIO 4
- LRC:  GPIO 5  
- DIN:  GPIO 6

NFC Module (SPI):
- RST:  GPIO 2
- CS:   GPIO 10
- MOSI: GPIO 7  (default SPI)
- MISO: GPIO 5  (default SPI)
- SCK:  GPIO 6  (default SPI)

Controls:
- Trigger Button: GPIO 3 (with internal pull-up)
- Status LED:     GPIO 7
```

## 🔧 Hardware Assembly

### 1. ESP32-C3 to MAX98357 Audio Amplifier
```
ESP32-C3    MAX98357
GPIO 4  ->  BCLK
GPIO 5  ->  LRC (LRCLK)
GPIO 6  ->  DIN
3.3V    ->  VDD
GND     ->  GND
```

### 2. NFC Module Connection (Optional)
```
ESP32-C3    PN532
GPIO 10 ->  CS/SDA
GPIO 2  ->  RST
GPIO 7  ->  MOSI
GPIO 5  ->  MISO  
GPIO 6  ->  SCK
3.3V    ->  VCC
GND     ->  GND
```

### 3. Additional Components
- Connect speaker to MAX98357 output terminals
- Connect momentary push button between GPIO 3 and GND
- Status LED with 220Ω resistor between GPIO 7 and GND

## 📦 Software Installation

### Prerequisites
- [PlatformIO](https://platformio.org/) installed in VS Code
- USB-C cable for programming
- Computer with Windows/Linux/macOS

### Build and Upload
1. Open this project in PlatformIO
2. Connect ESP32-C3 via USB-C
3. Build and upload:
   ```bash
   pio run --target upload
   ```
4. Monitor serial output:
   ```bash
   pio device monitor
   ```

### Dependencies
All required libraries are automatically installed via PlatformIO:
- ESPAsyncWebServer
- ArduinoJson
- Adafruit SSD1306
- Adafruit GFX
- ESP8266Audio
- MFRC522
- LittleFS

## 🌐 Web Interface Usage

### Initial Setup
1. Power on the device
2. Connect to WiFi network "FloppyDisk_AP" (password: "floppy123")
3. Open browser to http://192.168.4.1
4. Upload your sound files (.wav format recommended)

### Features
- **Status Panel**: Shows current playing status and configuration
- **Quick Play**: Instant play/stop controls with visual feedback
- **Volume Control**: Real-time volume adjustment slider
- **File Management**: Upload, play, and delete sound files
- **Configuration**: Toggle random play and NFC features

### Supported Audio Formats
- **WAV files**: Best compatibility and quality
- **MP3 files**: Supported with some limitations
- **Recommended**: 16-bit WAV, 44.1kHz, mono for best performance

## 🎵 Sound File Recommendations

### Authentic Floppy Disk Sounds
1. **Seek sounds**: Head movement across tracks
2. **Read/Write sounds**: Data access operations  
3. **Motor sounds**: Drive spindle motor
4. **Click sounds**: Head engagement/disengagement
5. **Error sounds**: Read/write errors

### Where to Find Sounds
- Record from real floppy drives
- Retro computing sound libraries
- Generate using audio synthesis tools
- Online retro sound archives

## 🎛️ Operation Modes

### Trigger Methods
1. **Web Interface**: Click play buttons on web page
2. **Physical Button**: Press the trigger button
3. **NFC Cards**: Present any NFC card to reader
4. **API Calls**: RESTful API for integration

### Playback Modes
- **Random**: Plays random file from collection
- **Sequential**: Plays files in order
- **Single**: Plays specific selected file

## 🔧 Configuration

### WiFi Setup
- Device creates AP "FloppyDisk_AP" on startup
- Can also connect to existing WiFi networks
- Configuration saved in flash memory

### Audio Settings
- Volume: 0-100% adjustable
- Sample rate: Auto-detected from files
- Bit depth: Supports 8/16-bit audio

### NFC Configuration
- Enable/disable NFC triggering
- Any NFC card triggers playback
- Future: Card-specific sound mapping

## 🚀 Advanced Features

### API Endpoints
```
GET  /api/status      - Get playback status
GET  /api/config      - Get configuration
POST /api/config      - Update configuration
GET  /api/sounds      - List sound files
POST /api/play        - Play specific file
POST /api/play-random - Play random file
POST /api/stop        - Stop playback
POST /api/upload      - Upload sound file
```

### OTA Updates
- Over-the-air firmware updates supported
- Access via web interface (future feature)
- Maintains configuration during updates

## 🐛 Troubleshooting

### Audio Issues
- **No sound**: Check MAX98357 connections and power
- **Distorted audio**: Reduce volume or check file format
- **Choppy playback**: Use lower sample rate files

### WiFi Issues  
- **Can't connect**: Reset device and try again
- **Slow web interface**: Move closer to device
- **AP not visible**: Check for interference on 2.4GHz

### NFC Issues
- **Cards not detected**: Check SPI connections
- **Intermittent detection**: Ensure stable power supply
- **Wrong card type**: Use ISO14443A compatible cards

### Display Issues
- **Blank display**: Check I2C connections (SDA/SCL)
- **Corrupted display**: Verify 3.3V power supply
- **No animation**: Check if sounds are playing

## 📋 Technical Specifications

### Performance
- **CPU**: ESP32-C3 RISC-V @ 160MHz
- **Memory**: 4MB Flash, 400KB SRAM
- **Audio**: 16-bit I2S output @ up to 44.1kHz
- **Display**: 128x64 OLED, I2C interface
- **Connectivity**: WiFi 802.11 b/g/n, Bluetooth 5.0

### Power Requirements
- **Input**: 5V USB-C or 3.3-5V external
- **Consumption**: ~200mA typical, ~500mA peak (audio)
- **Standby**: ~50mA with WiFi connected

### Storage
- **File System**: LittleFS on internal flash
- **Capacity**: ~2.5MB available for sound files
- **File Limit**: Depends on file sizes

## 🤝 Contributing

Contributions welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

### Development Setup
1. Fork the repository
2. Create feature branch
3. Make changes and test
4. Submit pull request

### Coding Standards
- Use Arduino/ESP32 conventions
- Comment complex functions
- Test on hardware before PR
- Update documentation as needed

## 📄 License

This project is licensed under the MIT License - see [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- ESP32 Arduino Core developers
- PlatformIO team
- Adafruit for excellent libraries
- Retro computing community for inspiration

## 📞 Support

- **Issues**: Report on GitHub Issues
- **Discussions**: GitHub Discussions
- **Email**: [Your email here]
- **Discord**: [Your Discord server]

---

**Made with ❤️ for retro computing enthusiasts**