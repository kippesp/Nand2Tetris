# Jack Language Extensions Reversal Guide

## Overview

The JackCompiler.py in the nand2tetris-sample repository implements several extensions to the standard Jack language. This document identifies all extensions and provides detailed strategies for converting extended Jack programs back to standard Jack.

## Language Extensions Identified

### 1. Register Variables (`register` keyword)

**Extension**: The `register` keyword allows declaring variables that are stored in static memory but scoped to individual subroutines.

**Syntax**:
```jack
function void myFunction() {
    register int tempValue;
    // function body
}
```

**Reversal Strategy**:
- Replace `register` variables with `var` (local) variables
- If register semantics are critical (persistent across calls), promote to `static` class variables with unique naming

**Example Conversion**:
```jack
// Extended Jack
class MyClass {
    function void process() {
        register int counter;
        // ...
    }
}

// Standard Jack
class MyClass {
    static int process_counter;  // If persistence needed
    
    function void process() {
        var int counter;  // If local scope sufficient
        // ...
    }
}
```

### 2. Direct Memory Access (`let @address = value`)

**Extension**: Direct memory address assignment using `@` symbol.

**Syntax**:
```jack
let @16384 = 255;        // Write to absolute address
let @screenPtr = ~0;     // Write negated value
let @var = -42;          // Write negative value
```

**Reversal Strategy**:
- **Primary**: Replace with standard `Memory.poke()` and `Memory.peek()` calls
- **Alternative optimization**: Use array access via RAM base pointer

**Example Conversion**:
```jack
// Extended Jack  
let @16384 = 255;
let @ptr = ~value;

// Standard Jack (Primary method)
do Memory.poke(16384, 255);
do Memory.poke(ptr, ~value);

// Standard Jack (Alternative optimization)
var int MEMORY;
let MEMORY = 0;  // Base of RAM
let MEMORY[16384] = 255;
let MEMORY[ptr] = ~value;
```

**Note**: The array-based approach (`MEMORY[addr] = value`) can be more efficient than `Memory.poke()` calls but requires careful setup of the base pointer.

### 3. Increment/Decrement Statements (`inc`, `dec`, `inv`)

**Extension**: Dedicated increment, decrement, and invert statements with optional step values.

**Syntax**:
```jack
inc counter;          // counter = counter + 1
dec counter 5;        // counter = counter - 5  
inv flags;            // flags = ~flags
inc array[index] 2;   // array[index] = array[index] + 2
```

**Reversal Strategy**:
- Convert to standard assignment statements
- For array access, use temporary variables

**Example Conversion**:
```jack
// Extended Jack
inc counter;
dec counter 5;
inv flags;
inc array[index] 2;

// Standard Jack
let counter = counter + 1;
let counter = counter - 5;
let flags = ~flags;
let array[index] = array[index] + 2;
```

### 4. Load/Store Instructions (`ldd`, `sto`)

**Extension**: Load and store instructions for temporary register operations.

**Syntax**:
```jack
ldd 42;              // Load 42 into temp register
ldd ~value;          // Load ~value into temp register
ldd variable;        // Load variable into temp register
sto @address;        // Store temp register to address
```

**Reversal Strategy**:
- Use explicit temporary variables to simulate register behavior
- Convert to standard assignment patterns

**Example Conversion**:
```jack
// Extended Jack
ldd 42;
sto @ptr;

// Standard Jack (Primary method)
var int tempReg;
let tempReg = 42;
do Memory.poke(ptr, tempReg);

// Standard Jack (Alternative optimization)
var int tempReg;
var int MEMORY;
let MEMORY = 0;
let tempReg = 42;
let MEMORY[ptr] = tempReg;
```

### 5. Pragma Directives (`#pragma`)

**Extension**: Compiler optimization hints.

**Syntax**:
```jack
#pragma optimizeArrayAssignment
class MyClass {
    // class body
}
```

**Reversal Strategy**:
- Simply remove pragma directives as they are optimization hints
- Ensure resulting code maintains correct functionality

**Example Conversion**:
```jack
// Extended Jack
#pragma optimizeArrayAssignment
class MyClass {
    // body
}

// Standard Jack
class MyClass {
    // body - pragma removed
}
```

### 6. Extended VM Operations (when `-x` flag used)

**Extension**: Extended VM instructions that generate different bytecode.

**Affected Operations**:
- `function-ext` vs `function`
- `call-ext` vs `call` 
- Extended arithmetic operations
- New instructions: `poke`, `inc`, `dec`, `inv`, `ldd`, `sto`, `drop`

**Reversal Strategy**:
- This is a backend optimization - no source code changes needed
- Ensure compiler is run without `-x` flag to generate standard VM code

## Comprehensive Reversal Process

### Step 1: Remove Pragma Directives
```bash
sed -i '/#pragma/d' *.jack
```

### Step 2: Convert Register Variables
```bash
# Find all register declarations
grep -n "register " *.jack
# Manually convert each to var or static as appropriate
```

### Step 3: Convert Direct Memory Access
Replace patterns like:
- `let @(\d+) = (.+);` → `do Memory.poke($1, $2);`
- `let @([a-zA-Z_]\w*) = (.+);` → `do Memory.poke($1, $2);`

### Step 4: Convert Inc/Dec/Inv Statements
Replace patterns:
- `inc ([a-zA-Z_]\w*)( \d+)?;` → `let $1 = $1 + $2;` (default step=1)
- `dec ([a-zA-Z_]\w*)( \d+)?;` → `let $1 = $1 - $2;`  
- `inv ([a-zA-Z_]\w*);` → `let $1 = ~$1;`

Handle array access specially:
- `inc array\[expr\]` → use temp variable pattern

### Step 5: Convert Load/Store Instructions
- Track `ldd` statements and corresponding `sto` statements
- Replace with explicit temporary variables and assignments

### Step 6: Verify Syntax Compliance
- Ensure all keywords are from standard Jack set
- Verify no extended symbols (`@` outside string constants)
- Check that all statements follow standard Jack grammar

## Template for Manual Conversion

```jack
// Original extended Jack class template
class ExtendedClass {
    #pragma optimizeArrayAssignment
    register int reg1;
    static int staticVar;
    field int fieldVar;
    
    function void extendedFunction() {
        register int localReg;
        var int normalVar;
        
        let @16384 = 255;
        inc localReg 5;
        dec staticVar;
        inv fieldVar;
        ldd 42;
        sto @normalVar;
    }
}

// Converted standard Jack class (Primary method using Memory.poke/peek)
class ExtendedClass {
    static int staticVar;
    static int reg1;              // Promoted register to static
    field int fieldVar;
    
    function void extendedFunction() {
        var int localReg;         // Converted register to var
        var int normalVar;
        var int tempReg;          // For ldd/sto simulation
        
        do Memory.poke(16384, 255);
        let localReg = localReg + 5;
        let staticVar = staticVar - 1;
        let fieldVar = ~fieldVar;
        let tempReg = 42;
        do Memory.poke(normalVar, tempReg);
    }
}

// Alternative conversion using array optimization
class ExtendedClass {
    static int staticVar;
    static int reg1;
    field int fieldVar;
    
    function void extendedFunction() {
        var int localReg;
        var int normalVar;
        var int tempReg;
        var int MEMORY;           // RAM base pointer
        let MEMORY = 0;
        
        let MEMORY[16384] = 255;
        let localReg = localReg + 5;
        let staticVar = staticVar - 1;
        let fieldVar = ~fieldVar;
        let tempReg = 42;
        let MEMORY[normalVar] = tempReg;
    }
}
```

## Validation Checklist

After conversion, verify:

- [ ] No pragma directives remain
- [ ] No `register` keyword usage  
- [ ] No `@` symbols outside strings
- [ ] No `inc`, `dec`, `inv` keywords
- [ ] No `ldd`, `sto` keywords
- [ ] All variables properly declared as `var`, `static`, or `field`
- [ ] All assignments use standard `let` syntax
- [ ] Code compiles with standard Jack compiler
- [ ] Functionality preserved (test with known inputs/outputs)

## Notes on Semantic Preservation

- **Register variables**: Choice between `var` and `static` affects semantics
  - Use `var` for truly local temporary values
  - Use `static` when persistence across calls is needed
- **Direct memory access**: `Memory.poke()` and `Memory.peek()` are functionally equivalent but require Memory class
  - Array-based approach (`var int MEMORY; let MEMORY = 0; let MEMORY[addr] = value;`) can be more efficient
- **Array optimization pragma**: Removal may affect performance but not correctness
- **Load/store pattern**: May require multiple temporary variables for complex sequences

This guide enables systematic conversion of any Jack program using the identified extensions back to standard Jack language compliance.