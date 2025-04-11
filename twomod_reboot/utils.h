#ifndef _UTILS_H 
#define _UTILS_H


#ifdef UPLOAD_METHOD_DEBUGPROBE
  #define DEBUG_SERIAL Serial1
#else
  #define DEBUG_SERIAL Serial
#endif
#include "twomode.h"

#endif 