// Filename: fftCompressor.h
// Created by:  drose (11Dec00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef FFTCOMPRESSOR_H
#define FFTCOMPRESSOR_H

#include "pandabase.h"

#include "pointerToArray.h"
#include "vector_float.h"
#include "vector_double.h"
#include "vector_LVecBase3f.h"

class Datagram;
class DatagramIterator;

////////////////////////////////////////////////////////////////////
//       Class : FFTCompressor
// Description : This class manages a lossy compression and
//               decompression of a stream of floating-point numbers
//               to a datagram, based a fourier transform algorithm
//               (similar in principle to JPEG compression).
//
//               Actually, it doesn't do any real compression on its
//               own; it just outputs a stream of integers that should
//               compress much tighter via gzip than the original
//               stream of floats would have.
//
//               This class depends on the external FFTW library;
//               without it, it will fall back on lossless output of
//               the original data.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FFTCompressor {
public:
  FFTCompressor();

  static bool is_compression_available();

  void set_quality(int quality);
  int get_quality() const;

  void set_use_error_threshold(bool use_error_threshold);
  bool get_use_error_threshold() const;

  void set_transpose_quats(bool flag);
  bool get_transpose_quats() const;

  void write_header(Datagram &datagram);
  void write_reals(Datagram &datagram, const float *array, int length);
  void write_hprs(Datagram &datagram, const LVecBase3f *array, int length);

  bool read_header(DatagramIterator &di, int bam_minor_version);
  bool read_reals(DatagramIterator &di, vector_float &array);
  bool read_hprs(DatagramIterator &di, vector_LVecBase3f &array,
                 bool new_hpr);
  bool read_hprs(DatagramIterator &di, vector_LVecBase3f &array);

  static void free_storage();

private:
  enum RunWidth {
    // We write a byte to the datagram at the beginning of each run to
    // encode the width and length of the run.  The width is indicated
    // by the top two bits, while the length fits in the lower six,
    // except RW_double, which is a special case.
    RW_width_mask  = 0xc0,
    RW_length_mask = 0x3f,
    RW_0           = 0x00,
    RW_8           = 0x40,
    RW_16          = 0x80,
    RW_32          = 0xc0,
    RW_double      = 0xff,
    RW_invalid     = 0x01
  };

  int write_run(Datagram &datagram, RunWidth run_width,
                const vector_double &run);
  int read_run(DatagramIterator &di, vector_double &run);
  double get_scale_factor(int i, int length) const;
  static double interpolate(double t, double a, double b);

  float get_compressability(const float *data, int length) const;

  int _bam_minor_version;
  int _quality;
  bool _use_error_threshold;
  double _fft_offset;
  double _fft_factor;
  double _fft_exponent;
  bool _transpose_quats;
};

#endif

