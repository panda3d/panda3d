// Filename: crypto_utils.h
// Created by:  drose (07Nov00)
//
////////////////////////////////////////////////////////////////////

#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <pandabase.h>
#include <filename.h>
#include "typedef.h"

class HashVal;

BEGIN_PUBLISH

EXPCL_PANDAEXPRESS void md5_a_file(const Filename &fname, HashVal &ret);
EXPCL_PANDAEXPRESS void md5_a_buffer(const uchar *buf, ulong len, HashVal &ret);

END_PUBLISH

#endif

