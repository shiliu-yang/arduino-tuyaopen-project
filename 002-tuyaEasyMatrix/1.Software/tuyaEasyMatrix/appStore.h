#ifndef __APP_STORE_H__
#define __APP_STORE_H__

#include "Arduino.h"

void storeEasyMatrix(void *data, size_t len);

void storeEasyMatrixGet(void **data, size_t *len);

void storeEasyMatrixFree(void *data);

void storeEasyMatrixErase(void);

#endif
