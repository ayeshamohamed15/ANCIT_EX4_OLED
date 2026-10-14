
#ifndef NVM_EXERCISE_H
#define NVM_EXERCISE_H

#include <stdint.h>

/* Standard return values for this exercise */
#define NVM_E_OK        0U
#define NVM_E_NOT_OK    1U

/* Configuration */
#define NVM_MAX_BLOCKS  16U

typedef uint8_t NvM_ReturnType;

/* Block configuration descriptor */
typedef struct
{
    uint16_t BlockId;
    uint16_t EepromOffset;
    uint16_t Size;
    uint8_t *RamMirrorPtr;
} NvM_BlockDescType;

/* Initialization and block registration */
void NvM_Init(void);

NvM_ReturnType NvM_RegisterBlock(uint16_t BlockId,
                                 uint16_t Offset,
                                 uint16_t Size);

NvM_ReturnType NvM_RegisterBlockWithMirror(uint16_t BlockId,
                                           uint16_t Offset,
                                           uint16_t Size,
                                           uint8_t *RamMirrorPtr);

/* Generic read/write APIs */
NvM_ReturnType NvM_ReadVariable(uint16_t BlockId,
                                uint8_t *DestRamPtr);

NvM_ReturnType NvM_WriteVariable(uint16_t BlockId,
                                 const uint8_t *SrcRamPtr);

#endif /* NVM_EXERCISE_H */
