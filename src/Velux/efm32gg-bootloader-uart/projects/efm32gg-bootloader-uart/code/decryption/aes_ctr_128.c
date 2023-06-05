/***************************************************************************//**
 * @file aes_ctr_128.c
 * @brief AES CTR 128-bit interrupt driven functions for EFM32
 * @author Silicon Labs
 * @version 1.11
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2014 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
 * obligation to support this Software. Silicon Labs is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Silicon Labs will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 ******************************************************************************/

#include <stdbool.h>
#include "em_device.h"
#include "em_aes.h"
#include "aes_ctr_128.h"

static uint16_t               numberOfBlocks;
static uint16_t               blockIndex;
static uint32_t*              outputDataG;
static const uint32_t*        inputDataG;
static uint32_t*              ctrG;
static AES_CtrFuncPtr_TypeDef ctrFuncG;
static volatile bool          aesFinished;


/**************************************************************************//**
 * @brief  AES IRQ Handler
 *****************************************************************************/
void AES_IRQHandler(void)
{
  int i;
  uint8_t* ctr8 = (uint8_t*) ctrG;

  /* Clear interrupt flag */
  AES->IFC = AES_IFC_DONE;


  if (blockIndex < numberOfBlocks)
  {    
    /* Update counter value every time a new block en/decryption starts.   */ 
    /* Only update if there are more blocks left, because a new call to    */
    /* AesCtr128() to decrypt the next data block from XMODEM input, will  */
    /* otherwise update the counter one too many times.                    */
    ctrFuncG(ctr8);
  
    /* Store en/decrypted data and increment to the next start value of the input */    
    for (i = 3; i >= 0; i--)
    {
      outputDataG[i] = __REV(AES->DATA) ^ inputDataG[i];
    }
    outputDataG += 4;
    inputDataG  += 4;

    /* Load data and trigger encryption using DATA*/   
    for (i = 3; i >= 0; i--)
    {
      AES->DATA = __REV(ctrG[i]);
    }
  }

  /* Indicate last block */
  else
  {
    aesFinished = true;
  }

  /* Increase block index */
  blockIndex++;
}


/**************************************************************************//**
 * @brief  Encrypt/decrypt with 128-bit key in CTR mode. Function returns after
 * initializing encryption/decryption. Use AesFinished() to check for 
 * completion. Output data only valid after completion. 
 *
 * @param[in] key
 *   This is the 128-bit encryption key regardless of encryption or decryption 
 *
 * @param[in] inputData
 *   Buffer holding data to encrypt/decrypt.
 *
 * @param[out] outputData
 *   Buffer to put output data, must be of size blockNumber*16 bytes. This can
 *   be the same location as inputData.
 *
 * @param[in] blockNumber
 *   Number of 128-bit blocks to decrypt/encrypt.
 *
 * @param[inout] ctr
 *   128-bit counter value to use
 *
 * @param[in] initDecryption
 *   Init decryption if true
 *****************************************************************************/
__ramfunc void AesCtr128(const uint8_t*         key,
                         const uint8_t*         inputData,
                         uint8_t*               outputData,
                         const uint32_t         blockNumber,
                         uint8_t*               ctr, 
                         AES_CtrFuncPtr_TypeDef ctrFunc,
                         bool initDecryption                  )
{
  int i;
  uint32_t*  key32 = (uint32_t*) key;
  
  /* Copy to global variables */
  inputDataG     = (uint32_t*) inputData;
  outputDataG    = (uint32_t*) outputData;
  
  if(initDecryption)
  {
    /* This function will be called for each data package received via XMODEM */
    /* so this setup must only be done on the first call.                    */

    ctrG           = (uint32_t*) ctr;
    ctrFuncG       = ctrFunc;
    
    /* Clear and enable AES interrupt */
    AES->IFC = AES_IFC_DONE;
    AES->IEN = AES_IEN_DONE;
    NVIC->ISER[(((uint32_t)(int32_t)AES_IRQn) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)(int32_t)AES_IRQn) & 0x1FUL));
    
    /* Configure AES module */
    AES->CTRL = AES_CTRL_KEYBUFEN |  /* Set key buffer */
    AES_CTRL_DATASTART;  /* Start en/decryption on data write */
    
    /* Write KEY to AES module */ 
    for (i = 3; i >= 0; i--)
    {
      AES->KEYHA = __REV(key32[i]);
    }
  }
  
  
  numberOfBlocks = blockNumber;

  /* Reset block index */
  blockIndex = 0;

  /* Initialize finished flag */
  aesFinished = false;
  
  /* Load data and trigger encryption using DATA*/ 
  for (i = 3; i >= 0; i--)
  {
    AES->DATA = __REV(ctrG[i]);
  }
}

/**************************************************************************//**
 * @brief  Function to check if AES process has finished
 * @return true if AES operation has finished
 *****************************************************************************/
__ramfunc bool AesFinished(void)
{
  return aesFinished;
}



