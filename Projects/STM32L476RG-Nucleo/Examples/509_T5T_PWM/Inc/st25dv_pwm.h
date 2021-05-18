/**
  ******************************************************************************
  * @file    st25dv-pwm.h
  * @author  MMY Ecosystem Team
  * @brief   ST25DV-PWM header file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2020 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef ST25DV_PWM_H
#define ST25DV_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "st_errno.h"


/* Exported types ------------------------------------------------------------*/
/** @addtogroup ST25Tags
 *  @{
 */

typedef enum {
    UNKNOWN_DVPWM_TAG = 0,
    ST25DV512_W1,
    ST25DV512_W2,
    ST25DV01K_W1,
    ST25DV01K_W2,
    ST25DV02K_W1,
    ST25DV02K_W2
} ST25TAGS_DVPWM_TagType_t;

typedef enum {
    DVPWM_OUTPUT_PWM1,
    DVPWM_OUTPUT_PWM2
} ST25TAGS_DVPWM_PwmNumber_t;

typedef enum {
    CONFIGURATION_SECURITY_SESSION,
    USER_AREA1_SECURITY_SESSION,
    USER_AREA2_SECURITY_SESSION,
    SINGLE_USER_AREA_SECURITY_SESSION,  /*!< AREA1 + AREA2 are merged in a single user area */
    PWM_CTRL_SECURITY_SESSION
} ST25TAG_DVPWM_SecuritySession_t;

typedef enum {
    AREA_USER_1,
    AREA_USER_2,
    AREA_PWM_CONTROL
} ST25TAG_DVPWM_UserAreaName_t;

typedef enum {
    READABLE_AND_WRITEABLE = 0U,
    READABLE_AND_WRITE_PROTECTED_BY_PWD = 1U,
    READ_AND_WRITE_PROTECTED_BY_PWD = 2U,
    READ_PROTECTED_BY_PWD_AND_WRITE_FORBIDDEN = 3U
} ST25TAG_DVPWM_AccessProtection_t;

typedef enum {
    FULL_POWER,
    THREE_QUARTER_POWER,
    HALF_POWER,
    QUARTER_POWER
} ST25TAGS_DVPWM_OutputPowerLevel_t;

typedef enum {
    DVPWM_PWM_OUTPUT_UNCHANGED,
    DVPWM_PWM_OUTPUT_QUARTER_POWER_FULL_FREQUENCY,
    DVPWM_PWM_OUTPUT_FULL_POWER_LOW_FREQUENCY,
    DVPWM_PWM_OUTPUT_QUARTER_POWER_LOW_FREQUENCY,
    DVPWM_PWM_OUTPUT_HIGH_IMPEDANCE_DURING_RF
} ST25TAGS_DVPWM_PwmCoexistenceWithRf_t;


/* Exported constants --------------------------------------------------------*/
#define ST25TAGS_ST_MFR_CODE                        0x02U

#define ST25TAGS_DVPWM_SYSINFO_USERBLOCKS_INDEX     12U
#define ST25TAGS_DVPWM_SYSINFO_BLOCKSIZE_INDEX      13U
#define ST25TAGS_DVPWM_SYSINFO_ICREF_INDEX          14U

#define ST25TAGS_DVPWM_PWM_AND_RF_FULL_COEXISTENCE  0x0U
#define ST25TAGS_DVPWM_PWM_IN_HIGH_IMPEDANCE        0x04U
#define ST25TAGS_DVPWM_PWM_ONE_QUARTER_FULL_POWER   0x02U
#define ST25TAGS_DVPWM_PWM_FREQ_REDUCED             0x01U

#define ST25TAGS_DVPWM_W1_PRODUCT_CODE              0x38U
#define ST25TAGS_DVPWM_W2_PRODUCT_CODE              0x39U
#define ST25TAGS_DVPWM_SYSINFO_MAX_LEN              17U

#define ST25TAGS_DVPWM_PWM_FREQUENCY_MIN            489
#define ST25TAGS_DVPWM_PWM_FREQUENCY_MAX            31250

#define ST25TAGS_DVPWM_BLOCK_LEN                    4U


/* Exported macro ------------------------------------------------------------*/
#define ST25TAGS_DVPWM_HAS_TWO_OUTPUTS(x) ((x) == ST25DV512_W2 || (x) == ST25DV01K_W2 || (x) == ST25DV02K_W2)

/* Exported variables --------------------------------------------------------*/
extern char ST25TAGS_ST25DVPWM_tagName[][16];

/* Exported functions --------------------------------------------------------*/

/**
  *****************************************************************************
  * \brief Determine if the UID parameter belongs to a ST25DV-PWM product
  *
  * \param[in]  uid : pointer to UID byte array. The expected length is 8 bytes
  *                              and the last byte of the array must be equal to 0xE0.
  *
  * \return true  if the UID belongs to a ST25DV-PWM product
  * \return false if the UID does not belong to a ST25DV-PWM product
  *****************************************************************************
  */
bool ST25TAGS_DVPWM_isTagSt25DvPwm(const uint8_t* uid);

/**
  *****************************************************************************
  * \brief Get Tag Type
  *
  * Determines if the tag with the UID as parameter belongs to a ST25DV-PWM product.
  *
  * \param[in]  uid : pointer to UID byte array. The expected length is 8 bytes
  *                              and the last byte of the array must be equal to 0xE0.
  *
  * \return ST25TAGS_DVPWM_TagType_t value corresponding to the tag's reference
  *****************************************************************************
  */
ST25TAGS_DVPWM_TagType_t ST25TAGS_DVPWM_GetSt25PwmTagType(const uint8_t* uid);

/**
  *****************************************************************************
  * \brief Open a security session
  *
  * Sends presentPassword RF command through rfalST25xVPollerPresentPassword() in
  * addressed mode using the uid provided as the first parameter.
  *
  * The security session will be closed upon taking the tag away from the RF field, or
  * when presenting an incorrect password.
  *
  * When AREA1 and AREA2 user areas are merged, using the command
  * ST25TAGS_DVPWM_SetNumberOfUserAreas(uid, 1), the password for the session must be set
  * to 64-bit length.
  *
  * \param[in]  uid            : pointer to UID byte array. The expected length is 8 bytes
  *                              and the last byte of the array must be equal to 0xE0.
  * \param[in]  sessionType    : Value taken from ST25TAG_DVPWM_SecuritySession_t
  * \param[in]  pPassword      : pointer to a 4-byte or 8-byte char array
  *                              pPassword length used is 8 bytes for SINGLE_USER_AREA_SECURITY_SESSION
  *                              pPassword length used is 4 bytes for all other sessions
  *
  * \return ERR_WRONG_STATE    : RFAL not initialized or incorrect mode
  * \return ERR_PARAM          : Invalid parameters
  * \return ERR_IO             : Generic internal error
  * \return ERR_CRC            : CRC error detected
  * \return ERR_FRAMING        : Framing error detected
  * \return ERR_PROTO          : Protocol error detected
  * \return ERR_TIMEOUT        : Timeout error
  * \return ERR_NONE           : No error
  *****************************************************************************
  */
ReturnCode ST25TAGS_DVPWM_OpenSecuritySession(const uint8_t* uid, ST25TAG_DVPWM_SecuritySession_t securitySession, const uint8_t * pConfigPassword);

/**
  *****************************************************************************
  * \brief Set the number of user areas
  *
  * Configures the number of user areas. Possible values are 1 and 2 (AREA0 and PWM are not counted).
  *
  * NOTE: A security session must be active for this command to execute correctly.
  *       If ST25TAGS_DVPWM_OpenSecuritySession() has not been called prior to this
  *       command, the method may return ERR_REQUEST (unless no write is required).
  *
  * \param[in] uid              : pointer to UID byte array. The expected length is 8 bytes
  *                               and the last byte of the array must be equal to 0xE0.
  * \param[in] numberOfUserAreas: set to 1 to merge Area1 and Area2 into a single user area configuration
  *                               set to 2 to keep 2 distinct user areas
  *
  * \return ERR_WRONG_STATE    : RFAL not initialized or incorrect mode
  * \return ERR_PARAM          : Invalid parameters
  * \return ERR_IO             : Generic internal error
  * \return ERR_CRC            : CRC error detected
  * \return ERR_FRAMING        : Framing error detected
  * \return ERR_PROTO          : Protocol error detected
  * \return ERR_TIMEOUT        : Timeout error
  * \return ERR_REQUEST        : Cannot be executed at the moment
  * \return ERR_NONE           : No error
  *****************************************************************************
  */
ReturnCode ST25TAGS_DVPWM_SetNumberOfUserAreas(const uint8_t* uid, uint8_t numberOfUserAreas);

/**
  *****************************************************************************
  * \brief Set area access protection
  *
  * Sets Read/Write protection for a given area.
  *
  * NOTE: A PWM_CTRL_SECURITY_SESSION security session must be active for this command
  *       to execute correctly.
  *       If ST25TAGS_DVPWM_OpenSecuritySession() has not been called prior to this
  *       command, the method may return ERR_REQUEST (unless no write is required).
  *
  * \param[in]  uid            : pointer to UID byte array. The expected length is 8 bytes
  *                              and the last byte of the array must be equal to 0xE0.
  * \param[in]  areaName       : Area to protect
  * \param[in]  accessControl  : Level of protection to apply to the given area
  *
  * \return ERR_WRONG_STATE    : RFAL not initialized or incorrect mode
  * \return ERR_PARAM          : Invalid parameters
  * \return ERR_IO             : Generic internal error
  * \return ERR_CRC            : CRC error detected
  * \return ERR_FRAMING        : Framing error detected
  * \return ERR_PROTO          : Protocol error detected
  * \return ERR_TIMEOUT        : Timeout error
  * \return ERR_REQUEST        : Cannot be executed at the moment
  * \return ERR_NONE           : No error
  *****************************************************************************
  */
ReturnCode ST25TAGS_DVPWM_ProtectUserArea(const uint8_t* uid, ST25TAG_DVPWM_UserAreaName_t areaName, ST25TAG_DVPWM_AccessProtection_t accessControl);

/**
  *****************************************************************************
  * \brief Enable/Disable PWM output signal
  *
  * Enables or disables PWM1 or PWM2 output.
  *
  * NOTE: A PWM_CTRL_SECURITY_SESSION security session must be active for this command
  *       to execute correctly.
  *       If ST25TAGS_DVPWM_OpenSecuritySession() has not been called prior to this
  *       command, the method may return ERR_REQUEST (unless no write is required).
  *
  * \param[in]  uid            : pointer to UID byte array. The expected length is 8 bytes
  *                              and the last byte of the array must be equal to 0xE0.
  * \param[in]  pwmName        : Select PWM1 or PWM2 output.
  * \param[in]  enableOutput   : Set to true to enable the given PWM output.
  *                            : Set to false to disable the given PWM output.
  *
  * \return ERR_WRONG_STATE    : RFAL not initialized or incorrect mode
  * \return ERR_PARAM          : Invalid parameters
  * \return ERR_IO             : Generic internal error
  * \return ERR_CRC            : CRC error detected
  * \return ERR_FRAMING        : Framing error detected
  * \return ERR_PROTO          : Protocol error detected
  * \return ERR_TIMEOUT        : Timeout error
  * \return ERR_REQUEST        : Cannot be executed at the moment
  * \return ERR_NONE           : No error
  *****************************************************************************
  */
ReturnCode ST25TAGS_DVPWM_EnablePwmOutputSignal(const uint8_t* uid, ST25TAGS_DVPWM_PwmNumber_t pwmName, bool enableOutput);

/**
  *****************************************************************************
  * \brief Set PWM output shape
  *
  * Programs a given PWM signal with the wanted characteristics.
  *
  * NOTE: A PWM_CTRL_SECURITY_SESSION security session must be active for this command
  *       to execute correctly.
  *       If ST25TAGS_DVPWM_OpenSecuritySession() has not been called prior to this
  *       command, the method will return ERR_REQUEST.
  *
  * \param[in] uid             : pointer to UID byte array. The expected length is 8 bytes
  *                              and the last byte of the array must be equal to 0xE0.
  * \param[in] pwmName         : PWM signal selection [PWM1, PWM2]
  * \param[in] frequencyInHertz: PWM output frequency in range [489, 31250]
  * \param[in] dutyCycle       : PWM signal's duty cycle in range [0, 100]
  *
  * \return ERR_WRONG_STATE    : RFAL not initialized or incorrect mode
  * \return ERR_PARAM          : Invalid parameters
  * \return ERR_IO             : Generic internal error
  * \return ERR_CRC            : CRC error detected
  * \return ERR_FRAMING        : Framing error detected
  * \return ERR_PROTO          : Protocol error detected
  * \return ERR_TIMEOUT        : Timeout error
  * \return ERR_REQUEST        : Cannot be executed at the moment
  * \return ERR_NONE           : No error
  *****************************************************************************
  */
ReturnCode ST25TAGS_DVPWM_SetPwmOutputSignal(const uint8_t* uid, ST25TAGS_DVPWM_PwmNumber_t pwmName, uint16_t frequencyInHertz, uint8_t dutyCycle);

/**
  *****************************************************************************
  * \brief  Configure PWM output driver level
  *
  * If the application does not require full power it is possible to reduce the
  * output drive capability independently through PWM_CFG trimming registers
  * (PWM_CFG bits b1-b0 for PWM1 and PWM_CFG bits b3-b2 for PWM2).
  *
  * NOTE: A PWM_CTRL_SECURITY_SESSION security session must be active for this command
  *       to execute correctly.
  *       If ST25TAGS_DVPWM_OpenSecuritySession() has not been called prior to this
  *       command, the method may return ERR_REQUEST (unless no write is required).
  *
  * \param[in] uid             : pointer to UID byte array. The expected length is 8 bytes
  *                              and the last byte of the array must be equal to 0xE0.
  * \param[in] pwmName         : PWM signal selection [PWM1, PWM2]
  * \param[in] outputPowerLevel: FULL (default), 75%, 50% or 25% of Imax level
  *
  * \return ERR_WRONG_STATE    : RFAL not initialized or incorrect mode
  * \return ERR_PARAM          : Invalid parameters
  * \return ERR_IO             : Generic internal error
  * \return ERR_CRC            : CRC error detected
  * \return ERR_FRAMING        : Framing error detected
  * \return ERR_PROTO          : Protocol error detected
  * \return ERR_TIMEOUT        : Timeout error
  * \return ERR_REQUEST        : Cannot be executed at the moment
  * \return ERR_NONE           : No error
  *****************************************************************************
  */
ReturnCode ST25TAGS_DVPWM_ConfigurePwmOutputPower(const uint8_t* uid, ST25TAGS_DVPWM_PwmNumber_t pwmName, ST25TAGS_DVPWM_OutputPowerLevel_t outputPowerLevel);

/**
  *****************************************************************************
  * \brief  Configure PWM output behavior during RF command
  *
  * Reduces the impact of PWM noise during RF commands
  *
  * NOTE: A PWM_CTRL_SECURITY_SESSION security session must be active for this command
  *       to execute correctly.
  *       If ST25TAGS_DVPWM_OpenSecuritySession() has not been called prior to this
  *       command, the method may return ERR_REQUEST (unless no write is required).
  *
  * \param[in] uid             : pointer to UID byte array. The expected length is 8 bytes
  *                              and the last byte of the array must be equal to 0xE0.
  * \param[in] pwmBehavior     : Can take one of 5 values
  *                              - PWM_OUTPUT_UNCHANGED                    : PWM output unchanged during rf command
  *                              - PWM_OUTPUT_QUARTER_POWER_FULL_FREQUENCY : PWM output drive level trimmed at 25%
  *                              - PWM_OUTPUT_FULL_POWER_LOW_FREQUENCY     : PWM output frequency lowered below minimum
  *                              - PWM_OUTPUT_QUARTER_POWER_LOW_FREQUENCY  : PWM output drive level and frequency reduced
  *                              - PWM_OUTPUT_HIGH_IMPEDANCE_DURING_RF     : PWM output in high impedance during rf
  *
  * \return ERR_WRONG_STATE    : RFAL not initialized or incorrect mode
  * \return ERR_PARAM          : Invalid parameters
  * \return ERR_IO             : Generic internal error
  * \return ERR_CRC            : CRC error detected
  * \return ERR_FRAMING        : Framing error detected
  * \return ERR_PROTO          : Protocol error detected
  * \return ERR_TIMEOUT        : Timeout error
  * \return ERR_REQUEST        : Cannot be executed at the moment
  * \return ERR_NONE           : No error
  *****************************************************************************
  */
ReturnCode ST25TAGS_DVPWM_ConfigurePwmOutputDuringRfCommand(const uint8_t* uid, ST25TAGS_DVPWM_PwmCoexistenceWithRf_t pwmOutputBehavior);

#ifdef __cplusplus
}
#endif

#endif /* ST25DV_PWM_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
