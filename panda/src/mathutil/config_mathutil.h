// Filename: config_mathutil.h
// Created by:  drose (01Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_MATHUTIL_H
#define CONFIG_MATHUTIL_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>

NotifyCategoryDecl(mathutil, EXPCL_PANDA, EXPTP_PANDA);

extern const double fft_offset;
extern const double fft_factor;
extern const double fft_exponent;

#endif


