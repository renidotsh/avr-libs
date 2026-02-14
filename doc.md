# AVR Single-File Library Collection - Design Document

## PROJECT OVERVIEW

**Project Name:** AVR-Core-Lib Collection  
**Target Architecture:** AVR (ATmega series: ATmega328P, ATmega2560, ATmega32U4, etc.)  
**Language:** C (C99 standard)  
**Deliverable:** 13 standalone single-file header libraries (.h files)  
**Purpose:** Demonstrate comprehensive AVR architecture expertise from bare-metal fundamentals to advanced peripheral control

---

## PROJECT GOALS & RATIONALE

### Primary Objectives:
1. **Technical Depth**: Show mastery of AVR hardware registers, interrupts, timers, and communication protocols
2. **Code Quality**: Demonstrate clean, efficient, well-documented embedded C practices
3. **Portfolio Value**: Create production-ready libraries suitable for real-world projects
4. **Educational Value**: Each library serves as reference implementation for specific AVR subsystem

### Resume Impact:
- Proves understanding of microcontroller architecture without abstraction layers
- Shows ability to write portable, reusable embedded code
- Demonstrates knowledge of hardware-software interface
- Highlights systems programming and low-level optimization skills

---

## LIBRARY COLLECTION DESIGN

### Architecture Coverage Matrix:

| Library | Complexity | AVR Concepts Covered |
|---------|-----------|----------------------|
| 1. GPIO Control | Fundamental | Port registers, DDR, PIN, PORT manipulation |
| 2. Timer/Counter | Fundamental | Timer modes, prescalers, OCR registers |
| 3. UART Serial | Fundamental | USART, baud rate calc, ring buffers |
| 4. ADC Interface | Fundamental | Analog conversion, reference voltage, prescaler |
| 5. External Interrupts | Intermediate | INT0/1, PCINT, ISR implementation |
| 6. PWM Generator | Intermediate | Fast/Phase-correct PWM, timer modes |
| 7. SPI Master/Slave | Intermediate | Synchronous serial, SPCR/SPSR registers |
| 8. I2C/TWI | Intermediate | Two-wire protocol, start/stop, ACK/NACK |
| 9. Internal EEPROM | Intermediate | Non-volatile storage, EEAR/EEDR registers |
| 10. Watchdog Timer | Intermediate | System reset, timeout configuration |
| 11. Bit Manipulation | Advanced | Atomic ops, bit-field extraction, endianness |
| 12. Sleep & Power Mgmt | Advanced | Sleep modes, BOD, power reduction register |
| 13. Software Timer | Advanced | Tick-based timing, callback system, no malloc |

---

## DETAILED LIBRARY SPECIFICATIONS

### 1. **avr_gpio.h** - GPIO Control Library
**Complexity:** Fundamental  
**Features:**
- Pin mode configuration (input/pullup/output)
- Digital read/write operations
- Port-level operations for speed
- Pin toggling
- Atomic bit operations using _BV()

**Key Concepts:**
- DDRx, PORTx, PINx registers
- Bitwise operations for individual pin control
- Read-modify-write hazards

---

### 2. **avr_timer.h** - Timer/Counter Management
**Complexity:** Fundamental  
**Features:**
- Timer initialization (Timer0, Timer1, Timer2)
- Prescaler configuration
- Overflow interrupt setup
- Millisecond delay using timer
- Microsecond timing functions

**Key Concepts:**
- TCCRx registers, TIMSK, TIFR
- CTC and Normal modes
- ISR(TIMER_OVF_vect) implementation

---

### 3. **avr_uart.h** - UART Serial Communication
**Complexity:** Fundamental  
**Features:**
- Baud rate auto-calculation from F_CPU
- Interrupt-driven TX/RX with ring buffers
- Blocking send/receive functions
- Printf integration support
- Error detection (frame, parity, overrun)

**Key Concepts:**
- UBRR calculation
- UCSRxA/B/C configuration
- Circular buffer implementation
- USART_RXC/TXC/UDRE interrupts

---

### 4. **avr_adc.h** - Analog to Digital Converter
**Complexity:** Fundamental  
**Features:**
- Single-ended conversion (ADC0-ADC7)
- Reference voltage selection (AREF, AVCC, Internal)
- Prescaler auto-configuration
- 10-bit result handling
- Free-running mode option

**Key Concepts:**
- ADMUX register (channel + reference)
- ADCSRA register (enable, start, prescaler)
- ADC interrupt handling

---

### 5. **avr_extint.h** - External Interrupts
**Complexity:** Intermediate  
**Features:**
- INT0/INT1 configuration
- Pin change interrupts (PCINT0-2)
- Edge detection (rising/falling/both/low)
- User callback registration
- Debouncing helper functions

**Key Concepts:**
- EICRA/EIMSK registers
- PCICR/PCMSK registers
- ISR context and volatility

---

### 6. **avr_pwm.h** - PWM Signal Generation
**Complexity:** Intermediate  
**Features:**
- Fast PWM and Phase-correct PWM
- 8-bit and 16-bit modes
- Duty cycle setting (0-100%)
- Frequency configuration
- Multi-channel support (OC0A/B, OC1A/B, OC2A/B)

**Key Concepts:**
- COM bits for output compare
- WGM bits for waveform generation
- OCRx registers for duty cycle

---

### 7. **avr_spi.h** - SPI Communication
**Complexity:** Intermediate  
**Features:**
- Master and Slave modes
- Configurable clock polarity/phase
- Speed configuration (fosc/4 to fosc/128)
- Byte and buffer transfer
- CS pin management helpers

**Key Concepts:**
- SPCR, SPSR, SPDR registers
- MOSI/MISO/SCK pin configuration
- SPI interrupt handling

---

### 8. **avr_i2c.h** - I2C/TWI Communication
**Complexity:** Intermediate  
**Features:**
- Master mode implementation
- Start/Stop condition generation
- Write/Read with ACK handling
- 7-bit addressing
- Clock speed configuration (100kHz, 400kHz)

**Key Concepts:**
- TWCR, TWSR, TWDR, TWBR registers
- State machine implementation
- Status code checking

---

### 9. **avr_eeprom.h** - Internal EEPROM Manager
**Complexity:** Intermediate  
**Features:**
- Byte read/write with wear leveling awareness
- Block read/write operations
- Busy-wait and interrupt-driven modes
- Data integrity helpers (checksums)
- Atomic operations

**Key Concepts:**
- EEAR, EEDR, EECR registers
- Write timing requirements
- EEPROM ready interrupt

---

### 10. **avr_wdt.h** - Watchdog Timer
**Complexity:** Intermediate  
**Features:**
- Timeout period configuration (15ms - 8s)
- System reset mode
- Interrupt mode
- Safe enable/disable sequences
- Reset cause detection

**Key Concepts:**
- WDTCSR register
- Timed sequence for configuration
- MCUSR register for reset source

---

### 11. **avr_bitops.h** - Advanced Bit Manipulation
**Complexity:** Advanced  
**Features:**
- Bit field extraction/insertion
- Rotate and shift operations
- Population count (number of set bits)
- Leading/trailing zero count
- CRC-8 calculation
- Endianness conversion

**Key Concepts:**
- Compiler intrinsics vs inline asm
- Optimization for AVR instruction set
- Look-up table techniques

---

### 12. **avr_power.h** - Sleep & Power Management
**Complexity:** Advanced  
**Features:**
- Sleep mode selection (Idle, ADC NR, Power-down, etc.)
- Peripheral power reduction (PRR register)
- Brown-out detector configuration
- Wake-up source configuration
- Power consumption calculation helpers

**Key Concepts:**
- SMCR register
- PRR register for peripheral shutdown
- BOD disable sequence
- Wake-up interrupt coordination

---

### 13. **avr_swtimer.h** - Software Timer System
**Complexity:** Advanced  
**Features:**
- Tick-based timer system (driven by hardware timer ISR)
- Multiple software timers (one-shot and periodic)
- Callback mechanism (function pointers)
- No dynamic memory allocation
- Timer pool with fixed size
- Microsecond resolution support

**Key Concepts:**
- Linked list or array-based timer management
- ISR-safe operations
- Callback execution in main loop vs ISR
- Jitter minimization

---

## NAMING CONVENTIONS & CODE STANDARDS

### File Naming:
- Format: `avr_<subsystem>.h`
- Examples: `avr_gpio.h`, `avr_uart.h`, `avr_i2c.h`

### Function Naming:
```
<SUBSYSTEM>_<Action><Object>()

Examples:
  GPIO_SetPin()
  GPIO_ReadPin()
  UART_SendByte()
  ADC_ReadChannel()
  PWM_SetDutyCycle()
  I2C_WriteByte()
```

### Constant Naming:
- Macros: `SUBSYSTEM_CONSTANT_NAME`
- Example: `UART_BUFFER_SIZE`, `ADC_PRESCALER_128`

### Type Naming:
- Structs: `subsystem_config_t`, `subsystem_handle_t`
- Enums: `subsystem_mode_e`

### Register Access Pattern:
```c
// Direct register manipulation with comments
DDRB |= (1 << PB5);  // Set PB5 as output

// Use _BV() macro for clarity
PORTD &= ~_BV(PD3);  // Clear PD3
```

---

## TECHNICAL REQUIREMENTS

### Compiler & Toolchain:
- **Compiler:** avr-gcc (GCC 7.x or newer)
- **Standard:** C99 (-std=c99)
- **Optimization:** -Os (optimize for size)
- **Flags:** `-Wall -Wextra -Werror` for clean compilation

### Hardware Support:
- **Primary targets:** ATmega328P, ATmega2560, ATmega32U4
- **Portability:** Use `#ifdef` for device-specific features
- **Register access:** Via `<avr/io.h>` definitions

### Memory Constraints:
- **Flash:** Minimize code size (target < 1KB per library average)
- **SRAM:** No malloc/free usage
- **Stack:** Minimize depth in ISRs
- **EEPROM:** Document usage if applicable

### Dependencies:
**Allowed:**
- `<avr/io.h>` - Register definitions
- `<avr/interrupt.h>` - ISR macros
- `<avr/pgmspace.h>` - PROGMEM support
- `<avr/sleep.h>` - Sleep mode macros
- `<avr/wdt.h>` - Watchdog macros
- `<stdint.h>` - Fixed-width types
- `<stdbool.h>` - Boolean type

**Prohibited:**
- Arduino libraries
- Any external dependencies between these libraries
- Standard C library functions that bloat code (sprintf, malloc, etc.)

---

## DOCUMENTATION REQUIREMENTS

### Header Comments:
```c
/**
 * @file     avr_gpio.h
 * @brief    GPIO control library for AVR microcontrollers
 * @author   [Your Name]
 * @date     2026
 * @version  1.0
 * 
 * @description
 * Provides direct register-level control of GPIO pins with
 * simple and efficient API. Supports individual pin control
 * and port-level operations for high-speed I/O.
 * 
 * @features
 * - Pin mode configuration (input/output/input-pullup)
 * - Atomic read/write operations
 * - Port-level bulk operations
 * - Pin toggling with single instruction
 * 
 * @example
 *   GPIO_SetMode(PORTB, PB5, GPIO_OUTPUT);
 *   GPIO_WritePin(PORTB, PB5, HIGH);
 * 
 * @target   ATmega328P, ATmega2560, ATmega32U4
 * @license  MIT License
 */
```

### Function Documentation:
```c
/**
 * @brief Configures pin direction and pull-up
 * @param port Port register (PORTB, PORTC, PORTD)
 * @param pin Pin number (0-7)
 * @param mode GPIO_INPUT, GPIO_OUTPUT, GPIO_INPUT_PULLUP
 * @return None
 * @note Modifies DDRx and PORTx registers atomically
 */
void GPIO_SetMode(volatile uint8_t *port, uint8_t pin, uint8_t mode);
```

### Example Usage Section:
Each library must include complete working examples showing:
- Basic initialization
- Common use cases
- Integration with ISRs (if applicable)
- Error handling patterns

---

## FILE STRUCTURE TEMPLATE

```c
#ifndef AVR_SUBSYSTEM_H
#define AVR_SUBSYSTEM_H

/* ===== INCLUDES ===== */
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

/* ===== CONFIGURATION MACROS ===== */
#ifndef SUBSYSTEM_CONFIG_VALUE
#define SUBSYSTEM_CONFIG_VALUE 64
#endif

/* ===== PUBLIC CONSTANTS ===== */
#define SUBSYSTEM_CONSTANT  0x01

/* ===== TYPE DEFINITIONS ===== */
typedef enum {
    SUBSYSTEM_MODE_A,
    SUBSYSTEM_MODE_B
} subsystem_mode_e;

/* ===== PUBLIC FUNCTION PROTOTYPES ===== */
void SUBSYSTEM_Init(void);
void SUBSYSTEM_DoSomething(uint8_t param);

/* ===== INLINE FUNCTIONS (if performance critical) ===== */
static inline void SUBSYSTEM_FastOperation(void) {
    // Implementation
}

/* ===== IMPLEMENTATION SECTION ===== */
// Function implementations go here

/* ===== EXAMPLE USAGE ===== */
#if 0
int main(void) {
    SUBSYSTEM_Init();
    // Usage example
    while(1) {
        // ...
    }
}
#endif

#endif /* AVR_SUBSYSTEM_H */
```

---

## QUALITY ASSURANCE

### Testing Strategy:
1. **Hardware Testing:**
   - Test on ATmega328P (Arduino Uno/Nano)
   - Test on ATmega2560 (Arduino Mega)
   - Verify timing with oscilloscope/logic analyzer

2. **Code Quality:**
   - Zero warnings with `-Wall -Wextra`
   - Static analysis with cppcheck
   - Memory usage analysis (avr-size)

3. **Documentation:**
   - Every public function documented
   - Example code compiles and works
   - Register usage explained in comments

### Performance Benchmarks:
- Document clock cycles for critical functions
- Measure interrupt latency
- Calculate actual timing values

---

## IMPLEMENTATION PHASES

### Phase 1: Foundation (Week 1-2)
- Libraries 1-4: GPIO, Timer, UART, ADC
- Establish coding standards
- Create template structure

### Phase 2: Communication (Week 3-4)
- Libraries 5-8: ExtInt, PWM, SPI, I2C
- Cross-testing between protocols
- Performance optimization

### Phase 3: Advanced (Week 5-6)
- Libraries 9-13: EEPROM, WDT, BitOps, Power, SwTimer
- Integration examples
- Documentation polish

### Phase 4: Portfolio Prep (Week 7)
- README.md with feature matrix
- GitHub repository setup
- Demo projects (blink, sensors, communication)
- Video demonstration (optional)

---

## PORTFOLIO & RESUME PRESENTATION

### GitHub Repository Structure:
```
avr-core-lib/
├── README.md
├── LICENSE
├── docs/
│   ├── design_document.txt
│   └── api_reference.md
├── libraries/
│   ├── avr_gpio.h
│   ├── avr_timer.h
│   ├── avr_uart.h
│   └── ... (all 13 libraries)
├── examples/
│   ├── gpio_blink/
│   ├── uart_echo/
│   ├── spi_communication/
│   └── power_sleep_demo/
└── tests/
    └── hardware_test_suite/
```

### Resume Bullet Points:
- "Developed 13 bare-metal C libraries for AVR microcontrollers demonstrating deep understanding of hardware registers, interrupts, timers, and communication protocols"
- "Implemented interrupt-driven UART with ring buffers, SPI/I2C master protocols, and advanced power management achieving XX% power reduction"
- "Created production-ready embedded libraries with zero external dependencies, comprehensive documentation, and hardware-validated on multiple AVR platforms"
- "Designed software timer system using cooperative scheduling with microsecond resolution without dynamic memory allocation"

### Key Differentiators:
✓ No Arduino framework - pure register-level programming  
✓ Production-quality code, not tutorial snippets  
✓ Hardware-validated with oscilloscope measurements  
✓ Performance benchmarks and timing guarantees  
✓ Covers complete AVR architecture systematically  

---

## TECHNICAL DEPTH INDICATORS

To prove expertise, each library must demonstrate:

1. **Register Knowledge:** Direct manipulation of control registers
2. **Timing Awareness:** Clock calculations, baud rates, PWM frequencies
3. **Interrupt Handling:** ISR implementation, atomicity, volatile usage
4. **Hardware Constraints:** SRAM limits, flash optimization, power consumption
5. **Portability:** Preprocessor guards for different AVR models
6. **Edge Cases:** Error handling, boundary conditions, hardware errata

---

## SUCCESS CRITERIA

### Technical Success:
- [x] All 13 libraries compile with zero warnings
- [x] Each library tested on real hardware
- [x] Total collection < 15KB flash, < 512B SRAM
- [x] Complete API documentation

### Professional Success:
- [x] Clean, consistent code style
- [x] Professional documentation
- [x] Portfolio-ready presentation
- [x] Demonstrates progression from basic to advanced

### Resume Impact:
- Shows embedded systems expertise
- Proves C programming mastery
- Demonstrates hardware understanding
- Indicates ability to create reusable components

---

## CONCLUSION

This library collection serves as a comprehensive demonstration of AVR architecture mastery, moving systematically from fundamental GPIO control through intermediate communication protocols to advanced power management and software timing systems. Each library is a self-contained, production-ready module that solves real embedded systems problems without abstraction layers or dependencies.

**Estimated Total Effort:** 6-7 weeks part-time  
**Lines of Code:** ~3,000-4,000 (including documentation)  
**Flash Usage:** 10-15KB total when all libraries used  
**Portfolio Value:** Senior Embedded Systems Engineer level

---

**Document Version:** 1.0  
**Date:** February 14, 2026  
**Status:** Ready for Implementation


### EACH LIBRARY IMPLEMENT SHOULD ALSO HAVE AN EXAMPLE IMPLEMENTING EACH FEATURE OF IT IN ATMEGA328P