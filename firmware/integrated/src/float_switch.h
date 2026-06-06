#pragma once
// Float switch — reservoir level (digital, GPIO27, INPUT_PULLUP).
// Per DL-040 Principle 3, read() returns a struct with a validity flag.
// Named float_switch (not float) to avoid colliding with <float.h>.
//
// Electrical: INPUT_PULLUP => HIGH when the switch is OPEN, LOW when CLOSED.
// Which physical level (empty vs full) maps to CLOSED depends on the switch
// orientation, so FLOAT_EMPTY_WHEN_CLOSED in config.h makes it flippable.
// Verify physically by lifting/dropping the float (see DL for this sensor).

struct FloatReading {
    bool reservoir_empty;  // true if the reservoir reads empty
    bool valid;            // digital read; always valid once begun
};

// Configure the pin (INPUT_PULLUP). Call once from setup().
void float_switch_begin();

// Read the switch and interpret it as empty/not-empty.
FloatReading float_switch_read();
