/******************************************************************************
  * \attention
  *
  * <h2><center>&copy; COPYRIGHT 2019 STMicroelectronics</center></h2>
  *
  * Licensed under ST MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        www.st.com/myliberty
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
/*! \file
 *
 *  \author 
 *
 *  \brief Demo application
 *
 *  This demo shows how to poll for several types of NFC cards/devices and how 
 *  to exchange data with these devices, using the RFAL library.
 *
 *  This demo does not fully implement the activities according to the standards,
 *  it performs the required to communicate with a card/device and retrieve 
 *  its UID. Also blocking methods are used for data exchange which may lead to
 *  long periods of blocking CPU/MCU.
 *  For standard compliant example please refer to the Examples provided
 *  with the RFAL library.
 * 
 */
 
/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include "demo.h"
#include "utils.h"
#include "rfal_nfc.h"
#include "ndef_poller.h"
#include "ndef_t5t.h"
#include "ndef_message.h"
#include "ndef_types_rtd.h"
#include "ndef_dump.h"


/*
******************************************************************************
* GLOBAL DEFINES
******************************************************************************
*/

/* Definition of possible states the demo state machine could have */
#define DEMO_ST_NOTINIT               0  /*!< Demo State:  Not initialized */
#define DEMO_ST_START_DISCOVERY       1  /*!< Demo State:  Start Discovery */
#define DEMO_ST_DISCOVERY             2  /*!< Demo State:  Discovery       */

#define NDEF_DEMO_READ              0U   /*!< NDEF menu read               */
#define NDEF_DEMO_WRITE_MSG1        1U   /*!< NDEF menu write 1 record     */
#define NDEF_DEMO_WRITE_MSG2        2U   /*!< NDEF menu write 2 records    */
#define NDEF_DEMO_FORMAT_TAG        3U   /*!< NDEF menu format tag         */
#if NDEF_FEATURE_ALL
#define NDEF_DEMO_MAX_FEATURES      4U   /*!< Number of menu items         */
#else
#define NDEF_DEMO_MAX_FEATURES      1U   /*!< Number of menu items         */
#endif /* NDEF_FEATURE_ALL */
#define NDEF_WRITE_FORMAT_TIMEOUT   10000U /*!< When write or format mode is selected, demo returns back to read mode after a timeout */
#define NDEF_LED_BLINK_DURATION       250U /*!< Led blink duration         */ 

#define DEMO_RAW_MESSAGE_BUF_LEN      8192 /*!< Raw message buffer len     */

#define DEMO_ST_MANUFACTURER_ID      0x02U /*!< ST Manufacturer ID         */

/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */

static uint8_t ndefAndroidPackName[] = "com.st.st25nfc";

/*
 ******************************************************************************
 * LOCAL VARIABLES
 ******************************************************************************
 */

static rfalNfcDiscoverParam discParam;
static uint8_t              state = DEMO_ST_NOTINIT;

static ndefContext          ndefCtx;
static bool                 verbose             = false;

static uint8_t              rawMessageBuf[DEMO_RAW_MESSAGE_BUF_LEN];


/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/

static void demoNdef(rfalNfcDevice *nfcDevice);
static void ndefT5TCCDump(ndefContext *ctx);
ReturnCode  demoTransceiveBlocking( uint8_t *txBuf, uint16_t txBufSize, uint8_t **rxBuf, uint16_t **rcvLen, uint32_t fwt );
//static void ledsOn(void);
static void ledsOff(void);


/*!
 *****************************************************************************
 * \brief Demo Ini
 *
 *  This function Initializes the required layers for the demo
 *
 * \return true  : Initialization ok
 * \return false : Initialization failed
 *****************************************************************************
 */
bool demoIni( void )
{
    ReturnCode err;
          
    err = rfalNfcInitialize();
    if( err == ERR_NONE )
    {
        discParam.compMode      = RFAL_COMPLIANCE_MODE_NFC;
        discParam.devLimit      = 1U;
      //  discParam.nfcfBR        = RFAL_BR_212;
     //   discParam.ap2pBR        = RFAL_BR_424;

    //    ST_MEMCPY( &discParam.nfcid3, NFCID3, sizeof(NFCID3) );
    //    ST_MEMCPY( &discParam.GB, GB, sizeof(GB) );
    //    discParam.GBLen         = sizeof(GB);

        discParam.notifyCb             = NULL;
        discParam.totalDuration        = 1000U;
        discParam.wakeupEnabled        = false;
        discParam.wakeupConfigDefault  = true;
       // discParam.techs2Find           = ( RFAL_NFC_POLL_TECH_A | RFAL_NFC_POLL_TECH_B | RFAL_NFC_POLL_TECH_F | RFAL_NFC_POLL_TECH_V | RFAL_NFC_POLL_TECH_ST25TB );
				discParam.techs2Find           = ( RFAL_NFC_POLL_TECH_V );//CL
			
        state = DEMO_ST_START_DISCOVERY;
        return true;
    }
    return false;
}

/*!
 *****************************************************************************
 * \brief Demo Cycle
 *
 *  This function executes the demo state machine. 
 *  It must be called periodically
 *****************************************************************************
 */
void demoCycle( void )
{
    static rfalNfcDevice *nfcDevice;

    rfalNfcvInventoryRes  invRes;
    uint16_t              rcvdLen;
    
    rfalNfcWorker();                                    /* Run RFAL worker periodically */
        
    switch( state )
    {
        /*******************************************************************************/
        case DEMO_ST_START_DISCOVERY:
            ledsOff();
    
            rfalNfcDeactivate( false );
            rfalNfcDiscover( &discParam );

            state = DEMO_ST_DISCOVERY;
            break;

        /*******************************************************************************/
        case DEMO_ST_DISCOVERY:
            if( rfalNfcIsDevActivated( rfalNfcGetState() ) )
            {
                rfalNfcGetActiveDevice( &nfcDevice );
                
                ledsOff();
                platformDelay(50);
                switch( nfcDevice->type )
                {
                 /*******************************************************************************/
                    case RFAL_NFC_LISTEN_TYPE_NFCV:
                        {
                            uint8_t devUID[RFAL_NFCV_UID_LEN];
                            
                            ST_MEMCPY( devUID, nfcDevice->nfcid, nfcDevice->nfcidLen );   /* Copy the UID into local var */
                            REVERSE_BYTES( devUID, RFAL_NFCV_UID_LEN );                 /* Reverse the UID for display purposes */
                            platformLog("ISO15693/NFC-V card found. UID: %s\r\n", hex2Str(devUID, RFAL_NFCV_UID_LEN));
                        
                            platformLedOn(PLATFORM_LED_V_PORT, PLATFORM_LED_V_PIN);
                            
                            demoNdef(nfcDevice);

                            /* Loop until tag is removed from the field */
                            platformLog("Operation completed\r\nTag can be removed from the field\r\n");
                            rfalNfcvPollerInitialize();
                            while (rfalNfcvPollerInventory( RFAL_NFCV_NUM_SLOTS_1, RFAL_NFCV_UID_LEN * 8U, nfcDevice->dev.nfcv.InvRes.UID, &invRes, &rcvdLen) == ERR_NONE)
                            {
                                platformDelay(130);
                            }
                        }
                        break;
                        
                    
                    /*******************************************************************************/
                    default:
                        break;
                }
                
                rfalNfcDeactivate( false );
                platformDelay( 500 );
                state = DEMO_ST_START_DISCOVERY;
            }
            break;

        /*******************************************************************************/
        case DEMO_ST_NOTINIT:
        default:
            break;
    }
}



/*!
 *****************************************************************************
 * \brief Demo Blocking Transceive 
 *
 * Helper function to send data in a blocking manner via the rfalNfc module 
 *  
 * \warning A protocol transceive handles long timeouts (several seconds), 
 * transmission errors and retransmissions which may lead to a long period of 
 * time where the MCU/CPU is blocked in this method.
 * This is a demo implementation, for a non-blocking usage example please 
 * refer to the Examples available with RFAL
 *
 * \param[in]  txBuf      : data to be transmitted
 * \param[in]  txBufSize  : size of the data to be transmited
 * \param[out] rxData     : location where the received data has been placed
 * \param[out] rcvLen     : number of data bytes received
 * \param[in]  fwt        : FWT to be used (only for RF frame interface, 
 *                                          otherwise use RFAL_FWT_NONE)
 *
 * 
 *  \return ERR_PARAM     : Invalid parameters
 *  \return ERR_TIMEOUT   : Timeout error
 *  \return ERR_FRAMING   : Framing error detected
 *  \return ERR_PROTO     : Protocol error detected
 *  \return ERR_NONE      : No error, activation successful
 * 
 *****************************************************************************
 */
ReturnCode demoTransceiveBlocking( uint8_t *txBuf, uint16_t txBufSize, uint8_t **rxData, uint16_t **rcvLen, uint32_t fwt )
{
    ReturnCode err;
    
    err = rfalNfcDataExchangeStart( txBuf, txBufSize, rxData, rcvLen, fwt );
    if( err == ERR_NONE )
    {
        do{
            rfalNfcWorker();
            err = rfalNfcDataExchangeGetStatus();
        }
        while( err == ERR_BUSY );
    }
    return err;
}

static void demoNdef(rfalNfcDevice *pNfcDevice)
{
    ReturnCode       err;
    ndefMessage      message;
    ndefInfo         info;
    ndefBuffer       bufRawMessage;
	  ndefRecord       record1;
    ndefType         aar;

    ndefConstBuffer bufAndroidPackName;
    /*
     * Perform NDEF Context Initialization
     */
    err = ndefPollerContextInitialization(&ndefCtx, pNfcDevice);
    if( err != ERR_NONE )
    {
        platformLog("NDEF NOT DETECTED (ndefPollerContextInitialization returns %d)\r\n", err);
        return;
    }    
		
    /*
     * Perform NDEF Detect procedure
     */
    err = ndefPollerNdefDetect(&ndefCtx, &info);
		
    if( err != ERR_NONE )
    {
        platformLog("NDEF NOT DETECTED (ndefPollerNdefDetect returns %d)\r\n", err);
				platformLog("Formatting & Initializing T5T CC file...\r\n");
        /* Format Tag */
        err = ndefPollerTagFormat(&ndefCtx, NULL, 0);
				/* update ndefCtx */
				err = ndefPollerNdefDetect(&ndefCtx, &info);
        if( err != ERR_NONE )
        {
            platformLog("Tag cannot be formatted (ndefPollerTagFormat returns %d)\r\n", err);
            return;
        }
    }
    ndefT5TCCDump(&ndefCtx);
/////////////////////////////////////////AAR////////////////////////////////////////////////////////////////////////
    err  = ndefMessageInit(&message);  /* Initialize message structure */

    bufAndroidPackName.buffer = ndefAndroidPackName;
    bufAndroidPackName.length = sizeof(ndefAndroidPackName) - 1U;
    err |= ndefRtdAar(&aar, &bufAndroidPackName); /* Initialize AAR type structure */
    err |= ndefRtdAarToRecord(&aar, &record1); /* Encode AAR record */

    err |= ndefMessageAppend(&message, &record1); /* Append AAR to message */

    bufRawMessage.buffer = rawMessageBuf;
    bufRawMessage.length = sizeof(rawMessageBuf);
    err |= ndefMessageEncode(&message, &bufRawMessage); /* Encode the message to the raw buffer */
    if( err != ERR_NONE )
    {
        platformLog("Raw message creation failed\r\n", err);
        return;
    }
    err = ndefPollerWriteRawMessage(&ndefCtx, bufRawMessage.buffer, bufRawMessage.length);
    if( err != ERR_NONE )
    {
        platformLog("Message cannot be written (ndefPollerWriteRawMessage return %d)\r\n", err);
        return;
    }
						
		platformLog("Wrote AAR record to the Tag\r\n");
		return; 
}


static void ndefT5TCCDump(ndefContext *ctx)
{
    ndefConstBuffer bufCcBuf;
    
    platformLog(" * Block Length: %d\r\n", ctx->subCtx.t5t.blockLen);
    platformLog(" * %d bytes CC\r\n * Magic: %2.2Xh Version: %d.%d MLEN: %d (%d bytes) \r\n * readAccess: %2.2xh writeAccess: %2.2xh \r\n", ctx->cc.t5t.ccLen, ctx->cc.t5t.magicNumber, ctx->cc.t5t.majorVersion, ctx->cc.t5t.minorVersion, ctx->cc.t5t.memoryLen, ctx->cc.t5t.memoryLen * 8U, ctx->cc.t5t.readAccess, ctx->cc.t5t.writeAccess);
    platformLog(" * [%c] Special Frame\r\n",       ctx->cc.t5t.specialFrame ?      'X' : ' ');
    platformLog(" * [%c] Multiple block Read\r\n", ctx->cc.t5t.multipleBlockRead ? 'X' : ' ');
    platformLog(" * [%c] Lock Block\r\n",          ctx->cc.t5t.lockBlock ?         'X' : ' ');
    bufCcBuf.buffer = ctx->ccBuf;
    bufCcBuf.length = ctx->cc.t5t.ccLen;
    ndefBufferDump(" CC Raw Data", &bufCcBuf, verbose);
}

static void ledsOff(void)
{
    platformLedOff(PLATFORM_LED_A_PORT, PLATFORM_LED_A_PIN);
    platformLedOff(PLATFORM_LED_B_PORT, PLATFORM_LED_B_PIN);
    platformLedOff(PLATFORM_LED_F_PORT, PLATFORM_LED_F_PIN);
    platformLedOff(PLATFORM_LED_V_PORT, PLATFORM_LED_V_PIN);
    platformLedOff(PLATFORM_LED_AP2P_PORT, PLATFORM_LED_AP2P_PIN);
    platformLedOff(PLATFORM_LED_FIELD_PORT, PLATFORM_LED_FIELD_PIN);
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
