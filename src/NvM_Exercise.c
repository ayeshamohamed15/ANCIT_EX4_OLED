#include "NvM_Exercise.h"
#include "MemIf_Sim.h"
#include "Det_Sim.h"

#include <stddef.h>
#include <string.h>

static NvM_BlockDescType NvM_BlockTable[NVM_MAX_BLOCKS];
static uint16_t NvM_BlockCount = 0U;

static NvM_BlockDescType *NvM_FindBlock(uint16_t BlockId)
{
    uint16_t i;

    for (i = 0U; i < NvM_BlockCount; i++)
    {
        if (NvM_BlockTable[i].BlockId == BlockId)
        {
            return &NvM_BlockTable[i];
        }
    }
    return NULL;
}

void NvM_Init(void)
{
    NvM_BlockCount = 0U;
    memset(NvM_BlockTable, 0, sizeof(NvM_BlockTable));

    Det_Sim_Init();
    MemIf_Sim_Init();
}

NvM_ReturnType NvM_RegisterBlockWithMirror(
    uint16_t BlockId,
    uint16_t Offset,
    uint16_t Size,
    uint8_t *RamMirrorPtr)
{
    uint16_t i;

    /* Reject invalid IDs, zero sizes, and out-of-bounds blocks. */
    if ((BlockId == 0U) ||
        (Size == 0U) ||
        (Offset >= MEMIF_SIM_EEPROM_SIZE) ||
        (Size > (MEMIF_SIM_EEPROM_SIZE - Offset)))
    {
        Det_Sim_ReportError(BlockId, DET_SIM_INVALID_PARAM);
        return NVM_E_NOT_OK;
    }

    /* Reject duplicate IDs. */
    for (i = 0U; i < NvM_BlockCount; i++)
    {
        if (NvM_BlockTable[i].BlockId == BlockId)
        {
            Det_Sim_ReportError(BlockId, DET_SIM_DUPLICATE_ID);
            return NVM_E_NOT_OK;
        }
    }

    /* Reject registration when the table is full. */
    if (NvM_BlockCount >= NVM_MAX_BLOCKS)
    {
        Det_Sim_ReportError(BlockId, DET_SIM_TABLE_FULL);
        return NVM_E_NOT_OK;
    }

    NvM_BlockTable[NvM_BlockCount].BlockId = BlockId;
    NvM_BlockTable[NvM_BlockCount].EepromOffset = Offset;
    NvM_BlockTable[NvM_BlockCount].Size = Size;
    NvM_BlockTable[NvM_BlockCount].RamMirrorPtr = RamMirrorPtr;

    NvM_BlockCount++;

    return NVM_E_OK;
}

NvM_ReturnType NvM_RegisterBlock(
    uint16_t BlockId,
    uint16_t Offset,
    uint16_t Size)
{
    return NvM_RegisterBlockWithMirror(
        BlockId, Offset, Size, NULL);
}

NvM_ReturnType NvM_ReadVariable(
    uint16_t BlockId,
    uint8_t *DestRamPtr)
{
    NvM_BlockDescType *block = NvM_FindBlock(BlockId);

    if (block == NULL)
    {
        Det_Sim_ReportError(BlockId, DET_SIM_UNKNOWN_BLOCK);
        return NVM_E_NOT_OK;
    }

    if (DestRamPtr == NULL)
    {
        Det_Sim_ReportError(BlockId, DET_SIM_INVALID_PARAM);
        return NVM_E_NOT_OK;
    }

    /* Read the registered block's exact offset and size. */
    if (MemIf_Sim_Read(
            block->EepromOffset,
            DestRamPtr,
            block->Size) != 0U)
    {
        Det_Sim_ReportError(BlockId, DET_SIM_MEMIF_ERROR);
        return NVM_E_NOT_OK;
    }

    if (block->RamMirrorPtr != NULL)
    {
        memcpy(block->RamMirrorPtr, DestRamPtr, block->Size);
    }

    return NVM_E_OK;
}

NvM_ReturnType NvM_WriteVariable(
    uint16_t BlockId,
    const uint8_t *SrcRamPtr)
{
    NvM_BlockDescType *block = NvM_FindBlock(BlockId);

    uint16_t bytesWritten = 0U;

    if (block == NULL)
    {
        Det_Sim_ReportError(BlockId, DET_SIM_UNKNOWN_BLOCK);
        return NVM_E_NOT_OK;
    }

    if (SrcRamPtr == NULL)
    {
        Det_Sim_ReportError(BlockId, DET_SIM_INVALID_PARAM);
        return NVM_E_NOT_OK;
    }

    /*
     * Split the block into page-bounded writes.
     * Each write stays within one 8-byte page.
     */
    while (bytesWritten < block->Size)
    {
        uint16_t currentOffset =
            block->EepromOffset + bytesWritten;

        uint16_t pageRemaining =
            MEMIF_SIM_PAGE_SIZE -
            (currentOffset % MEMIF_SIM_PAGE_SIZE);

        uint16_t remaining =
            block->Size - bytesWritten;

        uint16_t chunkSize =
            (remaining < pageRemaining)
                ? remaining
                : pageRemaining;
        if (MemIf_Sim_Write(
                currentOffset,
                &SrcRamPtr[bytesWritten],
                chunkSize) != 0U)
        {
            Det_Sim_ReportError(BlockId, DET_SIM_MEMIF_ERROR);
            return NVM_E_NOT_OK;
        }

        bytesWritten += chunkSize;
    }

    if (block->RamMirrorPtr != NULL)
    {
        memcpy(block->RamMirrorPtr, SrcRamPtr, block->Size);
    }

    return NVM_E_OK;
}
