/*
  This is a double-to-string conversion implementation by Milo Yip from:
    https://github.com/miloyip/dtoa-benchmark

  I introduced it because the ostringstream implementation is just too
  darned slow, especially when compiled to JavaScript.
*/

#ifndef PDTOA_H
#define PDTOA_H

#include "dtoolbase.h"

#ifdef __cplusplus
extern "C" {
#endif

EXPCL_DTOOL_DTOOLBASE void pdtoa(double value, char *buffer);

#ifdef __cplusplus
};  /* end of extern "C" */
#endif

#endif  // PDTOA_H
