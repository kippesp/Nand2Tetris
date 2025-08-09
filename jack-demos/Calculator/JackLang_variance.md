# Jack Language Variance Analysis - Calculator Project

## Overview

Analysis of non-standard Jack language extensions found in the `alt/Calculator` directory, comparing against the standard Jack specification and JackOS API documentation.

## JackOS Files Comparison

### Identified JackOS Files
Standard 8 JackOS classes found: Array.jack, Keyboard.jack, Math.jack, Memory.jack, Output.jack, Screen.jack, String.jack, Sys.jack

### API Compliance Summary

#### Array.jack - ✅ MATCHES API
- Implements standard `new(int size)` and `dispose()` methods as specified
- Simple wrapper around `Memory.alloc()`/`Memory.deAlloc()`

#### Keyboard.jack - ❌ INCOMPLETE/NON-STANDARD
- **Missing API functions**: `keyPressed()`, `readChar()`, `readInt()` 
- **Non-standard method**: `clear()` on String (not in API)
- Only implements `readLine()` with basic functionality
- Uses direct memory access `keyboard[0]` instead of standard patterns

#### Math.jack - ❌ CONTAINS LANGUAGE EXTENSIONS  
- **Language extension**: Uses `register` keyword (lines 31-32, 112) - not standard Jack
- Missing API functions: `abs()`, `max()`, `min()`
- Non-standard variable name `roots` instead of standard `twoToThe` for powers of 2
- Implements core functions: `multiply()`, `divide()`, `sqrt()` but with non-standard optimizations

#### Memory.jack - ❌ SIMPLIFIED IMPLEMENTATION
- Missing error checking for invalid sizes/parameters  
- No block coalescing in `deAlloc()` - just prepends to free list
- Missing `peek()` and `poke()` functions required by API
- Basic first-fit allocation instead of specified best-fit algorithm

#### Output.jack - ⚠️ MOSTLY COMPATIBLE
- Contains all required API functions
- Character map implementation matches standard approach
- **Potential issue**: Missing `printInt()` function (referenced in API)
- Screen coordinate calculations appear correct for 512×256 display

#### Screen.jack - ❌ INCOMPLETE IMPLEMENTATION  
- **Missing functions**: `drawPixel()`, `drawCircle()`
- `drawLine()` only handles horizontal/vertical lines, not diagonal (missing Bresenham)
- No error checking for coordinate bounds
- Optimized horizontal line drawing but incomplete overall

#### String.jack - ❌ SEVERELY INCOMPLETE
- **Missing functions**: `intValue()`, `setInt()`, `newLine()`, `backSpace()`, `doubleQuote()`
- **Non-standard method**: `clear()` not in API
- Missing proper error handling for bounds/capacity checks
- Only implements basic character array operations

#### Sys.jack - ❌ INCOMPLETE ERROR HANDLING
- Missing error code display in `error()` function (should print the code)
- `wait()` timing calibration may not match API specification
- Core initialization sequence matches API requirements

## Jack Language Extensions Found

### Core Language Extensions

#### 1. **`register` Keyword**
**Locations**: Math.jack:31-32, Decimal32.jack:78-79, UInt32.jack multiple

**Non-standard usage:**
```jack
function int multiply(int x, int y) {
    register boolean isNegative;
    register int i, z;
    // ... rest of function
}
```

**Standard Jack equivalent:**
```jack
function int multiply(int x, int y) {
    var boolean isNegative;
    var int i, z;
    // ... rest of function
}
```

#### 2. **`inc`/`dec` Operators**
**Locations**: Decimal32.jack:19, 89, 136, 171, 194

**Non-standard usage:**
```jack
while (~((i1 & (~127)) = 0)) {
    inc exponent;                    // Non-standard
    do UInt32.div32x8(this, 10, this);
}

while ((i < 7) & x.isNonzero()) {
    let remainder = UInt32.div32x8(x, 10, x);
    let s[i] = 48 + remainder;
    inc i;                           // Non-standard
}

while (i > 0) {
    let sd = sd - 1;
    dec i;                           // Non-standard
    do output.appendChar(s[i]);
}
```

**Standard Jack equivalent:**
```jack
inc exponent;     →  let exponent = exponent + 1;
inc i;            →  let i = i + 1;
dec i;            →  let i = i - 1;
```

#### 3. **`bool` Type Alias**
**Locations**: UInt32.jack:19, 78, Lexer.jack:42

**Non-standard usage:**
```jack
method bool isNonzero() {           // Non-standard type
    return ~((i0 | i1) = 0);
}

method bool next() {                // Non-standard type
    var int c;
    // ... method body
}
```

**Standard Jack equivalent:**
```jack
method boolean isNonzero() {        // Standard type
    return ~((i0 | i1) = 0);
}
```

### Advanced Non-Standard Patterns

#### 4. **Complex Multi-Declaration with `register`**
**Locations**: UInt32.jack:44-46, Decimal32.jack:116-119

**Non-standard usage:**
```jack
method void format(String output) {
    register UInt32 x;              // Non-standard storage class
    register Array s;               // Multiple register declarations
    register int i, r;              // Multiple vars in one register line
    // ... method body
}
```

#### 5. **Advanced Operator Combinations**
**Location**: Math.jack:53-54

**Non-standard usage:**
```jack
while (x > 0) {
    if ((x & i) > 0) {              // Bitwise AND in conditional
        let z = z + y;
        let x = x - i;
    }
    let y = y + y;                  // Self-addition for left shift
    let i = i + i;
}
```

#### 6. **Non-Standard Method Names/Patterns**
**Locations**: String.jack:32-35, Keyboard.jack:19

**Non-standard usage:**
```jack
method void clear() {               // Not in Jack OS API
    let length = 0;
    return;
}

function String readLine(String message) {
    do buffer.clear();              // Uses non-standard clear()
    // ... rest of method
}
```

#### 7. **Advanced Floating-Point Bit Manipulation**
**Locations**: Decimal32.jack:22-25, Bits.jack:19-22

**Non-standard usage:**
```jack
let i1 = i1 + (128 * (127 + exponent));
if (sign) {
    let i1 = i1 | (~32767);         // Sign bit manipulation
}

if (a & 128) {                      // Check high bit
    return ((a & 127) * 256) | (~32767);  // Complex bit operations
}
```

## Application Files Summary

### Calculator.jack
- Sophisticated expression evaluator with recursive descent parser
- Supports parentheses, operator precedence, floating-point arithmetic
- Uses custom `Decimal32` and `Lexer` classes

### Decimal32.jack
- IEEE-like 32-bit decimal floating point implementation
- Custom floating-point with sign, exponent, significand
- Scientific notation support, normalization algorithms
- **Heavy use of `register` keyword and `inc`/`dec` operators**

### UInt32.jack
- 32-bit unsigned integer arithmetic using two 16-bit Jack integers
- Multi-word arithmetic operations (add, multiply, divide)
- Uses `bool` type alias and `register` keyword extensively

### Lexer.jack
- Mathematical expression tokenizer for numeric literals, operators, parentheses
- Supports decimal numbers with fractional parts
- Uses `bool` return type and `inc` operator

### Bits.jack
- Bit manipulation utilities for word extraction and shifting
- Low-level bit operations for multi-word arithmetic

### Main.jack
- Calculator application with GUI using `Display`, `InputDialog`, `Text` classes
- REPL loop for calculator interaction with graphics positioning

## Conclusion

The Calculator project demonstrates **significant Jack language extensions** that would require major compiler modifications:

- **`register` storage class specifier** - C-like optimization hints
- **`inc`/`dec` unary operators** - Convenience operators for increment/decrement  
- **`bool` type alias** - Alternative to standard `boolean`
- **Advanced expression parsing** and floating-point arithmetic capabilities
- **Non-standard JackOS methods** - Extensions to standard API

These extensions suggest this is either a **custom Jack dialect** or **proof-of-concept** for language enhancements, rather than standard Jack code that would compile with the reference Jack compiler.

## Impact Assessment

**Language Extension Impact**: HIGH
- Requires significant compiler frontend changes
- Adds new keywords, operators, and type aliases
- Changes fundamental language syntax and semantics

**JackOS Compatibility**: PARTIAL  
- Core OS classes present but incomplete/non-standard
- Missing many required API functions
- Contains functionality not specified in official API

**Portability**: LOW
- Code would not compile with standard Jack compiler
- Requires custom compiler supporting these extensions
- Dependencies on non-standard library methods