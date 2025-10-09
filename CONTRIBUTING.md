# Contributing to Floppy Disk Audio Simulator

We welcome contributions to the Floppy Disk Audio Simulator project! This document provides guidelines for contributing code, documentation, and other improvements.

## Table of Contents
- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [How to Contribute](#how-to-contribute)
- [Coding Standards](#coding-standards)
- [Testing Guidelines](#testing-guidelines)
- [Documentation](#documentation)
- [Submitting Changes](#submitting-changes)
- [Review Process](#review-process)
- [Recognition](#recognition)

## Code of Conduct

This project adheres to a code of conduct that we expect all contributors to honor. Please read the [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) file for details.

Key principles:
- Be respectful and inclusive
- Focus on constructive feedback
- Help create a welcoming environment for all
- Report any unacceptable behavior

## Getting Started

### Prerequisites
- **Hardware**: ESP32-C3 development board with OLED display
- **Software**: PlatformIO IDE (VS Code recommended)
- **Knowledge**: Basic understanding of C++, Arduino framework, and web technologies
- **Tools**: Git for version control

### First Steps
1. **Read the documentation**: Familiarize yourself with the README.md and hardware documentation
2. **Set up hardware**: Build the basic circuit following the hardware guide
3. **Test the system**: Ensure everything works with the provided code
4. **Explore the code**: Understand the project structure and architecture

## Development Setup

### Environment Setup
```bash
# Clone the repository
git clone https://github.com/yourusername/floppy-disk-audio-simulator.git
cd floppy-disk-audio-simulator

# Install PlatformIO CLI (optional, but recommended)
pip install platformio

# Open in VS Code with PlatformIO extension
code .
```

### Building and Testing
```bash
# Build the project
pio run

# Upload to device
pio run --target upload

# Monitor serial output
pio device monitor

# Run tests (when available)
pio test
```

### Project Structure
```
├── src/                    # Main source code
│   ├── main.cpp           # Main application file
│   ├── config.h           # Configuration constants
│   └── utils/             # Utility functions
├── lib/                    # Custom libraries
├── data/                   # Web interface files
├── docs/                   # Documentation
├── test/                   # Unit tests
├── examples/              # Example code
└── platformio.ini         # PlatformIO configuration
```

## How to Contribute

### Types of Contributions
We welcome various types of contributions:

1. **Bug Reports**: Help us identify and fix issues
2. **Feature Requests**: Suggest new functionality
3. **Code Contributions**: Submit bug fixes and new features
4. **Documentation**: Improve existing docs or add new ones
5. **Testing**: Help test on different hardware configurations
6. **Hardware Designs**: PCB layouts, enclosures, schematics
7. **Sound Files**: Contribute authentic floppy disk sounds

### Reporting Bugs
When reporting bugs, please include:

**Bug Report Template:**
```markdown
## Bug Description
Brief description of the issue

## Steps to Reproduce
1. Step one
2. Step two
3. ...

## Expected Behavior
What should happen

## Actual Behavior
What actually happens

## Environment
- Hardware: ESP32-C3 board model
- Software version: v1.0.0
- Operating system: Windows 10
- Browser: Chrome 95

## Additional Information
- Serial output logs
- Screenshots if applicable
- Any error messages
```

### Suggesting Features
Feature requests should include:

**Feature Request Template:**
```markdown
## Feature Summary
Brief description of the proposed feature

## Use Case
Why is this feature needed? What problem does it solve?

## Proposed Implementation
How should this feature work?

## Alternative Solutions
Other ways to achieve the same goal

## Additional Context
Mockups, references, or examples
```

## Coding Standards

### C++ Guidelines
```cpp
// Use descriptive variable names
int currentVolume = 50;
bool isPlaying = false;

// Function naming: camelCase
void playRandomSound() {
    // Implementation
}

// Constants: UPPER_CASE
const int MAX_VOLUME = 100;
const char* DEFAULT_SSID = "FloppyDisk_AP";

// Class naming: PascalCase
class AudioManager {
private:
    int volume_;  // Private members with trailing underscore
    
public:
    void setVolume(int volume);
    int getVolume() const;
};
```

### Code Style
- **Indentation**: 4 spaces (no tabs)
- **Line Length**: Maximum 100 characters
- **Braces**: Opening brace on same line
- **Comments**: Use `//` for single line, `/* */` for multi-line
- **Headers**: Include guards or `#pragma once`

### Documentation Comments
```cpp
/**
 * @brief Play a sound file with specified volume
 * @param filename Name of the sound file to play
 * @param volume Volume level (0-100)
 * @return true if playback started successfully, false otherwise
 */
bool playSound(const String& filename, int volume = -1);
```

### Web Interface Guidelines
```javascript
// Use modern JavaScript (ES6+)
const playSound = async (filename) => {
    try {
        const response = await fetch('/api/play', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ file: filename })
        });
        return await response.json();
    } catch (error) {
        console.error('Failed to play sound:', error);
        throw error;
    }
};

// CSS: Use Material Design principles
.audio-controls {
    display: flex;
    align-items: center;
    gap: 1rem;
    padding: 1rem;
    border-radius: 4px;
    box-shadow: 0 2px 4px rgba(0,0,0,0.1);
}
```

## Testing Guidelines

### Hardware Testing
- Test on actual ESP32-C3 hardware
- Verify all GPIO connections work correctly
- Test with different audio amplifiers and speakers
- Validate NFC functionality with various card types

### Software Testing
```cpp
// Example unit test structure
void testVolumeControl() {
    AudioManager audio;
    
    // Test valid volume range
    assert(audio.setVolume(50) == true);
    assert(audio.getVolume() == 50);
    
    // Test invalid volume
    assert(audio.setVolume(150) == false);
    assert(audio.getVolume() == 50); // Should remain unchanged
}
```

### Web Interface Testing
- Test in multiple browsers (Chrome, Firefox, Safari, Edge)
- Verify responsive design on mobile devices
- Test file upload with various file types and sizes
- Validate error handling for network issues

### Integration Testing
- Test WiFi connectivity in different environments
- Verify NFC reader with multiple card types
- Test audio playback with various file formats
- Validate web interface functionality across devices

## Documentation

### Code Documentation
- Document all public functions and classes
- Include examples for complex functionality
- Keep comments up-to-date with code changes
- Use clear, concise language

### User Documentation
- Update README.md for new features
- Add hardware documentation for new components
- Include troubleshooting steps for common issues
- Provide clear setup instructions

### API Documentation
- Document all API endpoints
- Include request/response examples
- Specify error codes and handling
- Provide SDK examples for popular languages

## Submitting Changes

### Pull Request Process
1. **Fork the repository** and create a feature branch
2. **Make your changes** following the coding standards
3. **Test thoroughly** on actual hardware
4. **Update documentation** as needed
5. **Submit a pull request** with a clear description

### Pull Request Template
```markdown
## Description
Brief description of changes made

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Performance improvement
- [ ] Other (please describe)

## Testing
- [ ] Tested on ESP32-C3 hardware
- [ ] Web interface tested in multiple browsers
- [ ] All existing functionality still works
- [ ] New functionality works as expected

## Checklist
- [ ] Code follows project style guidelines
- [ ] Self-review completed
- [ ] Documentation updated
- [ ] No new compiler warnings
- [ ] Changes are backward compatible

## Screenshots (if applicable)
Include screenshots for UI changes

## Related Issues
Fixes #(issue number)
```

### Commit Messages
Use clear, descriptive commit messages:

```bash
# Good examples
git commit -m "Add volume control to web interface"
git commit -m "Fix NFC card detection timeout issue"
git commit -m "Update hardware documentation for v2.0 PCB"

# Bad examples
git commit -m "Fix bug"
git commit -m "Update stuff"
git commit -m "WIP"
```

### Branch Naming
Use descriptive branch names:
- `feature/web-volume-control`
- `bugfix/nfc-timeout-issue`
- `docs/hardware-v2-update`
- `refactor/audio-manager-cleanup`

## Review Process

### Review Criteria
Pull requests are reviewed for:
- **Functionality**: Does it work as intended?
- **Code Quality**: Follows coding standards and best practices
- **Testing**: Adequate testing on real hardware
- **Documentation**: Appropriate documentation updates
- **Compatibility**: Doesn't break existing functionality

### Review Timeline
- **Initial Response**: Within 48 hours
- **Full Review**: Within 1 week
- **Follow-up**: Ongoing until approved or closed

### Reviewer Guidelines
Reviewers should:
- Be constructive and respectful
- Focus on code quality and functionality
- Suggest improvements rather than just pointing out problems
- Test changes when possible
- Approve when ready or request specific changes

## Recognition

### Contributors
All contributors are recognized in:
- **README.md**: Contributors section
- **Release Notes**: Acknowledgment of contributions
- **GitHub**: Contributor statistics and graphs

### Types of Recognition
- **Code Contributors**: Listed in main contributors
- **Documentation**: Acknowledged in docs section  
- **Hardware Designs**: Featured in hardware documentation
- **Testing**: Recognized in testing acknowledgments
- **Bug Reports**: Credited in issue resolution

### Maintainers
Current project maintainers:
- **@yourusername**: Project lead, hardware design
- **@contributor1**: Web interface, API development
- **@contributor2**: Audio processing, testing

## Getting Help

### Communication Channels
- **GitHub Issues**: For bugs and feature requests
- **GitHub Discussions**: For general questions and ideas
- **Discord Server**: [Link to Discord] (if available)
- **Email**: project-email@example.com

### Documentation Resources
- **README.md**: Project overview and setup
- **HARDWARE.md**: Detailed hardware documentation
- **API.md**: Complete API reference
- **Examples**: Sample code and integrations

### Community
- **Show and Tell**: Share your builds and modifications
- **Help Others**: Answer questions and provide support
- **Share Ideas**: Contribute to project roadmap discussions

## License

By contributing to this project, you agree that your contributions will be licensed under the same license as the project (MIT License).

---

Thank you for contributing to the Floppy Disk Audio Simulator project! Your contributions help make this project better for everyone in the retro computing community.