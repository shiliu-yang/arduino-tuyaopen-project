        ////////////////////////////////////////////////////
        //       TFT_eSPI generic driver functions        //
        ////////////////////////////////////////////////////
#include "SPI.h"

#include "TFT_eSPI_Tuya_T2.h"
#include "tal_memory.h"

////////////////////////////////////////////////////////////////////////////////////////
// Global variables
////////////////////////////////////////////////////////////////////////////////////////

// Select the SPI port to use
SPIClass& spi = SPI;

#if defined (TFT_SDA_READ) && !defined (TFT_PARALLEL_8_BIT)
// TODO: Not supported
#endif

/***************************************************************************************
** Function name:           read byte  - supports class functions
** Description:             Parallel bus only - dummy function - not used
***************************************************************************************/
uint8_t TFT_eSPI::readByte(void)
{
  uint8_t b = 0xAA;
  return b;
}

#if defined (TFT_PARALLEL_8_BIT)
// TODO: Not supported
#elif defined (SPI_18BIT_DRIVER)
// TODO:
#else  // Standard SPI 16-bit colour TFT
void TFT_eSPI::pushBlock(uint16_t color, uint32_t len){
  #ifdef ENABLE_TUYA_DRAW_BUF

  uint8_t *data = (uint8_t*)tal_malloc(TUYA_DRAW_BUF_SIZE);
  if (data == NULL) {
    while ( len-- ) {tft_Write_16(color);}
    return;
  }
  memset(data, 0, TUYA_DRAW_BUF_SIZE);
  uint32_t remainLen = len * 2;
  while (remainLen > 0) {
    uint32_t l = remainLen;
    if (l > TUYA_DRAW_BUF_SIZE) l = TUYA_DRAW_BUF_SIZE;
    for (uint32_t j = 0; j < l / 2; j++) {
      data[j * 2] = (uint8_t)(color >> 8);
      data[j * 2 + 1] = (uint8_t)(color & 0xFF);
    }
    spi.transfer((uint8_t *)data, l);
    remainLen -= l;
  }

  tal_free(data);

  #else

  while ( len-- ) {tft_Write_16(color);}

  #endif
  return;
}
void TFT_eSPI::pushPixels(const void* data_in, uint32_t len) {

  uint16_t *data = (uint16_t*)data_in;

  if (_swapBytes) {
#ifdef ENABLE_TUYA_DRAW_BUF
    uint8_t *swapData = (uint8_t*)tal_malloc(TUYA_DRAW_BUF_SIZE);
    if (swapData == NULL) {
      while ( len-- ) {tft_Write_16(*data); data++;}
      return;
    }
    memset(swapData, 0, TUYA_DRAW_BUF_SIZE);
    uint32_t remainLen = len * 2;
    while (remainLen > 0) {
      uint32_t l = remainLen;
      if (l > TUYA_DRAW_BUF_SIZE) l = TUYA_DRAW_BUF_SIZE;
      for (uint32_t j = 0; j < l / 2; j++) {
        swapData[j * 2] = (uint8_t)(data[j] >> 8);
        swapData[j * 2 + 1] = (uint8_t)(data[j] & 0xFF);
      }
      spi.transfer((uint8_t *)swapData, l);
      remainLen -= l;
    }
    tal_free(swapData);
#else
    while ( len-- ) {tft_Write_16(*data); data++;}
#endif
  } else {
    spi.transfer((uint8_t *)data_in, len*2);
  }
  return;
}

void tft_Write_8(uint8_t C) {
  spi.transfer((void *)&C, 1);
}

void tft_Write_16(uint16_t C) {
  uint8_t data[2] = {(uint8_t)((C)>>8), (uint8_t)((C)>>0)};
  spi.transfer(data, 2);
}

void tft_Write_16S(uint16_t C) {
  uint8_t data[2] = {(uint8_t)((C)>>0), (uint8_t)((C)>>8)};
  spi.transfer(data, 2);
}

void tft_Write_32(uint32_t C) {
  uint8_t data[4] = {(uint8_t)((C)>>24), (uint8_t)((C)>>16), (uint8_t)((C)>>8), (uint8_t)((C)>>0)};
  spi.transfer(data, 4);
}

void tft_Write_32C(uint16_t C, uint16_t D) {
  uint8_t data[4] = {(uint8_t)((C)>>8), (uint8_t)((C)>>0), (uint8_t)((D)>>8), (uint8_t)((D)>>0)};
  spi.transfer(data, 4);
}

void tft_Write_32D(uint16_t C) {
  uint8_t data[4] = {(uint8_t)((C)>>8), (uint8_t)((C)>>0), (uint8_t)((C)>>8), (uint8_t)((C)>>0)};
  spi.transfer(data, 4);
}

#endif
