// Filename: crypto_utils.h
// Created by:  drose (07Nov00)
//
////////////////////////////////////////////////////////////////////

#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <pandabase.h>
#include <filename.h>
#include <typedef.h>

class EXPCL_PANDAEXPRESS HashVal {
public:
  uint get_value(int val) const {
    if (val < 4 && val >= 0)
      return hv[val];
  }
  uint hv[4];
};

EXPCL_PANDAEXPRESS void md5_a_file(const Filename &fname, HashVal &ret);
EXPCL_PANDAEXPRESS void md5_a_buffer(uchar *buf, ulong len, HashVal &ret);

#endif

