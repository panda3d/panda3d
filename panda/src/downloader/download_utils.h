// Filename: download_utils.h
// Created by:  mike (18Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef DOWNLOAD_UTILS_H
#define DOWNLOAD_UTILS_H

#include <pandabase.h>
#include <typedef.h>
#include <filename.h>

EXPCL_PANDAEXPRESS ulong check_crc(Filename name);
EXPCL_PANDAEXPRESS ulong check_adler(Filename name);

#endif

