/******************************************************************************
  * @attention
  *
  * COPYRIGHT 2020 STMicroelectronics, all rights reserved
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
******************************************************************************/


/*
 *      PROJECT:   st25tags library
 *      Revision:
 *      LANGUAGE:  ISO C99
 */

/*! \file
 *
 *  \author MMY Ecosystem, (c)2020
 *
 *  \brief st25dv-pwm higher level interface
 *
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "st25dv_pwm.h"
#include "rfal_nfcv.h"
#include "rfal_st25xv.h"
#include "utils.h"


/*
 ******************************************************************************
 * GLOBAL DEFINES
 ******************************************************************************
 */
#define PWMx_CTRL_PWM_ENABLE_SHIFT              7U
#define PWMx_CTRL_PWM_ENABLE_BLOCK              3U
#define PWMx_CTRL_PWM_ENABLE_BITMASK            0x80
#define PWMx_CTRL_RFU_BITMASK                   0x80

#define PWM1_CTRL_AREA_START_ADDR               0xF8U       /*!< PWM1 Control Byte address */
#define PWM2_CTRL_AREA_START_ADDR               0xF9U       /*!< PWM2 Control Byte address */

#define SYS_ADDR_AREA1_SECURITY_ATTRIBUTES      0x00
#define SYS_ADDR_AREA2_SECURITY_ATTRIBUTES      0x01
#define SYS_ADDR_PWM_CTRL_SECURITY_ATTRIBUTES   0x02
#define SYS_ADDR_PWM_CONFIG                     0x03
#define SYS_ADDR_LOCK_CONFIG                    0x04

#define SYS_A1SA_MEM_ORG_SHIFT                  2U
#define SYS_AxSA_RW_PROTECTION_BITMASK          0x03

#define ST25DV_W1_PRODUCT_CODE                  0x38
#define ST25DV_W2_PRODUCT_CODE                  0x39


/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
 char ST25TAGS_ST25DVPWM_tagName[][16] = {
    "UNKNOWN_TAG ",
    "ST25DV512_W1",
    "ST25DV512_W2",
    "ST25DV01K_W1",
    "ST25DV01K_W2",
    "ST25DV02K_W1",
    "ST25DV02K_W2",
};

/*
 ******************************************************************************
 * CONFIGURATION
 ******************************************************************************
 */

ReturnCode ST25TAGS_DVPWM_OpenSecuritySession(const uint8_t * uid, ST25TAG_DVPWM_SecuritySession_t sessionType, const uint8_t * pPassword)
{
    uint8_t     pwdNumber;
    uint8_t     pwdLengthInBytes = 4;
    ReturnCode  errCode;

    switch (sessionType)
    {
        case CONFIGURATION_SECURITY_SESSION:
            pwdNumber = 3;
            break;
        case USER_AREA1_SECURITY_SESSION:
            pwdNumber = 1;
            break;
        case USER_AREA2_SECURITY_SESSION:
            pwdNumber = 2;
            break;
        case SINGLE_USER_AREA_SECURITY_SESSION:
            pwdLengthInBytes = 8;
            pwdNumber = 1;
            break;
        case PWM_CTRL_SECURITY_SESSION:
            pwdNumber = 0;
            break;
        default:
            return ERR_PARAM;
    }

    /* Present password */
    errCode = rfalST25xVPollerPresentPassword( RFAL_NFCV_REQ_FLAG_DEFAULT | RFAL_NFCV_REQ_FLAG_ADDRESS, uid, pwdNumber, pPassword, pwdLengthInBytes);

    // Same ReturnCode definitions used in st_errno.h of RFAL
    return errCode;
}


/*******************************************************************************/
bool ST25TAGS_DVPWM_isTagSt25DvPwm(const uint8_t* uid)
{
    /* TODO: Move those definitions to a generic ST25TAGS Type    5 header? */
    const uint8_t   maxUidIndex = RFAL_NFCV_UID_LEN - 1;    // NFC Forum Type 5 UID is 8-byte long
    const uint8_t   iso15693MagicNumberIndex = maxUidIndex;
    const uint8_t   mfrIndex = maxUidIndex - 1;
    const uint8_t   productCodeIndex = maxUidIndex - 2;

    if ( (uid[iso15693MagicNumberIndex] == 0xE0) && (uid[mfrIndex] == ST25TAGS_ST_MFR_CODE) )
    {
        // STMicroelectronics Iso15693 tag recognized
        if ( (uid[ productCodeIndex ] == (ST25DV_W1_PRODUCT_CODE))
            || (uid[ productCodeIndex ] == (ST25DV_W2_PRODUCT_CODE)) )
        {
            return true;
        }
    }

    return false;
}


/*******************************************************************************/
ST25TAGS_DVPWM_TagType_t ST25TAGS_DVPWM_GetSt25PwmTagType(const uint8_t* uid)
{
    /* TODO: Move those definitions to generic ST25TAGS Type 5 header? */
    const uint8_t   maxUidIndex = RFAL_NFCV_UID_LEN - 1;    // NFC Forum Type 5 UID is 8-byte long
    const uint8_t   iso15693MagicNumberIndex = maxUidIndex;
    const uint8_t   mfrIndex = maxUidIndex - 1;
    const uint8_t   productCodeIndex = maxUidIndex - 2;

    ReturnCode      ret;
    uint8_t         rxBuf[ST25TAGS_DVPWM_SYSINFO_MAX_LEN];
    uint16_t        rcvLen;
    uint16_t        userDataMemorySizeInBits;
    uint8_t         icRef;

    ret = rfalNfcvPollerGetSystemInformation( RFAL_NFCV_REQ_FLAG_DEFAULT | RFAL_NFCV_REQ_FLAG_ADDRESS, uid, rxBuf, (uint16_t)sizeof(rxBuf), &rcvLen );

    if( (ret != ERR_NONE) || (rcvLen < 2) || (rcvLen > (ST25TAGS_DVPWM_SYSINFO_MAX_LEN)) )
    {
        return UNKNOWN_DVPWM_TAG;
    }

    if ( (uid[iso15693MagicNumberIndex] == 0xE0) && (uid[mfrIndex] == ST25TAGS_ST_MFR_CODE) )
    {
        /* An STMicroelectronics Iso15693 tag was recognized.
           Check tag memory size in bits by reading the content of the getSystemInfo command. */
        userDataMemorySizeInBits = (rxBuf[ST25TAGS_DVPWM_SYSINFO_BLOCKSIZE_INDEX] + 1) * (rxBuf[ST25TAGS_DVPWM_SYSINFO_USERBLOCKS_INDEX] + 1) * 8;
        icRef = rxBuf[ST25TAGS_DVPWM_SYSINFO_ICREF_INDEX];

        if ( (icRef == (ST25DV_W1_PRODUCT_CODE)) && (uid[ productCodeIndex ] == (ST25DV_W1_PRODUCT_CODE)) )
        {
            switch ( userDataMemorySizeInBits )
            {
                case 1024:
                    return ST25DV01K_W1;
                case 2048:
                    return ST25DV02K_W1;
                case 512:
                default:
                    return ST25DV512_W1;
            }
        }

        if ( (icRef == (ST25DV_W2_PRODUCT_CODE)) && (uid[ productCodeIndex ] == (ST25DV_W2_PRODUCT_CODE)) )
        {
            switch ( userDataMemorySizeInBits )
            {
                case 1024:
                    return ST25DV01K_W2;
                case 2048:
                    return ST25DV02K_W2;
                case 512:
                default:
                    return ST25DV512_W2;
            }
        }
    }

    return UNKNOWN_DVPWM_TAG;
}


/*
 ******************************************************************************
 * AREA MANAGEMENT
 ******************************************************************************
 */

/*******************************************************************************/
ReturnCode ST25TAGS_DVPWM_SetNumberOfUserAreas(const uint8_t* uid, uint8_t numberOfUserAreas)
{
    ReturnCode  errCode;
    uint8_t     regValue;

    if (numberOfUserAreas < 1 || numberOfUserAreas > 2)
    {
        return ERR_PARAM;
    }

    /* Sets content of register A1SA bit MEM_ORG (b2) to the wanted value */
    errCode = rfalST25xVPollerReadConfiguration( RFAL_NFCV_REQ_FLAG_DEFAULT | RFAL_NFCV_REQ_FLAG_ADDRESS, uid, SYS_ADDR_AREA1_SECURITY_ATTRIBUTES, &regValue );
    if ( errCode != ERR_NONE )
    {
        return errCode;
    }

    if (numberOfUserAreas == 1)
    {
        if ( ((regValue >> SYS_A1SA_MEM_ORG_SHIFT) & 1U) == 0x01 )
        {
            /* No need to update the register, already set */
            return ERR_NONE;
        }
        else
        {
            /* 1 area, register b2 bit not already set to 1 -> set MEM_ORG to 1 */
            regValue |= (1U << SYS_A1SA_MEM_ORG_SHIFT);
        }
    }
    else
    {
        if ( ((regValue >> SYS_A1SA_MEM_ORG_SHIFT) & 1U) == 0x00 )
        {
            /* No need to update the register, already set */
            return ERR_NONE;
        }
        else
        {
            /* 2 areas, register b2 bit not already set to 0 -> set MEM_ORG to 0 */
            regValue &= ~(1U << SYS_A1SA_MEM_ORG_SHIFT);
        }
    }

    errCode = rfalST25xVPollerWriteConfiguration( RFAL_NFCV_REQ_FLAG_DEFAULT | RFAL_NFCV_REQ_FLAG_ADDRESS, uid, SYS_ADDR_AREA1_SECURITY_ATTRIBUTES, regValue );

    return errCode;
}

/*******************************************************************************/
ReturnCode ST25TAGS_DVPWM_ProtectUserArea(const uint8_t* uid, ST25TAG_DVPWM_UserAreaName_t areaName, ST25TAG_DVPWM_AccessProtection_t accessControl)
{
    ReturnCode  errCode;
    uint8_t     systemAddress;
    uint8_t     regValue;

    switch (areaName)
    {
        case AREA_USER_1:
            systemAddress = SYS_ADDR_AREA1_SECURITY_ATTRIBUTES;
            break;
        case AREA_USER_2:
            systemAddress = SYS_ADDR_AREA2_SECURITY_ATTRIBUTES;
            break;
        case AREA_PWM_CONTROL:
            systemAddress = SYS_ADDR_PWM_CTRL_SECURITY_ATTRIBUTES;
            break;
        default:
            return ERR_PARAM;
    }

    /* Sets content of register A1SA bit MEM_ORG (b2) to the wanted value */
    errCode = rfalST25xVPollerReadConfiguration( RFAL_NFCV_REQ_FLAG_DEFAULT | RFAL_NFCV_REQ_FLAG_ADDRESS, uid, systemAddress, &regValue );
    if ( errCode != ERR_NONE )
    {
        return errCode;
    }

    if ((regValue & SYS_AxSA_RW_PROTECTION_BITMASK) == accessControl)
    {
        /* Register already set to the wanted value, exit without writing*/
        return ERR_NONE;
    }

    /* Replace b0:b1 of the register by the wanted value */
    regValue &= ~SYS_AxSA_RW_PROTECTION_BITMASK;
    regValue |= (uint8_t) accessControl;

    errCode = rfalST25xVPollerWriteConfiguration( RFAL_NFCV_REQ_FLAG_DEFAULT | RFAL_NFCV_REQ_FLAG_ADDRESS, uid, systemAddress, regValue );

    return errCode;
}


/*
 ******************************************************************************
 * PWM CONTROL
 ******************************************************************************
 */

/*******************************************************************************/
ReturnCode ST25TAGS_DVPWM_EnablePwmOutputSignal(const uint8_t* uid, ST25TAGS_DVPWM_PwmNumber_t pwmName, bool enableOutput)
{
    ReturnCode  errCode;
    uint8_t     pwmReadAddress;
    uint16_t    rcvLen;
    uint8_t     rxBuf[ 1 + ST25TAGS_DVPWM_BLOCK_LEN + RFAL_CRC_LEN ];    /* Flags + Block Data + CRC */
    uint8_t     wrData[ST25TAGS_DVPWM_BLOCK_LEN];

    if ( pwmName == DVPWM_OUTPUT_PWM1)
    {
        pwmReadAddress = PWM1_CTRL_AREA_START_ADDR;
    }
    else if ( pwmName == DVPWM_OUTPUT_PWM2)
    {
        pwmReadAddress = PWM2_CTRL_AREA_START_ADDR;
    }
    else
    {
        return ERR_PARAM;
    }

    errCode = rfalNfcvPollerReadSingleBlock(RFAL_NFCV_REQ_FLAG_DEFAULT | RFAL_NFCV_REQ_FLAG_ADDRESS, uid, pwmReadAddress, rxBuf, sizeof(rxBuf), &rcvLen);

    if ( errCode != ERR_NONE )
    {
        return errCode;
    }

    /* Copy read block data into a new block for a write command */
    ST_MEMCPY(wrData, rxBuf + 1, ST25TAGS_DVPWM_BLOCK_LEN);

    if ( enableOutput )
    {
        if ( ((wrData[PWMx_CTRL_PWM_ENABLE_BLOCK] >> PWMx_CTRL_PWM_ENABLE_SHIFT ) & 1U ) == 0x01)
        {
            // Nothing to do
            return ERR_NONE;
        }
        else
        {
            wrData[PWMx_CTRL_PWM_ENABLE_BLOCK] |= (1U << PWMx_CTRL_PWM_ENABLE_SHIFT);
        }
    }
    else
    {
        if ( ((wrData[PWMx_CTRL_PWM_ENABLE_BLOCK] >> PWMx_CTRL_PWM_ENABLE_SHIFT ) & 1U) == 0x00)
        {
            // Nothing to do
            return ERR_NONE;
        }
        else
        {
            wrData[PWMx_CTRL_PWM_ENABLE_BLOCK] &= ~(1U << PWMx_CTRL_PWM_ENABLE_SHIFT);
        }
    }

    errCode = rfalNfcvPollerWriteSingleBlock(RFAL_NFCV_REQ_FLAG_DEFAULT, uid, pwmReadAddress, wrData, sizeof(wrData));

    return errCode;
}

/*******************************************************************************/
ReturnCode ST25TAGS_DVPWM_SetPwmOutputSignal(const uint8_t* uid, ST25TAGS_DVPWM_PwmNumber_t pwmName, uint16_t frequencyInHertz, uint8_t dutyCycle)
{
    ReturnCode  errCode;
    uint16_t    periodInNanoSecondsSteps;
    uint16_t    pulseWidthInNanoSecondsSteps;
    uint8_t     pwmReadAddress;
    uint16_t    rcvLen;
    uint8_t     rxBuf[ 1 + ST25TAGS_DVPWM_BLOCK_LEN + RFAL_CRC_LEN ];    /* Flags + Block Data + CRC */
    uint8_t     wrData[ST25TAGS_DVPWM_BLOCK_LEN];

    /* Check parameters */
    if ( (frequencyInHertz < (ST25TAGS_DVPWM_PWM_FREQUENCY_MIN))
        || (frequencyInHertz > (ST25TAGS_DVPWM_PWM_FREQUENCY_MAX)) )
    {
        return ERR_PARAM;
    }

    if ( dutyCycle > 100 )
    {
        return ERR_PARAM;
    }

    /* Input parameters:
     *     - Frequency = 1 / Period
     *     - Duty cycle = Pulse width / Period
     * Values programmed in register:
     *     - Period = (1 / Frequency * 62.5ns)
     *     - Pulse Width = (Duty Cycle * Period) / 100
     */
    periodInNanoSecondsSteps = (uint16_t) (1000000000 / (frequencyInHertz * 62.5));
    pulseWidthInNanoSecondsSteps = (dutyCycle * periodInNanoSecondsSteps) / 100;

    if ( pwmName == DVPWM_OUTPUT_PWM1)
    {
        pwmReadAddress = PWM1_CTRL_AREA_START_ADDR;
    }
    else if ( pwmName == DVPWM_OUTPUT_PWM2)
    {
        pwmReadAddress = PWM2_CTRL_AREA_START_ADDR;
    }
    else
    {
        return ERR_PARAM;
    }

    errCode = rfalNfcvPollerReadSingleBlock(RFAL_NFCV_REQ_FLAG_DEFAULT | RFAL_NFCV_REQ_FLAG_ADDRESS, uid, pwmReadAddress, rxBuf, sizeof(rxBuf), &rcvLen);

    if ( errCode != ERR_NONE )
    {
        return errCode;
    }

    /* Copy read block data into a new block for a write command */
    ST_MEMCPY(wrData, rxBuf + 1, ST25TAGS_DVPWM_BLOCK_LEN);

    /* Update PWMx_CTRL register with new value */
    /* PWM period is stored on bits b15 (MSB) to b0 */
    wrData[0] = periodInNanoSecondsSteps & 0x00FF;
    wrData[1] &= PWMx_CTRL_RFU_BITMASK;         // Keep b15 unchanged
    wrData[1] |= (periodInNanoSecondsSteps >> 8) & 0xFF;

    /* PWM pulse width is stored on bits b30 (MSB) to b16 */
    wrData[2] = pulseWidthInNanoSecondsSteps & 0x00FF;
    wrData[3] &= PWMx_CTRL_PWM_ENABLE_BITMASK;  // Keep b31 unchanged
    wrData[3] |= (pulseWidthInNanoSecondsSteps >> 8) & 0xFF;

    errCode = rfalNfcvPollerWriteSingleBlock(RFAL_NFCV_REQ_FLAG_DEFAULT, uid, pwmReadAddress, wrData, sizeof(wrData));

    return errCode;
}

/*******************************************************************************/
ReturnCode ST25TAGS_DVPWM_ConfigurePwmOutputPower(const uint8_t* uid, ST25TAGS_DVPWM_PwmNumber_t pwmName, ST25TAGS_DVPWM_OutputPowerLevel_t outputPowerLevel)
{
    ReturnCode  errCode;
    uint8_t     regValue;

    /* Check parameters */
    if ( pwmName != DVPWM_OUTPUT_PWM1 && pwmName != DVPWM_OUTPUT_PWM2)
    {
        return ERR_PARAM;
    }

    if ( outputPowerLevel > QUARTER_POWER )
    {
        return ERR_PARAM;
    }

    errCode = rfalST25xVPollerReadConfiguration( RFAL_NFCV_REQ_FLAG_DEFAULT | RFAL_NFCV_REQ_FLAG_ADDRESS, uid, SYS_ADDR_PWM_CONFIG, &regValue );

    if ( errCode != ERR_NONE )
    {
        return errCode;
    }

    /* Update PWM_CFG register with new value */
    if ( pwmName == DVPWM_OUTPUT_PWM1)
    {
        regValue &= 0xFC;  // Clears b0 and b1
        regValue |= outputPowerLevel & 0x0F;
    }
    else
    {
        regValue &= 0xF3;  // Clears b2 and b3
        regValue |= (outputPowerLevel & 0x0F) << 2;
    }

    errCode = rfalST25xVPollerWriteConfiguration( RFAL_NFCV_REQ_FLAG_DEFAULT | RFAL_NFCV_REQ_FLAG_ADDRESS, uid, SYS_ADDR_PWM_CONFIG, regValue );

    return errCode;
}

/*******************************************************************************/
ReturnCode ST25TAGS_DVPWM_ConfigurePwmOutputDuringRfCommand(const uint8_t* uid, ST25TAGS_DVPWM_PwmCoexistenceWithRf_t pwmOutputBehavior)
{
    ReturnCode  errCode;
    uint8_t     regValue;

    if ( pwmOutputBehavior > DVPWM_PWM_OUTPUT_HIGH_IMPEDANCE_DURING_RF )
    {
        return ERR_PARAM;
    }

    errCode = rfalST25xVPollerReadConfiguration( RFAL_NFCV_REQ_FLAG_DEFAULT | RFAL_NFCV_REQ_FLAG_ADDRESS, uid, SYS_ADDR_PWM_CONFIG, &regValue );

    if ( errCode != ERR_NONE )
    {
        return errCode;
    }

    regValue &= 0x8F;   // Reset b6-b4 bits

    /* Update PWM_CFG register with new value */
    switch ( pwmOutputBehavior )
    {
        case DVPWM_PWM_OUTPUT_UNCHANGED:
            regValue |= 0U << 4;
            break;
        case DVPWM_PWM_OUTPUT_FULL_POWER_LOW_FREQUENCY:
            regValue |= 1U << 4;
            break;
        case DVPWM_PWM_OUTPUT_QUARTER_POWER_FULL_FREQUENCY:
            regValue |= 2U << 4;
            break;
        case DVPWM_PWM_OUTPUT_QUARTER_POWER_LOW_FREQUENCY:
            regValue |= 3U << 4;
            break;
        case DVPWM_PWM_OUTPUT_HIGH_IMPEDANCE_DURING_RF:
            regValue |= 4U << 4;
            break;
        default:
            return ERR_PARAM;
    }

    errCode = rfalST25xVPollerWriteConfiguration( RFAL_NFCV_REQ_FLAG_DEFAULT | RFAL_NFCV_REQ_FLAG_ADDRESS, uid, SYS_ADDR_PWM_CONFIG, regValue );

    return errCode;
}
