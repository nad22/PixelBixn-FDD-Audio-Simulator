# PixelBixn FDD Audio Simulator - Verkabelung

## ESP32-C3 Super Mini (diymore) Pinout

### Bereits integriert (keine Verkabelung nötig):
- **0.42" OLED Display (72x40 Pixel)**
  - SCL: GPIO 6 (intern verdrahtet)
  - SDA: GPIO 5 (intern verdrahtet)
  - Stromversorgung: intern
  - ✅ **FUNKTIONIERT BEREITS**

### Externe Komponenten (optional):

## 1. MAX98357 I2S Audio Verstärker
```
ESP32-C3 Pin  →  MAX98357 Pin
GPIO 8        →  BCLK (Bit Clock)
GPIO 9        →  LRC (Left/Right Clock)  
GPIO 7        →  DIN (Data Input)
GPIO 1        →  SD (Shutdown) - Verstärker ein/aus
GPIO 0        →  GAIN (Verstärkung) - LOW=9dB, HIGH=15dB
3.3V          →  VDD
GND           →  GND
```

**SD Pin Funktion:**
- HIGH (3.3V): Verstärker AN
- LOW (GND): Verstärker AUS (Standby, sehr geringer Stromverbrauch)
- Offen lassen: Verstärker immer AN

## 2. MFRC522 NFC/RFID Reader
```
ESP32-C3 Pin  →  MFRC522 Pin
GPIO 10       →  CS/SDA
GPIO 2        →  RST
3.3V          →  VCC (3.3V)
GND           →  GND
GPIO 6        →  SCK (ACHTUNG: Konflikt mit Display!)
GPIO 5        →  MOSI (ACHTUNG: Konflikt mit Display!)
GPIO 4        →  MISO
```

**⚠️ WICHTIG: NFC Reader hat I2C-Konflikt mit Display!**
Alternative Pins für NFC (SPI):
- SCK: GPIO 1
- MOSI: GPIO 0  
- MISO: GPIO 4

## 3. Trigger Taster
```
ESP32-C3 Pin  →  Taster
GPIO 3        →  Ein Anschluss
GND           →  Anderer Anschluss
```

## 4. Status LED
```
ESP32-C3 Pin  →  LED + Widerstand
GPIO 4        →  LED Anode (längeres Bein)
GND           →  LED Kathode über 220Ω Widerstand
```

## 5. Stromversorgung
```
USB-C Anschluss am Board
oder
5V/3.3V Pin → Externe Stromversorgung
GND Pin     → Masse
```

## Aktueller Status:
- ✅ **Display**: Funktioniert perfekt (intern verdrahtet)
- ✅ **WiFi**: Funktioniert, AccessPoint "FloppyDisk_AP" 
- ✅ **Web-Interface**: Erreichbar unter http://192.168.4.1
- ⏳ **Audio**: Code vorbereitet, Hardware optional
- ⏳ **NFC**: Code vorbereitet, Hardware optional  
- ⏳ **Taster**: Code vorbereitet, Hardware optional

## GPIO Übersicht ESP32-C3 Super Mini:
```
Verfügbare GPIOs: 0, 1, 2, 3, 4, 7, 8, 9, 10
Reserviert:       5 (Display SDA), 6 (Display SCL)
Audio Pins:       7 (DIN), 8 (BCLK), 9 (LRC), 1 (SD), 0 (GAIN)
USB:              18, 19 (nicht verfügbar für User)
```

## Nächste Schritte:
1. **Nur Display + WiFi**: Läuft bereits! 🎉
2. **+ Audio**: MAX98357 an GPIO 7,8,9 anschließen
3. **+ NFC**: MFRC522 mit alternativen SPI-Pins
4. **+ Taster**: Einfacher Taster an GPIO 3

## Stromverbrauch:
- ESP32-C3 + Display: ~80mA 
- + Audio Verstärker: +200mA (je nach Lautstärke)
- + NFC Reader: +20mA
- **Gesamt**: ~300mA bei voller Ausstattung