#include "appStore.h"
#include "appDisplay.h"
#include "Ticker.h"
#include "Log.h"
#include "tal_memory.h"
#include "tal_kv.h"

Ticker storeTicker;

#define APP_STORE_DELAY (5*1000)

#define KEY_EASY_MATRIX "easy_matrix"

static uint8_t *p_data = NULL;
static size_t data_len = 0;

void storeEasyMatrixTicker(void)
{
  if (p_data == NULL || data_len == 0) {
    PR_ERR("storeEasyMatrixTicker p_data == NULL || data_len == 0");
    return;
  }

  PR_DEBUG("storeEasyMatrixTicker");
  tal_kv_set(KEY_EASY_MATRIX, (uint8_t *)p_data, data_len);


  tal_kv_free(p_data);
  p_data = NULL;
}

void storeEasyMatrix(void *data, size_t len)
{
  if (p_data != NULL) {
    tal_kv_free(p_data);
    p_data = NULL;
  }

  if (data == NULL || len == 0) {
    return;
  }

  data_len = len;
  p_data = (uint8_t *)tal_malloc(data_len);
  if (p_data == NULL) {
    PR_DEBUG("storeEasyMatrix tal_malloc failed");
    return;
  }
  memcpy(p_data, data, data_len);

  storeTicker.once_ms(APP_STORE_DELAY, storeEasyMatrixTicker);
}

void storeEasyMatrixGet(void **data, size_t *len)
{
  PR_DEBUG("storeEasyMatrixGet");
  int rt = tal_kv_get(KEY_EASY_MATRIX, (uint8_t **)data, len);
  if (rt != 0) {
    PR_DEBUG("storeEasyMatrixGet tal_kv_get failed");
    *data = NULL;
    *len = 0;
  }
}

void storeEasyMatrixFree(void *data)
{
  PR_DEBUG("storeEasyMatrixFree");
  tal_kv_free((uint8_t *)data);
}

void storeEasyMatrixErase(void)
{
  PR_DEBUG("storeEasyMatrixErase");
  tal_kv_del(KEY_EASY_MATRIX);
}
