#include "MemIf_Sim.h"
#include <string.h>
#include <stddef.h>

/* Simulated EEPROM storage in RAM */
static uint8_t EepromMemory[MEMIF_SIM_EEPROM_SIZE];

void MemIf_Sim_Init(void)
{
    /* Initialize simulated EEPROM to erased state */
    memset(EepromMemory, 0xFF, sizeof(EepromMemory));
}

uint8_t MemIf_Sim_Read(uint16_t offset,
                       uint8_t *dest,
                       uint16_t length)
{
    if ((dest == NULL) ||
        (length == 0U) ||
        (offset >= MEMIF_SIM_EEPROM_SIZE) ||
        (length > (MEMIF_SIM_EEPROM_SIZE - offset)))
    {
        return 1U;  /* Failure */
    }

    memcpy(dest, &EepromMemory[offset], length);

    return 0U;      /* Success */
}

uint8_t MemIf_Sim_Write(uint16_t offset,
                        const uint8_t *src,
                        uint16_t length)
{
    uint16_t bytesWritten = 0U;

    if ((src == NULL) ||
        (length == 0U) ||
        (offset >= MEMIF_SIM_EEPROM_SIZE) ||
        (length > (MEMIF_SIM_EEPROM_SIZE - offset)))
    {
        return 1U;  /* Failure */
    }

    /* Split writes at 8-byte page boundaries */
    while (bytesWritten < length)
    {
        uint16_t currentOffset = offset + bytesWritten;

        uint16_t pageRemaining =
            MEMIF_SIM_PAGE_SIZE -
            (currentOffset % MEMIF_SIM_PAGE_SIZE);

        uint16_t remaining = length - bytesWritten;

        uint16_t chunkSize =
            (remaining < pageRemaining) ? remaining : pageRemaining;

        memcpy(&EepromMemory[currentOffset],
               &src[bytesWritten],
               chunkSize);

        bytesWritten += chunkSize;
    }

    return 0U;      /* Success */
}
