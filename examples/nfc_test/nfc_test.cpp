# Basic NFC Card Test Example

This example demonstrates how to test NFC card detection functionality independently of the main audio simulator code.

## Purpose
- Test NFC reader hardware connections
- Verify card detection and UID reading
- Debug communication issues
- Validate different card types

## Hardware Requirements
- ESP32-C3 development board
- PN532 V2.0 NFC module
- NFC cards (ISO14443A compatible)
- Jumper wires for connections

## Wiring
```
ESP32-C3    PN532
GPIO 10 ->  CS
GPIO 2  ->  RST  
GPIO 7  ->  MOSI
GPIO 5  ->  MISO
GPIO 6  ->  SCK
3.3V    ->  VCC
GND     ->  GND
```

## Code

```cpp
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

// Pin definitions
#define NFC_CS_PIN   10
#define NFC_RST_PIN  2

// Create MFRC522 instance
MFRC522 nfc(NFC_CS_PIN, NFC_RST_PIN);

void setup() {
    Serial.begin(115200);
    Serial.println("NFC Card Test Starting...");
    
    // Initialize SPI
    SPI.begin();
    
    // Initialize NFC reader
    nfc.PCD_Init();
    
    // Check if NFC reader is connected
    byte version = nfc.PCD_ReadRegister(nfc.VersionReg);
    if (version == 0x00 || version == 0xFF) {
        Serial.println("ERROR: NFC reader not found!");
        Serial.println("Check connections and power supply");
        while(1) delay(1000);
    }
    
    Serial.print("NFC reader found, version: 0x");
    Serial.println(version, HEX);
    Serial.println("Place an NFC card near the reader...");
}

void loop() {
    // Check for new cards
    if (!nfc.PICC_IsNewCardPresent()) {
        delay(100);
        return;
    }
    
    // Select one of the cards
    if (!nfc.PICC_ReadCardSerial()) {
        Serial.println("Failed to read card serial");
        delay(100);
        return;
    }
    
    // Card detected - print information
    Serial.println("\n=== NFC Card Detected ===");
    
    // Print UID
    Serial.print("UID: ");
    for (byte i = 0; i < nfc.uid.size; i++) {
        if (nfc.uid.uidByte[i] < 0x10) Serial.print("0");
        Serial.print(nfc.uid.uidByte[i], HEX);
        if (i < nfc.uid.size - 1) Serial.print(":");
    }
    Serial.println();
    
    // Print card type
    MFRC522::PICC_Type piccType = nfc.PICC_GetType(nfc.uid.sak);
    Serial.print("Card type: ");
    Serial.println(nfc.PICC_GetTypeName(piccType));
    
    // Print SAK (Select Acknowledge)
    Serial.print("SAK: 0x");
    Serial.println(nfc.uid.sak, HEX);
    
    // Print ATQA (Answer to Request Type A)
    Serial.print("ATQA: 0x");
    Serial.print(nfc.uid.atqa[0], HEX);
    Serial.println(nfc.uid.atqa[1], HEX);
    
    Serial.println("========================\n");
    
    // Halt PICC
    nfc.PICC_HaltA();
    
    // Stop encryption on PCD
    nfc.PCD_StopCrypto1();
    
    // Wait before next read
    delay(2000);
}
```

## Usage Instructions

1. **Upload the code** to your ESP32-C3
2. **Open serial monitor** at 115200 baud
3. **Place an NFC card** near the reader
4. **Observe the output** for card information

## Expected Output

```
NFC Card Test Starting...
NFC reader found, version: 0x92
Place an NFC card near the reader...

=== NFC Card Detected ===
UID: A1:B2:C3:D4
Card type: MIFARE Classic 1K
SAK: 0x08
ATQA: 0x0004
========================

=== NFC Card Detected ===
UID: E5:F6:A7:B8:C9
Card type: MIFARE Ultralight
SAK: 0x00
ATQA: 0x0044
========================
```

## Troubleshooting

### No NFC Reader Found
```
ERROR: NFC reader not found!
Check connections and power supply
```

**Solutions:**
- Verify all pin connections
- Check 3.3V power supply
- Ensure proper SPI wiring
- Test with multimeter for continuity

### Card Not Detected
**Symptoms:** No output when placing cards near reader

**Solutions:**
- Try different card types (Mifare Classic, Ultralight, etc.)
- Reduce distance between card and reader
- Check for interference from other devices
- Verify card is ISO14443A compatible

### Intermittent Detection
**Symptoms:** Cards detected sometimes but not consistently

**Solutions:**
- Check power supply stability
- Add decoupling capacitors near NFC module
- Ensure solid connections (no loose wires)
- Try different card orientations

### Read Errors
```
Failed to read card serial
```

**Solutions:**
- Card may be moving too fast
- Try holding card steady for longer
- Check for damaged or corrupted cards
- Reduce read frequency in code

## Advanced Testing

### Multiple Card Types
Test with various card technologies:
- **Mifare Classic 1K/4K**: Most common NFC cards
- **Mifare Ultralight**: Smaller memory cards  
- **NTAG213/215/216**: NFC Forum Type 2 cards
- **DESFire**: High-security cards
- **Phone NFC**: Test with smartphone NFC

### Performance Testing
```cpp
// Add timing measurements
unsigned long startTime = millis();
bool cardPresent = nfc.PICC_IsNewCardPresent();
unsigned long detectionTime = millis() - startTime;

Serial.print("Detection time: ");
Serial.print(detectionTime);
Serial.println(" ms");
```

### Range Testing
- Test detection range at various distances
- Document maximum reliable range
- Note any dead zones or sweet spots

### Power Consumption
- Measure current draw during operation
- Test power-saving modes if available
- Monitor for power supply issues

## Integration Notes

When integrating this code into the main project:

1. **Pin Conflicts**: Ensure SPI pins don't conflict with I2S audio
2. **Timing**: Coordinate NFC polling with audio playback
3. **Error Handling**: Implement proper error recovery
4. **Configuration**: Make pin assignments configurable

## Common Card UIDs for Testing

Document your test cards for consistent testing:

```cpp
// Test card database
struct TestCard {
    String name;
    byte uid[10];
    byte uidSize;
    String type;
};

TestCard testCards[] = {
    {"Blue Mifare", {0xA1, 0xB2, 0xC3, 0xD4}, 4, "Classic 1K"},
    {"White NTAG", {0xE5, 0xF6, 0xA7, 0xB8}, 4, "NTAG213"},
    {"Phone NFC", {0x12, 0x34, 0x56, 0x78}, 4, "Variable"}
};
```

## Next Steps

Once basic NFC functionality is confirmed:
1. Integrate with main audio simulator code
2. Implement card-specific sound mapping
3. Add card registration/configuration features
4. Test with complete hardware setup

---

*This example provides a solid foundation for NFC testing and troubleshooting in your floppy disk audio simulator project.*