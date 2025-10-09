# Audio Test Example

This example demonstrates how to test the MAX98357 I2S audio amplifier independently of the main project code.

## Purpose
- Verify I2S audio connections
- Test MAX98357 amplifier functionality
- Generate test tones and sounds
- Debug audio playback issues

## Hardware Requirements
- ESP32-C3 development board
- MAX98357 I2S Class D Audio Amplifier
- Speaker (4-8 ohms, 3-5 watts)
- Jumper wires for connections

## Wiring
```
ESP32-C3    MAX98357
GPIO 4  ->  BCLK (Bit Clock)
GPIO 5  ->  LRC  (Left/Right Clock)
GPIO 6  ->  DIN  (Data Input)
3.3V    ->  VDD
GND     ->  GND

MAX98357    Speaker
OUT+    ->  Speaker + (Red)
OUT-    ->  Speaker - (Black)
```

## Code

```cpp
#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>

// I2S Configuration
#define I2S_NUM         I2S_NUM_0
#define I2S_BCLK_PIN    4
#define I2S_LRC_PIN     5
#define I2S_DOUT_PIN    6
#define SAMPLE_RATE     44100
#define BITS_PER_SAMPLE 16
#define CHANNELS        1

// Audio generation parameters
#define BUFFER_SIZE     1024
#define PI              3.14159265359

// Global variables
int16_t audioBuffer[BUFFER_SIZE];
size_t bytesWritten;
float phase = 0.0;

void setup() {
    Serial.begin(115200);
    Serial.println("I2S Audio Test Starting...");
    
    // Configure I2S
    i2s_config_t i2sConfig = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = BUFFER_SIZE,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };
    
    // I2S pin configuration
    i2s_pin_config_t pinConfig = {
        .bck_io_num = I2S_BCLK_PIN,
        .ws_io_num = I2S_LRC_PIN,
        .data_out_num = I2S_DOUT_PIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    
    // Install and start I2S driver
    esp_err_t result = i2s_driver_install(I2S_NUM, &i2sConfig, 0, NULL);
    if (result != ESP_OK) {
        Serial.printf("Failed to install I2S driver: %d\n", result);
        while(1) delay(1000);
    }
    
    result = i2s_set_pin(I2S_NUM, &pinConfig);
    if (result != ESP_OK) {
        Serial.printf("Failed to set I2S pins: %d\n", result);
        while(1) delay(1000);
    }
    
    Serial.println("I2S initialized successfully!");
    Serial.println("Starting audio tests...");
    
    // Run test sequence
    runAudioTests();
}

void loop() {
    // Main loop - tests run once in setup
    delay(1000);
}

void runAudioTests() {
    Serial.println("\n=== Audio Test Sequence ===");
    
    // Test 1: Silence (verify no noise)
    Serial.println("Test 1: Playing silence (2 seconds)");
    playSilence(2000);
    delay(1000);
    
    // Test 2: Sine wave tones
    Serial.println("Test 2: Playing sine wave tones");
    playSineWave(440, 1000);   // A4 note, 1 second
    delay(500);
    playSineWave(880, 1000);   // A5 note, 1 second
    delay(500);
    playSineWave(220, 1000);   // A3 note, 1 second
    delay(1000);
    
    // Test 3: Frequency sweep
    Serial.println("Test 3: Frequency sweep (100Hz to 2kHz)");
    playFrequencySweep(100, 2000, 3000);
    delay(1000);
    
    // Test 4: White noise
    Serial.println("Test 4: White noise (2 seconds)");
    playWhiteNoise(2000);
    delay(1000);
    
    // Test 5: Click sounds (like floppy disk)
    Serial.println("Test 5: Simulated floppy disk clicks");
    playFloppyClickSequence();
    delay(1000);
    
    Serial.println("Audio tests completed!");
}

void playSilence(int durationMs) {
    int totalSamples = (SAMPLE_RATE * durationMs) / 1000;
    int bufferCount = (totalSamples + BUFFER_SIZE - 1) / BUFFER_SIZE;
    
    // Fill buffer with zeros
    memset(audioBuffer, 0, sizeof(audioBuffer));
    
    for (int i = 0; i < bufferCount; i++) {
        int samplesToWrite = min(BUFFER_SIZE, totalSamples - (i * BUFFER_SIZE));
        i2s_write(I2S_NUM, audioBuffer, samplesToWrite * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
    }
}

void playSineWave(float frequency, int durationMs) {
    int totalSamples = (SAMPLE_RATE * durationMs) / 1000;
    int bufferCount = (totalSamples + BUFFER_SIZE - 1) / BUFFER_SIZE;
    
    float phaseIncrement = 2.0 * PI * frequency / SAMPLE_RATE;
    
    for (int bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++) {
        int samplesToGenerate = min(BUFFER_SIZE, totalSamples - (bufferIndex * BUFFER_SIZE));
        
        // Generate sine wave samples
        for (int i = 0; i < samplesToGenerate; i++) {
            float amplitude = sin(phase) * 0.3;  // 30% volume
            audioBuffer[i] = (int16_t)(amplitude * 32767);
            phase += phaseIncrement;
            
            // Keep phase in range
            if (phase >= 2.0 * PI) {
                phase -= 2.0 * PI;
            }
        }
        
        // Fill remaining buffer with zeros if needed
        for (int i = samplesToGenerate; i < BUFFER_SIZE; i++) {
            audioBuffer[i] = 0;
        }
        
        i2s_write(I2S_NUM, audioBuffer, BUFFER_SIZE * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
    }
}

void playFrequencySweep(float startFreq, float endFreq, int durationMs) {
    int totalSamples = (SAMPLE_RATE * durationMs) / 1000;
    int bufferCount = (totalSamples + BUFFER_SIZE - 1) / BUFFER_SIZE;
    
    float currentFreq = startFreq;
    float freqIncrement = (endFreq - startFreq) / totalSamples;
    
    for (int bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++) {
        int samplesToGenerate = min(BUFFER_SIZE, totalSamples - (bufferIndex * BUFFER_SIZE));
        
        for (int i = 0; i < samplesToGenerate; i++) {
            float phaseIncrement = 2.0 * PI * currentFreq / SAMPLE_RATE;
            float amplitude = sin(phase) * 0.3;
            audioBuffer[i] = (int16_t)(amplitude * 32767);
            
            phase += phaseIncrement;
            if (phase >= 2.0 * PI) {
                phase -= 2.0 * PI;
            }
            
            currentFreq += freqIncrement;
        }
        
        for (int i = samplesToGenerate; i < BUFFER_SIZE; i++) {
            audioBuffer[i] = 0;
        }
        
        i2s_write(I2S_NUM, audioBuffer, BUFFER_SIZE * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
    }
}

void playWhiteNoise(int durationMs) {
    int totalSamples = (SAMPLE_RATE * durationMs) / 1000;
    int bufferCount = (totalSamples + BUFFER_SIZE - 1) / BUFFER_SIZE;
    
    for (int bufferIndex = 0; bufferIndex < bufferCount; bufferIndex++) {
        int samplesToGenerate = min(BUFFER_SIZE, totalSamples - (bufferIndex * BUFFER_SIZE));
        
        // Generate white noise samples
        for (int i = 0; i < samplesToGenerate; i++) {
            float noise = ((float)random(-32768, 32767) / 32768.0) * 0.1;  // 10% volume
            audioBuffer[i] = (int16_t)(noise * 32767);
        }
        
        for (int i = samplesToGenerate; i < BUFFER_SIZE; i++) {
            audioBuffer[i] = 0;
        }
        
        i2s_write(I2S_NUM, audioBuffer, BUFFER_SIZE * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
    }
}

void playFloppyClickSequence() {
    Serial.println("  Simulating floppy disk seek pattern...");
    
    // Simulate head seeking with rapid clicks
    for (int click = 0; click < 10; click++) {
        // Generate click sound
        playClick(1000 + (click * 200), 50);  // Varying pitch, 50ms each
        delay(80);  // Gap between clicks
    }
    
    delay(500);
    
    // Simulate motor spin-up
    Serial.println("  Simulating motor spin-up...");
    playFrequencySweep(50, 150, 2000);
    
    delay(300);
    
    // Simulate read/write activity
    Serial.println("  Simulating read/write activity...");
    for (int i = 0; i < 15; i++) {
        playClick(800, 30);
        delay(random(50, 200));
    }
}

void playClick(float frequency, int durationMs) {
    int totalSamples = (SAMPLE_RATE * durationMs) / 1000;
    float phaseIncrement = 2.0 * PI * frequency / SAMPLE_RATE;
    float localPhase = 0;
    
    // Generate click with exponential decay
    for (int i = 0; i < totalSamples && i < BUFFER_SIZE; i++) {
        float envelope = exp(-5.0 * i / totalSamples);  // Exponential decay
        float amplitude = sin(localPhase) * envelope * 0.4;
        audioBuffer[i] = (int16_t)(amplitude * 32767);
        
        localPhase += phaseIncrement;
        if (localPhase >= 2.0 * PI) {
            localPhase -= 2.0 * PI;
        }
    }
    
    // Fill remaining buffer with zeros
    for (int i = totalSamples; i < BUFFER_SIZE; i++) {
        audioBuffer[i] = 0;
    }
    
    i2s_write(I2S_NUM, audioBuffer, min(totalSamples, BUFFER_SIZE) * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
}
```

## Usage Instructions

1. **Wire the hardware** according to the diagram above
2. **Upload the code** to your ESP32-C3
3. **Open serial monitor** at 115200 baud
4. **Listen to the audio output** through the speaker
5. **Check serial output** for test progress

## Expected Behavior

The test sequence will play:
1. **2 seconds of silence** - Verify no background noise
2. **Three sine wave tones** - A3, A4, A5 musical notes
3. **Frequency sweep** - Rising tone from 100Hz to 2kHz
4. **White noise** - Random noise for 2 seconds
5. **Floppy disk simulation** - Clicks and motor sounds

## Troubleshooting

### No Audio Output

**Check these items:**
- Verify all I2S pin connections
- Ensure MAX98357 has proper power (3.3V)
- Check speaker connections and polarity
- Verify speaker impedance (4-8 ohms recommended)

### Distorted Audio

**Possible causes:**
- Volume too high (reduce amplitude in code)
- Speaker impedance mismatch
- Power supply issues
- Poor connections

### Choppy/Interrupted Audio

**Solutions:**
- Increase DMA buffer size
- Check for timing issues in other code
- Verify stable power supply
- Monitor serial output for I2S errors

### I2S Driver Installation Failure

```
Failed to install I2S driver: [error code]
```

**Solutions:**
- Check if I2S is already initialized elsewhere
- Verify ESP32-C3 has sufficient resources
- Try restarting the device
- Check PlatformIO configuration

## Advanced Testing

### Volume Control Testing
```cpp
// Test different volume levels
for (float volume = 0.1; volume <= 1.0; volume += 0.1) {
    Serial.printf("Testing volume: %.1f\n", volume);
    playSineWaveWithVolume(440, 1000, volume);
    delay(500);
}
```

### Audio Quality Analysis
```cpp
// Measure audio timing
unsigned long startTime = micros();
i2s_write(I2S_NUM, audioBuffer, BUFFER_SIZE * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
unsigned long writeTime = micros() - startTime;
Serial.printf("I2S write time: %lu us\n", writeTime);
```

### Power Consumption Testing
- Measure current draw during silence vs. active playback
- Test different volume levels
- Monitor power supply stability

## Integration Notes

When integrating with the main project:

1. **Resource Management**: Ensure I2S driver is properly shared
2. **Buffer Management**: Coordinate with file playback systems
3. **Error Handling**: Implement proper I2S error recovery
4. **Performance**: Monitor for audio dropouts during web/NFC activity

## Common Issues and Solutions

| Issue | Cause | Solution |
|-------|-------|----------|
| No sound | Wrong pins | Verify I2S pin assignments |
| Distortion | High volume | Reduce amplitude multiplier |
| Clicks/pops | Buffer underrun | Increase buffer size |
| One channel only | Mono configuration | Check channel format setting |
| Timing issues | Interrupt conflicts | Use proper I2S timing |

---

*This audio test provides a comprehensive way to verify your I2S audio setup before integrating with the full floppy disk simulator.*