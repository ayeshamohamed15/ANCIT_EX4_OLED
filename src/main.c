
#include <stdint.h>
#include <stddef.h>

#include "sdk_project_config.h"
#include "NvM_Exercise.h"
#include "Det_Sim.h"
#include "OLED.h"
#include "NvM_OLED.h"

/* LPI2C0 master state */
static lpi2c_master_state_t lpi2cMasterState;

/* Test result variables */
volatile uint8_t ReadFlag = 0U;
volatile uint8_t WriteReadStatus = 0U;

volatile uint8_t Test_SingleByte = 0U;
volatile uint8_t Test_20ByteBlock = 0U;
volatile uint8_t Test_UnknownRead = 0U;
volatile uint8_t Test_UnknownWrite = 0U;
volatile uint8_t Test_InvalidBounds = 0U;
volatile uint8_t Test_DuplicateID = 0U;
volatile uint8_t Test_TableFull = 0U;
volatile uint8_t Test_DataIntegrity = 0U;
volatile uint8_t Test_DET = 0U;

/* Additional test results */
volatile uint8_t Test_PageBoundary = 0U;
volatile uint8_t Test_NullRead = 0U;
volatile uint8_t Test_NullWrite = 0U;
volatile uint8_t Test_ZeroSize = 0U;
volatile uint8_t Test_LastAddress = 0U;

volatile uint8_t allTestsPassed = 0U;
volatile uint8_t failureStage = 0U;

int main(void)
{
    status_t status;

    uint8_t flagWrite = 0x5AU;
    uint8_t flagRead = 0U;

    uint8_t dataWrite[20];
    uint8_t dataRead[20];

    uint8_t boundaryWrite[4] =
    {
        0x11U, 0x22U, 0x33U, 0x44U
    };

    uint8_t boundaryRead[4] = {0U, 0U, 0U, 0U};

    uint8_t lastWrite = 0xA5U;
    uint8_t lastRead = 0U;

    uint8_t i;
    uint8_t dataMatch;
    uint8_t boundaryMatch;

    /* ================================================
     * STEP 1: Initialize system clocks
     * ================================================ */
    status = CLOCK_DRV_Init(&clockMan1_InitConfig0);
    DEV_ASSERT(status == STATUS_SUCCESS);

    /* ================================================
     * STEP 2: Initialize pin configuration
     * ================================================ */
    status = PINS_DRV_Init(
        NUM_OF_CONFIGURED_PINS0,
        g_pin_mux_InitConfigArr0
    );
    DEV_ASSERT(status == STATUS_SUCCESS);

    /* ================================================
     * STEP 3: Initialize LPI2C0 for the OLED
     * ================================================ */
    status = LPI2C_DRV_MasterInit(
        0U,
        &lpi2c0_MasterConfig0,
        &lpi2cMasterState
    );
    DEV_ASSERT(status == STATUS_SUCCESS);

    /* ================================================
     * STEP 4: Initialize NvM simulation and OLED
     * ================================================ */
    NvM_Init();

    OLED_Init();
    OLED_Clear();

    OLED_ShowString(8U, 0U, "NVM EXERCISE 4");
    OLED_ShowString(8U, 8U, "RUNNING TESTS");

    /* ================================================
     * TEST 1: Register a one-byte block
     * ================================================ */
    if (NvM_RegisterBlock(1U, 0U, 1U) != NVM_E_OK)
    {
        failureStage = 1U;
    }

    /* ================================================
     * TEST 2: Register a 20-byte block
     * ================================================ */
    if (NvM_RegisterBlock(2U, 8U, 20U) != NVM_E_OK)
    {
        failureStage = 2U;
    }

    /* ================================================
     * TEST 3: Write/read one byte
     * ================================================ */
    if (NvM_WriteVariable(1U, &flagWrite) == NVM_E_OK)
    {
        if (NvM_ReadVariable(1U, &flagRead) == NVM_E_OK)
        {
            ReadFlag = flagRead;

            if (flagRead == flagWrite)
            {
                Test_SingleByte = 1U;
            }
            else
            {
                failureStage = 3U;
            }
        }
        else
        {
            failureStage = 3U;
        }
    }
    else
    {
        failureStage = 3U;
    }

    /* ================================================
     * TEST 4: Write/read 20 bytes and verify integrity
     * ================================================ */
    for (i = 0U; i < 20U; i++)
    {
        dataWrite[i] = i;
        dataRead[i] = 0U;
    }

    if (NvM_WriteVariable(2U, dataWrite) == NVM_E_OK)
    {
        if (NvM_ReadVariable(2U, dataRead) == NVM_E_OK)
        {
            dataMatch = 1U;

            for (i = 0U; i < 20U; i++)
            {
                if (dataRead[i] != dataWrite[i])
                {
                    dataMatch = 0U;
                    break;
                }
            }

            if (dataMatch == 1U)
            {
                Test_20ByteBlock = 1U;
                Test_DataIntegrity = 1U;
                WriteReadStatus = 1U;
            }
            else
            {
                failureStage = 4U;
            }
        }
        else
        {
            failureStage = 4U;
        }
    }
    else
    {
        failureStage = 4U;
    }

    /* ================================================
     * TEST 5: Unknown BlockId read
     * ================================================ */
    if (NvM_ReadVariable(99U, &flagRead) == NVM_E_NOT_OK)
    {
        Test_UnknownRead = 1U;
    }
    else
    {
        failureStage = 5U;
    }

    /* ================================================
     * TEST 6: Unknown BlockId write
     * ================================================ */
    if (NvM_WriteVariable(99U, &flagWrite) == NVM_E_NOT_OK)
    {
        Test_UnknownWrite = 1U;
    }
    else
    {
        failureStage = 6U;
    }

    /* ================================================
     * TEST 7: Out-of-bounds registration
     * 500 + 20 exceeds the 512-byte memory size
     * ================================================ */
    if (NvM_RegisterBlock(100U, 500U, 20U) == NVM_E_NOT_OK)
    {
        if (Det_LastErrorId == DET_SIM_INVALID_PARAM)
        {
            Test_InvalidBounds = 1U;
        }
        else
        {
            failureStage = 7U;
        }
    }
    else
    {
        failureStage = 7U;
    }

    /* ================================================
     * TEST 8: Duplicate BlockId
     * ================================================ */
    if (NvM_RegisterBlock(1U, 30U, 1U) == NVM_E_NOT_OK)
    {
        if (Det_LastErrorId == DET_SIM_DUPLICATE_ID)
        {
            Test_DuplicateID = 1U;
        }
        else
        {
            failureStage = 8U;
        }
    }
    else
    {
        failureStage = 8U;
    }

    /* ================================================
     * TEST 9: Page-boundary write
     * Block covers offsets 6, 7, 8, 9
     * ================================================ */
    if (NvM_RegisterBlock(3U, 6U, 4U) == NVM_E_OK)
    {
        if (NvM_WriteVariable(3U, boundaryWrite) == NVM_E_OK)
        {
            if (NvM_ReadVariable(3U, boundaryRead) == NVM_E_OK)
            {
                boundaryMatch = 1U;

                for (i = 0U; i < 4U; i++)
                {
                    if (boundaryRead[i] != boundaryWrite[i])
                    {
                        boundaryMatch = 0U;
                        break;
                    }
                }

                if (boundaryMatch == 1U)
                {
                    Test_PageBoundary = 1U;
                }
                else
                {
                    failureStage = 9U;
                }
            }
            else
            {
                failureStage = 9U;
            }
        }
        else
        {
            failureStage = 9U;
        }
    }
    else
    {
        failureStage = 9U;
    }

    /* ================================================
     * TEST 10: NULL destination pointer on read
     * ================================================ */
    if (NvM_ReadVariable(1U, NULL) == NVM_E_NOT_OK)
    {
        Test_NullRead = 1U;
    }
    else
    {
        failureStage = 10U;
    }

    /* ================================================
     * TEST 11: NULL source pointer on write
     * ================================================ */
    if (NvM_WriteVariable(1U, NULL) == NVM_E_NOT_OK)
    {
        Test_NullWrite = 1U;
    }
    else
    {
        failureStage = 11U;
    }

    /* ================================================
     * TEST 12: Zero-size registration
     * ================================================ */
    if (NvM_RegisterBlock(5U, 100U, 0U) == NVM_E_NOT_OK)
    {
        if (Det_LastErrorId == DET_SIM_INVALID_PARAM)
        {
            Test_ZeroSize = 1U;
        }
        else
        {
            failureStage = 12U;
        }
    }
    else
    {
        failureStage = 12U;
    }

    /* ================================================
     * TEST 13: Last valid EEPROM address
     * Offset 511, size 1
     * ================================================ */
    if (NvM_RegisterBlock(4U, 511U, 1U) == NVM_E_OK)
    {
        if (NvM_WriteVariable(4U, &lastWrite) == NVM_E_OK)
        {
            if (NvM_ReadVariable(4U, &lastRead) == NVM_E_OK)
            {
                if (lastRead == lastWrite)
                {
                    Test_LastAddress = 1U;
                }
                else
                {
                    failureStage = 13U;
                }
            }
            else
            {
                failureStage = 13U;
            }
        }
        else
        {
            failureStage = 13U;
        }
    }
    else
    {
        failureStage = 13U;
    }

    /* ================================================
     * TEST 14: Fill the remaining block table
     *
     * Registered IDs:
     * 1, 2, 3, 4 = four blocks.
     * Fill IDs 5 through 16 = twelve blocks.
     * Total = sixteen blocks.
     * ================================================ */
    for (i = 5U; i <= 16U; i++)
    {
        if (NvM_RegisterBlock(
                i,
                (uint16_t)(40U + i),
                1U) != NVM_E_OK)
        {
            failureStage = 14U;
            break;
        }
    }

    /* Next registration must fail because table is full */
    if (NvM_RegisterBlock(17U, 100U, 1U) == NVM_E_NOT_OK)
    {
        if (Det_LastErrorId == DET_SIM_TABLE_FULL)
        {
            Test_TableFull = 1U;
        }
        else
        {
            failureStage = 15U;
        }
    }
    else
    {
        failureStage = 15U;
    }

    /* ================================================
     * TEST 15: Verify expected DET reporting
     * Unknown read/write, invalid bounds, duplicate ID,
     * NULL read/write, zero-size, and table-full errors.
     * ================================================ */
    if ((Test_UnknownRead == 1U) &&
        (Test_UnknownWrite == 1U) &&
        (Test_InvalidBounds == 1U) &&
        (Test_DuplicateID == 1U) &&
        (Test_NullRead == 1U) &&
        (Test_NullWrite == 1U) &&
        (Test_ZeroSize == 1U) &&
        (Test_TableFull == 1U) &&
        (Det_ErrorCount >= 8U))
    {
        Test_DET = 1U;
    }
    else
    {
        failureStage = 16U;
    }

    /* ================================================
     * FINAL: Overall test result
     * ================================================ */
    if ((Test_SingleByte == 1U) &&
        (Test_20ByteBlock == 1U) &&
        (Test_UnknownRead == 1U) &&
        (Test_UnknownWrite == 1U) &&
        (Test_InvalidBounds == 1U) &&
        (Test_DuplicateID == 1U) &&
        (Test_TableFull == 1U) &&
        (Test_DataIntegrity == 1U) &&
        (Test_DET == 1U) &&
        (Test_PageBoundary == 1U) &&
        (Test_NullRead == 1U) &&
        (Test_NullWrite == 1U) &&
        (Test_ZeroSize == 1U) &&
        (Test_LastAddress == 1U) &&
        (WriteReadStatus == 1U) &&
        (failureStage == 0U))
    {
        allTestsPassed = 1U;
    }
    else
    {
        allTestsPassed = 0U;
    }

    /* ================================================
     * DISPLAY FINAL RESULTS
     * ================================================ */
    NvM_OLED_DisplayResults();

    while (1)
    {
        /* Keep the final OLED results visible */
    }
}
