# AVR-Core-Lib Collection

> 13 standalone, single-file C header libraries for bare-metal AVR programming — no Arduino, no abstraction layers, pure register-level control.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Build](https://img.shields.io/badge/build-passing-brightgreen)]()
[![MCU](https://img.shields.io/badge/MCU-ATmega328P-orange)]()
[![Standard](https://img.shields.io/badge/C-C99-blue)]()

---

## What Is This?

A production-ready collection of **13 single-file header libraries** covering the complete ATmega AVR architecture — from GPIO fundamentals to advanced power management and software timers. Every library compiles with **zero warnings** under `-Wall -Wextra -Werror` and is validated on ATmega328P.

Each library is fully self-contained: just `#include` it, define the implementation guard in **one** `.c` file, and you're done. No build system dependencies, no linking headaches.

---

## Libraries

| # | Library | Description | Flash¹ |
|---|---------|-------------|--------|
| 1 | [`avr_gpio.h`](avr_gpio.h) | Pin mode, digital R/W, toggle, port-level bulk ops, atomic R-M-W | 606 B |
| 2 | [`avr_timer.h`](avr_timer.h) | Timer0/1/2 Normal & CTC modes, ISR callbacks, 1 ms system tick, delay | 1.5 KB |
| 3 | [`avr_uart.h`](avr_uart.h) | Interrupt-driven ring-buffer TX/RX, auto baud + U2X, printf stream | 1.9 KB |
| 4 | [`avr_adc.h`](avr_adc.h) | Ref voltage select, auto prescaler, single-shot & free-running, mV conv | 2.5 KB |
| 5 | [`avr_extint.h`](avr_extint.h) | INT0/INT1, PCINT0-2, edge config, user callbacks, debounce helper | 1.4 KB |
| 6 | [`avr_pwm.h`](avr_pwm.h) | Fast & Phase-correct PWM, 8/16-bit, servo via ICR1, frequency calculator | 2.5 KB |
| 7 | [`avr_spi.h`](avr_spi.h) | Master/Slave, SPI modes 0-3, SPI2X speeds, buffer transfer, CS helpers | 2.1 KB |
| 8 | [`avr_i2c.h`](avr_i2c.h) | Master TWI, 100/400 kHz, register R/W, burst read/write, bus scan | 2.8 KB |
| 9 | [`avr_eeprom.h`](avr_eeprom.h) | Byte/word/block R/W, wear-leveling update, CRC-8 integrity check | 2.7 KB |
| 10 | [`avr_wdt.h`](avr_wdt.h) | Timed-sequence enable/disable, reset/interrupt/combined modes, MCUSR | 2.2 KB |
| 11 | [`avr_bitops.h`](avr_bitops.h) | Bit-field ops, rotate, popcount, CLZ/CTZ, CRC-8 PROGMEM LUT, byte swap | 2.7 KB |
| 12 | [`avr_power.h`](avr_power.h) | Sleep modes, PRR peripheral gating, BOD-disable deep sleep | 2.4 KB |
| 13 | [`avr_swtimer.h`](avr_swtimer.h) | Static timer pool, one-shot & periodic, ISR tick + main-loop dispatch | 3.8 KB |

<sup>¹ Flash usage includes the full example program + UART output. Library-only footprint is significantly smaller.</sup>

---

## Quick Start

### Prerequisites

```bash
# Debian/Ubuntu
sudo apt install gcc-avr avr-libc

# Arch
sudo pacman -S avr-gcc avr-libc

# macOS (Homebrew)
brew tap osx-cross/avr && brew install avr-gcc avr-libc
```

### Usage

Each library follows the **single-file header** pattern. In exactly **one** `.c` file, define the implementation macro before including:

```c
#define F_CPU 16000000UL        // Your clock frequency

#define AVR_GPIO_IMPLEMENTATION // ← Enable implementation in THIS file
#include "avr_gpio.h"

#define AVR_UART_IMPLEMENTATION
#include "avr_uart.h"

int main(void) {
    gpio_set_mode(&DDRB, &PORTB, PB5, GPIO_OUTPUT);
    uart_init(9600);
    sei();

    uart_send_string("Hello, AVR!\r\n");

    while (1) {
        gpio_toggle_pin(&PINB, PB5);
        uart_send_string("Blink!\r\n");
        for (volatile uint32_t i = 0; i < 200000; i++);
    }
}
```

In other files that only need the API declarations, include **without** the `#define`:

```c
#include "avr_gpio.h"   // declarations only — no duplicate symbols
```

### Build

```bash
# Compile a single file
avr-gcc -std=c99 -Os -Wall -Wextra -Werror -mmcu=atmega328p -DF_CPU=16000000UL \
    -o main.elf main.c

# Generate hex for flashing
avr-objcopy -O ihex -R .eeprom main.elf main.hex

# Flash (e.g. with avrdude for Arduino Uno)
avrdude -p m328p -c arduino -P /dev/ttyUSB0 -b 115200 -U flash:w:main.hex:i
```

### Build All Examples

```bash
make all     # Compile all 13 examples
make size    # Flash + SRAM usage report
make hex     # Generate .hex files for flashing
make clean   # Remove build artifacts
```

---

## Examples

Every library has a matching example in [`examples/`](examples/) that exercises **every API function** on ATmega328P @ 16 MHz:

| Example | What It Demonstrates |
|---------|---------------------|
| `gpio_example.c` | Pin modes, read/write, toggle, port-level, atomic, macros |
| `timer_example.c` | System tick, CTC callbacks, overflow ISR, `DelayMs`, `DelayUs` |
| `uart_example.c` | Init, send/receive, printf redirect, hex/decimal print, error detection |
| `adc_example.c` | Single-shot, 8-bit, averaging, millivolt, ref change, temp sensor, free-run |
| `extint_example.c` | INT0 falling edge, INT1 rising, PCINT, pin enable/disable, debounce |
| `pwm_example.c` | Fast/phase-correct, servo control, duty fade, frequency calculator |
| `spi_example.c` | Master init, JEDEC ID read, buffer transfer, data order, CS pattern |
| `i2c_example.c` | Bus scan, MPU6050 WHO_AM_I, register R/W, burst read, low-level ops |
| `eeprom_example.c` | Byte/word/block R/W, wear-level update, CRC-8 write/verify |
| `wdt_example.c` | Reset cause, reset mode, interrupt mode, combo mode, timed kick |
| `bitops_example.c` | Extract/insert, rotate, popcount, CLZ/CTZ, CRC-8, byte swap |
| `power_example.c` | Idle/power-down/deep sleep, PRR enable/disable, peripheral status |
| `swtimer_example.c` | Create/delete/stop/restart, periodic & one-shot, set period, active count |

---

## Architecture

```
avr-libs/
├── avr_gpio.h            # Library 1  – GPIO
├── avr_timer.h           # Library 2  – Timer/Counter
├── avr_uart.h            # Library 3  – UART Serial
├── avr_adc.h             # Library 4  – ADC
├── avr_extint.h          # Library 5  – External Interrupts
├── avr_pwm.h             # Library 6  – PWM
├── avr_spi.h             # Library 7  – SPI
├── avr_i2c.h             # Library 8  – I2C/TWI
├── avr_eeprom.h          # Library 9  – EEPROM
├── avr_wdt.h             # Library 10 – Watchdog Timer
├── avr_bitops.h          # Library 11 – Bit Manipulation
├── avr_power.h           # Library 12 – Sleep & Power
├── avr_swtimer.h         # Library 13 – Software Timers
├── examples/             # ATmega328P examples (one per library)
├── Makefile              # Build system
├── doc.md                # Design document
├── LICENSE               # MIT
└── README.md
```

### Design Principles

- **Zero external dependencies** — each library is standalone; no inter-library coupling
- **No dynamic allocation** — all buffers and pools are statically sized
- **Atomic where it matters** — SREG save/restore for multi-byte reads and R-M-W sequences
- **Portable** — `#ifdef` guards for ATmega328P / ATmega2560 / ATmega32U4
- **Configurable** — buffer sizes, pool counts, prescalers etc. via `#ifndef` macros

### Supported MCUs

| MCU | Flash | SRAM | EEPROM | Status |
|-----|-------|------|--------|--------|
| ATmega328P | 32 KB | 2 KB | 1 KB | ✅ Primary target |
| ATmega2560 | 256 KB | 8 KB | 4 KB | ✅ Compatible |
| ATmega32U4 | 32 KB | 2.5 KB | 1 KB | ✅ Compatible |

---

## Toolchain

| Tool | Version | Purpose |
|------|---------|---------|
| `avr-gcc` | 7.x+ | Compiler |
| `avr-libc` | 2.x+ | Standard library + register defs |
| `avr-objcopy` | — | ELF → HEX conversion |
| `avrdude` | — | Flash programming (optional) |

**Compiler flags:** `-std=c99 -Os -Wall -Wextra -Werror -mmcu=<mcu> -DF_CPU=<freq>`

---

## License

[MIT](LICENSE) — use freely in personal, educational, and commercial projects.
