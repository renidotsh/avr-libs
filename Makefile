# ==============================================================================
# AVR-Core-Lib Collection – Makefile
# Compiles all 13 example programs targeting ATmega328P @ 16 MHz
# ==============================================================================

MCU      = atmega328p
F_CPU    = 16000000UL
CC       = avr-gcc
OBJCOPY  = avr-objcopy
SIZE     = avr-size

CFLAGS   = -std=c99 -Os -Wall -Wextra -Werror \
           -mmcu=$(MCU) -DF_CPU=$(F_CPU) \
           -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums

EXAMPLES = gpio timer uart adc extint pwm spi i2c \
           eeprom wdt bitops power swtimer

ELFS     = $(patsubst %,build/%_example.elf,$(EXAMPLES))
HEXS     = $(patsubst %,build/%_example.hex,$(EXAMPLES))

# ==============================================================================
.PHONY: all clean size help

all: $(ELFS)
	@echo ""
	@echo "========== BUILD COMPLETE =========="
	@echo "All 13 examples compiled successfully with -Wall -Wextra -Werror"
	@echo ""

# Pattern rule: compile example .c → .elf
build/%_example.elf: examples/%_example.c | build
	$(CC) $(CFLAGS) -o $@ $<

# Pattern rule: .elf → .hex (for flashing)
build/%_example.hex: build/%_example.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

hex: $(HEXS)

build:
	mkdir -p build

# ==============================================================================
# Size report
# ==============================================================================
size: $(ELFS)
	@echo ""
	@echo "========== SIZE REPORT =========="
	@echo "   text    data     bss     dec     hex filename"
	@echo "-----------------------------------------------"
	@for elf in $(ELFS); do \
		$(SIZE) --format=avr --mcu=$(MCU) $$elf 2>/dev/null || $(SIZE) $$elf; \
	done
	@echo ""

# ==============================================================================
# Clean
# ==============================================================================
clean:
	rm -rf build

# ==============================================================================
# Help
# ==============================================================================
help:
	@echo "AVR-Core-Lib Makefile targets:"
	@echo "  make all    – Compile all 13 examples (.elf)"
	@echo "  make hex    – Generate Intel HEX files (.hex)"
	@echo "  make size   – Show flash/SRAM usage per example"
	@echo "  make clean  – Remove build artifacts"
	@echo ""
	@echo "Individual examples:"
	@echo "  make build/gpio_example.elf"
	@echo "  make build/uart_example.elf"
	@echo "  make build/<name>_example.elf"
