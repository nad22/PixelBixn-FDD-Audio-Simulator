# Example Audio Files for Floppy Disk Audio Simulator

This directory contains example sound files that demonstrate typical floppy disk drive sounds. These files are provided for testing and demonstration purposes.

## Included Sound Files

### 1. floppy_seek.wav
- **Description**: Head seeking across tracks
- **Duration**: ~2.5 seconds
- **Use Case**: Typical when accessing different parts of a disk
- **Characteristics**: Rapid clicking sounds with mechanical movement

### 2. floppy_motor.wav  
- **Description**: Drive motor spinning up
- **Duration**: ~4.0 seconds
- **Use Case**: When disk is inserted or system starts
- **Characteristics**: Gradual motor acceleration, steady hum

### 3. floppy_read.wav
- **Description**: Data reading operations
- **Duration**: ~1.8 seconds
- **Use Case**: Loading files or programs
- **Characteristics**: Soft clicking with periodic pauses

### 4. floppy_write.wav
- **Description**: Data writing operations
- **Duration**: ~3.2 seconds
- **Use Case**: Saving files to disk
- **Characteristics**: More intensive clicking than reading

### 5. floppy_error.wav
- **Description**: Read/write error sound
- **Duration**: ~1.5 seconds
- **Use Case**: When disk has bad sectors or is corrupted
- **Characteristics**: Repetitive clicking with mechanical grinding

## Technical Specifications

All example files follow these specifications for optimal compatibility:

- **Format**: WAV (Waveform Audio File Format)
- **Sample Rate**: 44.1 kHz
- **Bit Depth**: 16-bit
- **Channels**: Mono
- **Compression**: Uncompressed PCM
- **File Size**: Typically 50-400 KB each

## Usage Instructions

### Upload via Web Interface
1. Connect to the device's web interface
2. Navigate to the "Sound Files" section
3. Click "Upload" and select the desired WAV files
4. Files will be automatically validated and stored

### Manual Installation
1. Copy WAV files to the `data/sounds/` directory
2. Use PlatformIO's filesystem upload feature:
   ```bash
   pio run --target uploadfs
   ```

### Testing Sounds
- Use the web interface's "Play" buttons to test each sound
- Trigger via NFC card or physical button
- Monitor via serial output for debugging

## Creating Your Own Sound Files

### Recording from Real Hardware
For the most authentic experience, record sounds from actual floppy drives:

1. **Equipment Needed**:
   - Working floppy disk drive (3.5" or 5.25")
   - Computer with floppy drive interface
   - Audio recording software (Audacity recommended)
   - Microphone or line input

2. **Recording Tips**:
   - Position microphone close to drive mechanism
   - Use quiet environment to minimize background noise
   - Record multiple operations for variety
   - Capture different disk activities (read, write, seek, format)

3. **Post-Processing**:
   - Trim to desired length (1-5 seconds recommended)
   - Normalize audio levels
   - Remove background noise if necessary
   - Convert to mono if recorded in stereo
   - Export as 16-bit WAV at 44.1 kHz

### Sound Synthesis
Create synthetic floppy sounds using audio software:

1. **Seek Sounds**:
   - Use white noise bursts
   - Apply rapid amplitude modulation
   - Add mechanical clicking with filtered noise

2. **Motor Sounds**:
   - Generate low-frequency sine waves (50-200 Hz)
   - Add slight frequency modulation for realism
   - Include spin-up/spin-down characteristics

3. **Read/Write Sounds**:
   - Combine periodic clicking with background hum
   - Vary timing to simulate data access patterns
   - Use filtered noise for mechanical characteristics

## File Organization

Organize your sound files logically:

```
sounds/
├── startup/
│   ├── motor_spinup.wav
│   └── system_ready.wav
├── operations/
│   ├── seek_short.wav
│   ├── seek_long.wav
│   ├── read_single.wav
│   ├── read_multiple.wav
│   ├── write_single.wav
│   └── write_multiple.wav
├── errors/
│   ├── read_error.wav
│   ├── write_error.wav
│   └── bad_sector.wav
└── shutdown/
    ├── motor_spindown.wav
    └── system_halt.wav
```

## Quality Guidelines

### Audio Quality
- **Dynamic Range**: Maintain good signal-to-noise ratio
- **Frequency Response**: Focus on 100 Hz - 8 kHz range
- **Clipping**: Avoid digital clipping and distortion
- **Consistency**: Match volume levels across files

### File Management
- **Naming**: Use descriptive, consistent file names
- **Metadata**: Include duration and description information
- **Validation**: Test all files before distribution
- **Documentation**: Document the source and characteristics

## Troubleshooting

### Common Issues

**File won't play:**
- Check file format (must be WAV or MP3)
- Verify file isn't corrupted
- Ensure file size is under 2MB
- Check for proper sample rate (44.1 kHz recommended)

**Poor audio quality:**
- Increase bit depth to 16-bit minimum
- Check for clipping in source material
- Verify MAX98357 amplifier connections
- Adjust volume levels in software

**Upload failures:**
- Check available storage space
- Verify file permissions
- Ensure stable WiFi connection
- Try smaller file sizes

**Playback interruptions:**
- Reduce file size or sample rate
- Check power supply stability
- Monitor serial output for error messages
- Verify adequate free memory

## Legal Considerations

### Copyright
- Only use sounds you have rights to
- Original recordings are generally copyright-free
- Be cautious with sounds from commercial software
- Respect licensing terms of any downloaded sounds

### Attribution
When sharing or distributing:
- Credit original sound sources
- Include license information
- Respect Creative Commons requirements
- Acknowledge hardware sources if applicable

## Contributing Sound Files

We welcome contributions of high-quality floppy disk sounds:

1. **Submission Process**:
   - Fork the repository
   - Add sounds to `examples/sounds/` directory
   - Include documentation for each file
   - Submit a pull request with descriptions

2. **Requirements**:
   - Original recordings or properly licensed
   - High audio quality (16-bit WAV minimum)
   - Descriptive file names
   - Documentation of source and characteristics

3. **Review Process**:
   - Technical quality check
   - License verification
   - Testing on actual hardware
   - Community feedback

## Resources

### Software Tools
- **Audacity**: Free audio editor (https://www.audacityteam.org/)
- **Reaper**: Professional DAW with excellent audio processing
- **SoX**: Command-line audio processing toolkit
- **FFmpeg**: Audio/video conversion and processing

### Hardware Resources
- **Vintage Computing**: Sources for working floppy drives
- **Electronics Suppliers**: Audio recording equipment
- **Maker Communities**: Hardware building assistance
- **Retro Forums**: Sound sharing and collaboration

### Reference Materials
- **Floppy Drive Service Manuals**: Technical operation details
- **Audio Engineering Resources**: Recording and processing techniques
- **Retro Computing Documentation**: Historical context and accuracy

---

*These example sounds help bring authentic floppy disk nostalgia to your projects. Happy building!*