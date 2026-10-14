# Exercise 4: Generic Read/Write APIs (Config-Table Driven)

**Language:** Embedded C  
**Platform:** NXP S32K144  
**IDE:** S32 Design Studio  
**Memory Backend:** Simulated EEPROM (512 bytes, 8-byte page size)  
**Display:** SSD1306 OLED (128 × 64, I²C)  
**Repository:** [ANCIT_EX4_OLED](https://github.com/ayeshamohamed15/ANCIT_EX4_OLED)

---

## 1. Overview

This project implements a configuration-table-driven Non-Volatile Memory (NvM) module for the NXP S32K144 microcontroller using S32 Design Studio and the S32 SDK.

Instead of hardcoding EEPROM addresses and data sizes inside read and write functions, the implementation uses a **block-descriptor table**. Each registered block contains a unique Block ID, EEPROM offset, data size, and optional RAM mirror pointer.

The generic `NvM_ReadVariable()` and `NvM_WriteVariable()` APIs use the registered block descriptors to locate and access memory. The write API supports data that crosses an 8-byte page boundary by dividing the write into smaller chunks.

A simulated EEPROM backend provides a 512-byte memory model for testing. Invalid parameters, unknown Block IDs, duplicate registrations, memory-boundary violations, and configuration-table overflow are handled through a simulated Development Error Tracer (DET).

An SSD1306 128 × 64 OLED display connected through LPI2C0 presents the test status and overall result.

## 2. Problem Statement

Embedded applications often store variables of different sizes in non-volatile memory. Implementing separate read and write functions for every variable leads to duplicated code and hardcoded memory addresses, making the software difficult to maintain and extend.

The objective of this exercise is to design reusable, generic read and write APIs driven by a configuration table.

The APIs must support a one-byte flag and a 20-byte data block, validate memory accesses, handle page-boundary writes, and report invalid operations without duplicating read/write logic.

## 3. Objectives

- Implement a configuration-table-driven memory block descriptor.
- Provide generic read and write APIs using Block IDs.
- Register memory blocks through `NvM_RegisterBlock()`.
- Support data blocks of different sizes.
- Validate EEPROM offsets and sizes against the 512-byte memory limit.
- Split writes across 8-byte page boundaries.
- Detect unknown IDs, duplicate IDs, invalid pointers, and invalid sizes.
- Report errors through a simulated DET module.
- Verify data integrity through read-back comparisons.
- Display execution results on an SSD1306 OLED.

## 4. Features and Implementation

### 4.1 Configuration-Table-Driven Block Management

Each memory block is described using the following structure:

```c
typedef struct {
    uint16_t BlockId;
    uint16_t EepromOffset;
    uint16_t Size;
    uint8_t *RamMirrorPtr;
} NvM_BlockDescType;
```

The table stores up to `NVM_MAX_BLOCKS` descriptors. The implementation uses:

```c
#define NVM_MAX_BLOCKS 16
```

Adding a new variable requires registering its Block ID, EEPROM offset, and size instead of modifying the generic read and write function bodies.

### 4.2 Generic Read API

```c
NvM_ReturnType NvM_ReadVariable(
    uint16_t BlockId,
    uint8_t *DestRamPtr
);
```

The function:

1. Looks up the requested Block ID.
2. Obtains the corresponding EEPROM offset and size.
3. Validates the destination pointer and memory range.
4. Reads data from the simulated EEPROM.
5. Copies the data into the destination RAM buffer.

If the Block ID is unknown or the destination pointer is invalid, the function returns `NVM_E_NOT_OK` and reports the error through DET.

### 4.3 Generic Write API

```c
NvM_ReturnType NvM_WriteVariable(
    uint16_t BlockId,
    const uint8_t *SrcRamPtr
);
```

The function:

1. Looks up the requested Block ID.
2. Retrieves the configured EEPROM location and data size.
3. Validates the source pointer and memory boundaries.
4. Writes the data to the configured EEPROM location.
5. Splits writes that cross an 8-byte page boundary into page-safe chunks.

The same API can therefore write both small variables and larger data blocks.

### 4.4 Block Registration

```c
NvM_ReturnType NvM_RegisterBlock(
    uint16_t BlockId,
    uint16_t Offset,
    uint16_t Size
);
```

The registration API validates the Block ID, size, memory boundaries, duplicate IDs, and available configuration-table capacity.

An optional `NvM_RegisterBlockWithMirror()` API supports registering a RAM mirror pointer.

### 4.5 Simulated EEPROM

The memory backend models an EEPROM with the following characteristics:

| Property | Value |
|---|---|
| Total memory size | 512 bytes |
| Initial erased value | `0xFF` |
| Page size | 8 bytes |
| Valid offsets | 0–511 |
| Maximum registered blocks | 16 |

The simulator provides read and write functions with pointer, length, and memory-boundary validation.


### 4.6 Development Error Tracer (DET)

The DET simulation records the most recent error's Block ID and error code and maintains an error counter.

Error scenarios include:

- Unknown Block ID during read or write.
- Invalid memory offset or size.
- Duplicate Block ID.
- Null source or destination pointer.
- Configuration-table overflow.

### 4.7 OLED Result Display

The SSD1306 OLED provides a visual status interface for the exercise.

| Property | Configuration |
|---|---|
| Display | SSD1306 OLED |
| Resolution | 128 × 64 pixels |
| Communication | I²C |
| Peripheral | LPI2C0 |
| SDA | PTA2 |
| SCL | PTA3 |
| I²C address | `0x3C` |

The display is initialized after the clock, pin mux, and LPI2C master driver. Test results are displayed through the OLED helper module.

## 5. System Architecture

The project follows a modular architecture that separates memory management, memory simulation, error reporting, and display output.

```text
                    main()
                       |
                 Hardware Init
                       |
                    NvM_Init()
                       |
              Register Block Table
                       |
             Execute Read/Write Tests
                       |
              +--------+--------+
              |                 |
       NvM_ReadVariable()  NvM_WriteVariable()
              |                 |
              +--------+--------+
                       |
             Configuration Table
                       |
                 MemIf_Sim
                       |
             512-byte EEPROM Model

    Error Reporting: NvM_Exercise -> Det_Sim

    Test Results: Application -> NvM_OLED -> OLED
```

## 6. Implementation Details

| Function / Module | Responsibility |
|---|---|
| `NvM_Init()` | Initializes the NvM simulation, block table, DET, and memory backend. |
| `NvM_RegisterBlock()` | Registers a block after validating its ID, offset, size, and capacity. |
| `NvM_RegisterBlockWithMirror()` | Registers a block with an optional RAM mirror. |
| `NvM_ReadVariable()` | Reads data using the registered Block ID. |
| `NvM_WriteVariable()` | Writes data using the registered Block ID and page-safe handling. |
| `MemIf_Sim_Init()` | Initializes simulated EEPROM to `0xFF`. |
| `MemIf_Sim_Read()` | Reads bytes from the simulated EEPROM. |
| `MemIf_Sim_Write()` | Writes bytes while respecting 8-byte page boundaries. |
| `Det_Sim_ReportError()` | Records detected development errors. |
| `NvM_OLED_DisplayResults()` | Displays test results on the OLED. |

## 7. Test Cases

The main application exercises the generic APIs and checks the expected results.

| Test Case | Description | Expected Result |
|---|---|---|
| Single-byte variable | Write and read a one-byte flag. | Read-back value matches. |
| 20-byte variable | Write and read a 20-byte block. | All bytes match. |
| Unknown read | Read an unregistered Block ID. | `NVM_E_NOT_OK`; DET error recorded. |
| Unknown write | Write to an unregistered Block ID. | `NVM_E_NOT_OK`; DET error recorded. |
| Invalid bounds | Register a block extending beyond offset 511. | Registration rejected. |
| Duplicate ID | Register an existing Block ID. | Registration rejected. |
| Page boundary | Write and read four bytes starting at offset 6. | Data matches across the page boundary. |
| Null read pointer | Read using a null destination pointer. | Operation rejected. |
| Null write pointer | Write using a null source pointer. | Operation rejected. |
| Zero-size block | Register a block with size zero. | Registration rejected. |
| Last valid address | Access offset 511 with size one. | Operation succeeds. |
| Table full | Exceed the 16-block table capacity. | Registration rejected. |
| DET validation | Check error count and error codes. | Expected errors recorded. |

## 8. Page-Boundary Handling

The simulated EEPROM uses an 8-byte page size. A write starting at offset 6 with a length of 4 crosses the boundary between addresses 7 and 8.

The write is split into two chunks:

**First chunk**

```text
Offset: 6–7
Length: 2 bytes
```

**Second chunk**

```text
Offset: 8–9
Length: 2 bytes
```

This demonstrates that the generic write API can handle page-crossing data without relying on hardcoded addresses for individual variables.

## 9. Memory-Bounds Validation

The valid EEPROM address range is `0` through `511`.

A registration request is rejected if its block extends beyond the memory capacity.

**Example: Invalid block**

```text
Offset = 500
Size   = 20

500 + 20 = 520 > 512
```

This block is invalid because it exceeds the available memory.

**Example: Last valid one-byte block**

```text
Offset = 511
Size   = 1

511 + 1 = 512
```

This registration is valid because the byte occupies the final available address.

## 10. Build and Run

1. Open S32 Design Studio.
2. Import the `FINALPROJECT` project into the workspace if it is not already present.
3. Confirm the S32K144 target and SDK configuration.
4. Verify the generated clock, pin mux, and LPI2C0 settings.
5. Build the project and resolve any compiler errors or warnings.
6. Connect the SSD1306 OLED using the configured I²C pins.
7. Program and run the application on the target board.
8. Observe the OLED status display.
9. Inspect the test flags, `allTestsPassed`, `failureStage`, and DET variables in the debugger.

## 11. Expected Results

If the implementation and configuration are correct:

- The one-byte flag is read back correctly.
- All 20 bytes of the larger block match the original data.
- Invalid registration and unknown Block ID operations are rejected.
- The page-boundary test preserves the data.
- The last valid EEPROM address can be accessed.
- The configuration table rejects additional blocks when full.
- DET records the expected errors.
- The overall test status indicates success.

## 12. Output

The OLED displays the test status and overall result.

<img width="720" height="1280" alt="image" src="https://github.com/user-attachments/assets/f4fe0280-04d1-40e5-a8b4-dc018bd95891" />

<img width="334" height="499" alt="image" src="https://github.com/user-attachments/assets/312ac518-5bfe-4d98-9257-f5bcd4dca6c0" />


## 13. Video Demonstration

[Watch the project demonstration on Google Drive](https://drive.google.com/file/d/11einQNfhve4ZyrtDBg2pX7eDc-FIovqo/view?usp=drivesdk)


## 14. Repository Structure

```text
ANCIT_EX4_OLED/
├── board/
├── doc/
├── Project_Settings/
├── SDK/
├── src/
│   ├── main.c
│   ├── NvM_Exercise.c
│   ├── NvM_Exercise.h
│   ├── MemIf_Sim.c
│   ├── MemIf_Sim.h
│   ├── Det_Sim.c
│   ├── Det_Sim.h
│   ├── OLED.c
│   ├── OLED.h
│   └── NvM_OLED.c
├── .project
├── .cproject
├── FINALPROJECT.mex
└── README.md
```

## 15. Design Decisions

- **Generic APIs:** One read implementation and one write implementation support multiple block sizes.
- **Configuration table:** Block metadata is separated from API logic.
- **Bounds checking:** Invalid memory ranges are rejected during registration.
- **Page-safe writes:** Writes are split according to the configured page size.
- **DET simulation:** Invalid operations can be diagnosed during development.
- **Separation of concerns:** NvM, memory simulation, error reporting, and OLED output are handled by separate modules.

## 16. Limitations

This is an educational NvM-style implementation using simulated EEPROM. It is not a complete AUTOSAR-compliant NvM module and does not demonstrate persistence across real power cycles.

The OLED output and test flags indicate software test results. Physical hardware validation should be documented separately.

## 17. Authors 

### Ayesha M

**Project:** Generic Read/Write APIs (Config-Table Driven)  
**Repository:** [ANCIT_EX4_OLED](https://github.com/ayeshamohamed15/ANCIT_EX4_OLED)  
**Output interface:** OLED

### Agnes Almeida M. L.

**Repository:** [ANCIT_project_GenericNVM_EEPROM](https://github.com/Agnes-1711/ANCIT_project_GenericNVM_EEPROM)  
**Output interface:** PuTTY

### Tryphena 

## 18. References

- NXP S32K144 documentation and reference manual.
- NXP S32 Design Studio and S32 SDK documentation.
- SSD1306 OLED controller documentation.
- Embedded C memory-management and EEPROM page-write concepts.
