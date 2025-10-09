# Hardware Documentation

## Overview
This document provides detailed hardware specifications, assembly instructions, and troubleshooting guide for the Floppy Disk Audio Simulator project.

## Bill of Materials (BOM)

### Required Components

| Component | Part Number | Quantity | Description | Estimated Cost |
|-----------|-------------|----------|-------------|----------------|
| ESP32-C3 Development Board | diymore ESP32-C3 OLED | 1 | Main microcontroller with 0.42" OLED display | $13 |
| Audio Amplifier | MAX98357A | 1 | I2S Class D audio amplifier breakout | $8 |
| NFC Module | PN532 V2.0 | 1 | 13.56MHz NFC/RFID reader/writer | $12 |
| Speaker | 4Ω 3W | 1 | Small speaker for audio output | $5 |
| Push Button | 6x6mm tactile | 1 | Momentary push button for manual trigger | $0.50 |
| Resistors | 220Ω, 10kΩ | 2 | For LED and pull-up (if needed) | $0.20 |
| LED | 3mm/5mm | 1 | Status indicator LED | $0.10 |
| Breadboard/PCB | Half-size | 1 | For prototyping connections | $5 |
| Jumper Wires | M-M, M-F | 20 | Connection wires | $3 |
| USB-C Cable | Data capable | 1 | For programming and power | $5 |

**Total Estimated Cost: ~$52**

### Optional Components

| Component | Description | Cost |
|-----------|-------------|------|
| Enclosure | 3D printed or plastic box | $10 |
| NFC Cards | ISO14443A compatible | $5 |
| External Power Supply | 5V 2A USB power adapter | $8 |
| PCB | Custom PCB instead of breadboard | $15 |

## Detailed Component Specifications

### ESP32-C3 Development Board
- **MCU**: ESP32-C3 RISC-V single-core @ 160MHz
- **Memory**: 4MB Flash, 400KB SRAM
- **Display**: 0.42" OLED 72x40 pixels
- **Connectivity**: WiFi 802.11 b/g/n, Bluetooth 5.0 LE
- **GPIO**: 15 programmable pins
- **Power**: USB-C input, 3.3V operation
- **Size**: 25 x 21 x 10mm

### MAX98357A Audio Amplifier
- **Type**: I2S input, Class D output
- **Power**: Up to 3.2W @ 4Ω, 1.4W @ 8Ω
- **SNR**: 92dB
- **THD+N**: 0.013% @ 1kHz
- **Sample Rate**: 8kHz to 96kHz
- **Supply Voltage**: 2.5V to 5.5V
- **Gain**: Fixed 9dB gain

### PN532 NFC Module
- **Frequency**: 13.56MHz
- **Standards**: ISO14443A/B, FeliCa, Mifare
- **Range**: Up to 5cm
- **Interface**: SPI, I2C, UART (SPI used in this project)
- **Power**: 3.3V operation
- **Current**: 100mA max

## Pin Mapping and Connections

### ESP32-C3 Pinout
```
     ┌─────────────┐
     │    ESP32-C3 │
     │     OLED    │
     └─────────────┘
  EN │1          21│ GPIO21
 GPIO0│2          20│ GPIO20  
 GPIO1│3          19│ GPIO19
 GPIO2│4          18│ GPIO18
 GPIO3│5          17│ GPIO17 (not available)
 GPIO4│6          16│ GPIO16 (not available)
 GPIO5│7          15│ GPIO15 (not available)
 GPIO6│8          14│ GPIO14 (not available)
 GPIO7│9          13│ GPIO13 (not available)
GPIO10│10         12│ GPIO12 (not available)
  3.3V│11         11│ GND
```

### Connection Diagram

#### Power Connections
```
ESP32-C3 (3.3V) → MAX98357 (VDD)
ESP32-C3 (3.3V) → PN532 (VCC)
ESP32-C3 (GND)  → All GND connections
```

#### I2S Audio (ESP32-C3 → MAX98357)
```
GPIO 4 → BCLK (Bit Clock)
GPIO 5 → LRC  (Left/Right Clock)
GPIO 6 → DIN  (Data Input)
```

#### SPI NFC (ESP32-C3 → PN532)
```
GPIO 10 → CS   (Chip Select)
GPIO 2  → RST  (Reset)
GPIO 7  → MOSI (Master Out Slave In)
GPIO 5  → MISO (Master In Slave Out) [shared with LRC]
GPIO 6  → SCK  (Serial Clock) [shared with DIN]
```

#### I2C Display (Built-in OLED)
```
GPIO 8 → SDA (Serial Data)
GPIO 9 → SCL (Serial Clock)
```

#### Controls
```
GPIO 3 → Push Button → GND (with internal pull-up)
GPIO 7 → LED Anode → 220Ω Resistor → GND
```

## Assembly Instructions

### Step 1: Prepare Components
1. Unpack all components and verify against BOM
2. Check ESP32-C3 board functionality by connecting to USB and powering on
3. Verify OLED display shows startup screen

### Step 2: Breadboard Layout
1. Place ESP32-C3 board on breadboard
2. Position MAX98357 amplifier module nearby
3. Mount PN532 NFC module on opposite side
4. Leave space for speaker connections

### Step 3: Power Connections
1. Connect 3.3V rail to breadboard power rail
2. Connect GND rail to breadboard ground rail
3. Connect all component VCC/VDD pins to 3.3V rail
4. Connect all component GND pins to ground rail

### Step 4: Audio Connections (I2S)
```
ESP32-C3 Pin 6 (GPIO 4) → MAX98357 BCLK
ESP32-C3 Pin 7 (GPIO 5) → MAX98357 LRC  
ESP32-C3 Pin 8 (GPIO 6) → MAX98357 DIN
```

### Step 5: NFC Connections (SPI)
```
ESP32-C3 Pin 10 (GPIO 10) → PN532 CS
ESP32-C3 Pin 4  (GPIO 2)  → PN532 RST
ESP32-C3 Pin 9  (GPIO 7)  → PN532 MOSI
ESP32-C3 Pin 7  (GPIO 5)  → PN532 MISO
ESP32-C3 Pin 8  (GPIO 6)  → PN532 SCK
```

### Step 6: Control Connections
```
ESP32-C3 Pin 5 (GPIO 3) → One side of push button
Other side of push button → GND
ESP32-C3 Pin 9 (GPIO 7) → LED Anode (longer leg)
LED Cathode → 220Ω Resistor → GND
```

### Step 7: Speaker Connection
```
MAX98357 OUT+ → Speaker positive terminal
MAX98357 OUT- → Speaker negative terminal
```

### Step 8: Final Checks
1. Double-check all connections against pin diagrams
2. Verify no short circuits between power and ground
3. Ensure secure connections without loose wires
4. Check speaker polarity (+ to +, - to -)

## PCB Design Considerations

### Layout Guidelines
1. **Ground Plane**: Use solid ground plane for noise reduction
2. **Power Decoupling**: Place bypass capacitors close to IC power pins
3. **I2S Traces**: Keep I2S signals short and parallel
4. **Antenna Clearance**: Keep WiFi antenna area clear of copper
5. **Component Spacing**: Allow adequate clearance for connectors

### Recommended PCB Stackup
- **Layer 1**: Components and signal traces  
- **Layer 2**: Ground plane
- **Layer 3**: Power plane (3.3V)
- **Layer 4**: Additional signal routing

### Critical Design Rules
- **Trace Width**: Minimum 0.2mm for signals, 0.5mm for power
- **Via Size**: 0.2mm drill, 0.4mm pad
- **Clearance**: 0.15mm minimum between traces
- **Copper Pour**: Connect to ground plane where possible

## Mechanical Design

### Enclosure Requirements
- **Dimensions**: Minimum 80x60x25mm internal space
- **Materials**: ABS plastic or 3D printed PLA/PETG
- **Ventilation**: Slots for heat dissipation
- **Access**: Holes for USB-C, button, speaker, and NFC area
- **Mounting**: Standoffs for PCB mounting

### 3D Printing Considerations
- **Layer Height**: 0.2mm for good detail
- **Infill**: 20% for adequate strength
- **Support**: Required for overhangs
- **Post-Processing**: Sand and fit-test before final assembly

## Power Supply Design

### Power Requirements
| Component | Voltage | Current (Typical) | Current (Peak) |
|-----------|---------|-------------------|----------------|
| ESP32-C3 | 3.3V | 80mA | 250mA |
| OLED Display | 3.3V | 20mA | 30mA |
| MAX98357 | 3.3V | 10mA | 1.5A |
| PN532 | 3.3V | 50mA | 100mA |
| **Total** | **3.3V** | **160mA** | **1.88A** |

### Power Supply Options
1. **USB-C**: 5V input with onboard 3.3V regulator
2. **External**: 5V 2A wall adapter
3. **Battery**: 3.7V LiPo with charge controller
4. **Desktop**: Bench power supply for development

### Power Management
- **Voltage Monitoring**: Check for brown-out conditions
- **Current Limiting**: Protect against overcurrent
- **Thermal Protection**: Monitor temperature during operation
- **Efficiency**: Use switching regulators for battery operation

## Testing and Validation

### Basic Functionality Tests
1. **Power On**: Verify 3.3V on all supply pins
2. **Display**: Check OLED shows startup message
3. **WiFi**: Confirm AP mode activation
4. **Audio**: Test speaker output with test tone
5. **NFC**: Verify card detection
6. **Button**: Test manual trigger functionality

### Performance Tests
1. **Audio Quality**: THD+N measurement
2. **WiFi Range**: Signal strength at various distances
3. **NFC Range**: Card detection distance
4. **Power Consumption**: Current measurement in all modes
5. **Temperature**: Thermal imaging under load

### Compliance Testing
1. **EMC**: Electromagnetic compatibility
2. **RF**: WiFi certification requirements
3. **Safety**: Electrical safety standards
4. **Environmental**: Temperature and humidity limits

## Troubleshooting Guide

### Power Issues
| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| No power indication | Dead power supply | Check USB cable and power source |
| Intermittent operation | Loose connections | Verify all power connections |
| System resets | Insufficient current | Use higher capacity power supply |
| Overheating | Poor ventilation | Add cooling or reduce power |

### Audio Issues
| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| No audio output | Wrong pin connections | Verify I2S wiring |
| Distorted audio | Overdriven amplifier | Reduce volume in software |
| Noise/hum | Ground loops | Improve grounding scheme |
| Choppy playback | Insufficient processing | Reduce sample rate or bit depth |

### Communication Issues
| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| WiFi not working | Antenna issue | Check PCB antenna clearance |
| NFC not detecting | SPI timing | Verify SPI connections and speed |
| Web interface slow | Network congestion | Move closer to device |
| Upload failures | File system full | Delete old files or increase storage |

### Display Issues
| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| Blank display | I2C communication | Check SDA/SCL connections |
| Corrupted graphics | Power supply noise | Add power supply filtering |
| Dim display | Contrast setting | Adjust contrast in software |
| Frozen display | Software crash | Reset device and check code |

## Maintenance and Upgrades

### Regular Maintenance
- **Clean NFC reader**: Remove dust from antenna area
- **Check connections**: Verify no loose wires
- **Update firmware**: Install latest software versions
- **Backup config**: Save configuration settings

### Possible Upgrades
- **Better speaker**: Higher quality audio output
- **External antenna**: Improved WiFi range
- **Battery pack**: Portable operation
- **RGB LEDs**: Enhanced visual feedback
- **Touch screen**: Advanced user interface

## Safety Considerations

### Electrical Safety
- **Voltage levels**: All circuits operate at safe low voltage (≤5V)
- **Overcurrent protection**: Use appropriate fuses or current limiters
- **ESD protection**: Handle components with anti-static precautions
- **Isolation**: Maintain isolation between power and signal circuits

### Mechanical Safety
- **Enclosure**: Ensure no sharp edges or exposed conductors
- **Mounting**: Secure all components to prevent movement
- **Ventilation**: Provide adequate cooling to prevent overheating
- **Accessibility**: Emergency shutdown capability

### Environmental Considerations
- **Operating temperature**: 0°C to 60°C
- **Storage temperature**: -20°C to 80°C
- **Humidity**: 10% to 90% non-condensing
- **Altitude**: Up to 2000m above sea level

---

*This hardware documentation is part of the Floppy Disk Audio Simulator project. For software documentation, see README.md*