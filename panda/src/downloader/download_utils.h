// Filename: download_utils.h
// Created by:  mike (18Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef DOWNLOAD_UTILS_H
#define DOWNLOAD_UTILS_H

#include <pandabase.h>
#include <typedef.h>
#include <filename.h>

typedef uint HashVal[4];

EXPCL_PANDAEXPRESS ulong check_crc(Filename name);
EXPCL_PANDAEXPRESS ulong check_adler(Filename name);
EXPCL_PANDAEXPRESS void md5_a_file(const Filename &fname, HashVal &ret);
EXPCL_PANDAEXPRESS void md5_a_buffer(uchar *buf, ulong len, HashVal &ret);

#endif

