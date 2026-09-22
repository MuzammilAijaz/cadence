# Cadence

A minimalist desk focus timer and rhythm tool built on the STM8S103F3 microcontroller, designed to fit on a single half-size breadboard. The hardware interface uses a standard 16x2 character LCD, a joystick with buttons for navigation, and a speaker for audio output.

Firmware is written in C using the STM8 Standard Peripheral Library and structured around finite state machines (inspired by QP/C).

Targets the STM8S103F3 (1KB RAM / 8KB Flash).

## Features & Roadmap

### UI & Navigation
- [x] Menu navigation with joystick and buttons
- [x] 16x2 screen display
- [ ] Sound effects for menu navigation and button presses

### Timers & Stopwatch
- [x] Working stopwatch (start, stop)
- [ ] Countdown focus/pomodoro timers
- [ ] Multiple active timers

### Audio Synthesizer
- [x] Basic melody / tone playback
- [ ] Play musical notes using joystick and buttons (mini synth mode)

## Building

The build system and toolchain setup are adapted from the [STM8 Project Template](https://github.com/CTXz/stm8-project-template) by CTXz (see their repository for more detailed toolchain documentation and options).

### Docker (Recommended)

Build and flash without installing tools on the host:

```bash
./compose.sh build   # build firmware
./compose.sh flash   # flash via ST-LINK over USB
```
