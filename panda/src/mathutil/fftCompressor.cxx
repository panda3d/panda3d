/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fftCompressor.cxx
 * @author drose
 * @date 2000-12-11
 */

#include "fftCompressor.h"
#include "config_mathutil.h"
#include "config_linmath.h"

#include "datagram.h"
#include "datagramIterator.h"
#include "compose_matrix.h"
#include "pmap.h"
#include <math.h>

#ifdef HAVE_FFTW

// hack..... this is a hack to help interrogate sort out a macro in the system
// poll and select definitions
#ifdef howmany
#undef howmany
#endif

#include <fftw3.h>

// These FFTW support objects can only be defined if we actually have the FFTW
// library available.
static fftw_plan get_real_compress_plan(int length);
static fftw_plan get_real_decompress_plan(int length);

typedef pmap<int, fftw_plan> RealPlans;
static RealPlans _real_compress_plans;
static RealPlans _real_decompress_plans;

#endif

/**
 * Constructs a new compressor object with default parameters.
 */
FFTCompressor::
FFTCompressor() {
  _bam_minor_version = 0;
  set_quality(-1);
  _use_error_threshold = false;
  _transpose_quats = false;
}

/**
 * Returns true if the FFTW library is compiled in, so that this class is
 * actually capable of doing useful compression/decompression work.  Returns
 * false otherwise, in which case any attempt to write a compressed stream
 * will actually write an uncompressed stream, and any attempt to read a
 * compressed stream will fail.
 */
bool FFTCompressor::
is_compression_available() {
#ifndef HAVE_FFTW
  return false;
#else
  return true;
#endif
}

/**
 * Sets the quality factor for the compression.  This is an integer in the
 * range 0 - 100 that roughly controls how aggressively the reals are
 * compressed; lower numbers mean smaller output, and more data loss.
 *
 * There are a few special cases.  Quality -1 means to use whatever individual
 * parameters are set in the user's Configrc file, rather than the single
 * quality dial.  Quality 101 or higher means to generate lossless output
 * (this is the default if libfftw is not available).
 *
 * Quality 102 writes all four components of quaternions to the output file,
 * rather than just three, quality 103 converts hpr to matrix (instead of
 * quat) and writes a 9-component matrix, and quality 104 just writes out hpr
 * directly.  Quality levels 102 and greater are strictly for debugging
 * purposes, and are only available if NDEBUG is not defined.
 */
void FFTCompressor::
set_quality(int quality) {
#ifndef HAVE_FFTW
  // If we don't actually have FFTW, we can't really compress anything.
  if (_quality <= 100) {
    mathutil_cat.warning()
      << "FFTW library is not available; generating uncompressed output.\n";
  }
  _quality = 101;

#else
  _quality = quality;

  if (_quality < 0) {
    // A negative quality indicates we should read the various parameters from
    // individual config variables.
    _fft_offset = fft_offset;
    _fft_factor = fft_factor;
    _fft_exponent = fft_exponent;

  } else if (_quality < 40) {
    // 0 - 40 : fft-offset 1.0 - 0.001 fft-factor 1.0 fft-exponent 4.0

    double t = (double)_quality / 40.0;
    _fft_offset = interpolate(t, 1.0, 0.001);
    _fft_factor = 1.0;
    _fft_exponent = 4.0;

  } else if (_quality < 95) {
    // 40 - 95: fft-offset 0.001 fft-factor 1.0 - 0.1 fft-exponent 4.0

    double t = (double)(_quality - 40) / 55.0;
    _fft_offset = 0.001;
    _fft_factor = interpolate(t, 1.0, 0.1);
    _fft_exponent = 4.0;

  } else {
    // 95 - 100: fft-offset 0.001 fft-factor 0.1 - 0.0 fft-exponent 4.0

    double t = (double)(_quality - 95) / 5.0;
    _fft_offset = 0.001;
    _fft_factor = interpolate(t, 0.1, 0.0);
    _fft_exponent = 4.0;
  }
#endif
}

/**
 * Returns the quality number that was previously set via set_quality().
 */
int FFTCompressor::
get_quality() const {
  return _quality;
}

/**
 * Enables or disables the use of the error threshold measurement to put a cap
 * on the amount of damage done by lossy compression.  When this is enabled,
 * the potential results of the compression are analyzed before the data is
 * written; if it is determined that the compression will damage a particular
 * string of reals too much, that particular string of reals is written
 * uncompressed.
 */
void FFTCompressor::
set_use_error_threshold(bool use_error_threshold) {
  _use_error_threshold = use_error_threshold;
}

/**
 * Returns whether the error threshold measurement is enabled.  See
 * set_use_error_threshold().
 */
bool FFTCompressor::
get_use_error_threshold() const {
  return _use_error_threshold;
}

/**
 * Sets the transpose_quats flag.  This is provided mainly for backward
 * compatibility with old bam files that were written out with the quaternions
 * inadvertently transposed.
 */
void FFTCompressor::
set_transpose_quats(bool flag) {
  _transpose_quats = flag;
}

/**
 * Returns the transpose_quats flag.  See set_transpose_quats().
 */
bool FFTCompressor::
get_transpose_quats() const {
  return _transpose_quats;
}

/**
 * Writes the compression parameters to the indicated datagram.  It is
 * necessary to call this before writing anything else to the datagram, since
 * these parameters will be necessary to correctly decompress the data later.
 */
void FFTCompressor::
write_header(Datagram &datagram) {
  datagram.add_int8(_quality);
  if (_quality < 0) {
    datagram.add_float64(_fft_offset);
    datagram.add_float64(_fft_factor);
    datagram.add_float64(_fft_exponent);
  }
}

/**
 * Writes an array of floating-point numbers to the indicated datagram.
 */
void FFTCompressor::
write_reals(Datagram &datagram, const PN_stdfloat *array, int length) {
  datagram.add_int32(length);

  if (_quality > 100) {
    // Special case: lossless output.
    for (int i = 0; i < length; i++) {
      datagram.add_stdfloat(array[i]);
    }
    return;
  }

#ifndef HAVE_FFTW
  // If we don't have FFTW, we shouldn't get here.
  nassertv(false);

#else

  if (length == 0) {
    // Special case: do nothing.
    return;
  }

  if (length == 1) {
    // Special case: just write out the one number.
    datagram.add_stdfloat(array[0]);
    return;
  }

  // Normal case: FFT the array, and write that out.

  // First, check the compressability.
  bool reject_compression = false;

  // This logic needs a closer examination.  Not sure it's useful as-is.
  /*
  if (_use_error_threshold) {
    // Don't encode the data if it moves too erratically.
    PN_stdfloat error = get_compressability(array, length);
    if (error > fft_error_threshold) {
      // No good: the data probably won't compress well.  Just write out
      // lossless data.
      reject_compression = true;
    }
  }
  */

  datagram.add_bool(reject_compression);
  if (reject_compression) {
    if (mathutil_cat.is_debug()) {
      mathutil_cat.debug()
        << "Writing stream of " << length << " numbers uncompressed.\n";
    }
    for (int i = 0; i < length; i++) {
      datagram.add_stdfloat(array[i]);
    }
    return;
  }

  // Now generate the Fourier transform.
  int fft_length = length / 2 + 1;
  fftw_complex *fft_bins = (fftw_complex *)alloca(fft_length * sizeof(fftw_complex));

  // This is for an in-place transform. It doesn't violate strict aliasing
  // rules because &fft_bins[0][0] is still a double pointer. This saves on
  // precious stack space.
  double *data = &fft_bins[0][0];
  int i;
  for (i = 0; i < length; i++) {
    data[i] = array[i];
  }

  // Note: This is an in-place DFT. `data` and `fft_bins` are aliases.
  fftw_plan plan = get_real_compress_plan(length);
  fftw_execute_dft_r2c(plan, data, fft_bins);


  // Now encode the numbers, run-length encoded by size, so we only write out
  // the number of bits we need for each number.
  // Note that Panda3D has conventionally always used FFTW2's halfcomplex
  // format for serializing the bins. In short, this means that for an n-length
  // FFT, it stores:
  // 1) The real components for bins 0 through floor(n/2), followed by...
  // 2) The imaginary components for bins floor((n+1)/2)-1 through 1.
  //    (Imaginary component for bin 0 is never stored, as that's always zero.)

  vector_double run;
  RunWidth run_width = RW_invalid;
  int num_written = 0;

  for (i = 0; i < length; i++) {
    static const double max_range_32 = 2147483647.0;
    static const double max_range_16 = 32767.0;
    static const double max_range_8 = 127.0;

    int bin; // which FFT bin we're storing
    int j; // 0=real; 1=imag
    if (i < fft_length) {
      bin = i;
      j = 0;
    } else {
      bin = length - i;
      j = 1;
    }

    double scale_factor = get_scale_factor(bin, fft_length);
    double num = cfloor(fft_bins[bin][j] / scale_factor + 0.5);

    // How many bits do we need to encode this integer?
    double a = fabs(num);
    RunWidth num_width;

    if (a == 0.0) {
      num_width = RW_0;

    } else if (a <= max_range_8) {
      num_width = RW_8;

    } else if (a <= max_range_16) {
      num_width = RW_16;

    } else if (a <= max_range_32) {
      num_width = RW_32;

    } else {
      num_width = RW_double;
    }

    // A special case: if we're writing a string of one-byters and we come
    // across a single intervening zero, don't interrupt the run just for
    // that.
    if (run_width == RW_8 && num_width == RW_0) {
      if (run.back() != 0) {
        num_width = RW_8;
      }
    }

    if (num_width != run_width) {
      // Now we need to flush the last run.

      // First, however, take care of the special case above: if we're
      // switching from RW_8 to RW_0, there could be a zero at the end, which
      // should be reclaimed into the RW_0 run.
      bool reclaimed_zero = (run_width == RW_8 && num_width == RW_0 &&
                             run.back() == 0);
      if (reclaimed_zero) {
        run.pop_back();
      }

      num_written += write_run(datagram, run_width, run);
      run.clear();
      run_width = num_width;

      if (reclaimed_zero) {
        run.push_back(0);
      }
    }

    run.push_back(num);
  }

  num_written += write_run(datagram, run_width, run);
  nassertv(num_written == length);
#endif
}

/**
 * Writes an array of HPR angles to the indicated datagram.
 */
void FFTCompressor::
write_hprs(Datagram &datagram, const LVecBase3 *array, int length) {
#ifndef NDEBUG
  if (_quality >= 104) {
    // If quality level is at least 104, we don't even convert hpr at all.
    // This is just for debugging.
    vector_stdfloat h, p, r;

    h.reserve(length);
    p.reserve(length);
    r.reserve(length);

    for (int i = 0; i < length; i++) {
      h.push_back(array[i][0]);
      p.push_back(array[i][1]);
      r.push_back(array[i][2]);
    }

    if (length == 0) {
      write_reals(datagram, nullptr, length);
      write_reals(datagram, nullptr, length);
      write_reals(datagram, nullptr, length);
    } else {
      write_reals(datagram, &h[0], length);
      write_reals(datagram, &p[0], length);
      write_reals(datagram, &r[0], length);
    }
    return;
  }
  if (_quality >= 103) {
    // If quality level is 103, we convert hpr to a table of matrices.  This
    // is just for debugging.
    vector_stdfloat
      m00, m01, m02,
      m10, m11, m12,
      m20, m21, m22;
    for (int i = 0; i < length; i++) {
      LMatrix3 mat;
      compose_matrix(mat, LVecBase3(1.0, 1.0, 1.0), LVecBase3(0.0, 0.0, 0.0),
                     array[i]);
      m00.push_back(mat(0, 0));
      m01.push_back(mat(0, 1));
      m02.push_back(mat(0, 2));
      m10.push_back(mat(1, 0));
      m11.push_back(mat(1, 1));
      m12.push_back(mat(1, 2));
      m20.push_back(mat(2, 0));
      m21.push_back(mat(2, 1));
      m22.push_back(mat(2, 2));
    }

    if (length == 0) {
      write_reals(datagram, nullptr, length);
      write_reals(datagram, nullptr, length);
      write_reals(datagram, nullptr, length);
      write_reals(datagram, nullptr, length);
      write_reals(datagram, nullptr, length);
      write_reals(datagram, nullptr, length);
      write_reals(datagram, nullptr, length);
      write_reals(datagram, nullptr, length);
      write_reals(datagram, nullptr, length);
    } else {
      write_reals(datagram, &m00[0], length);
      write_reals(datagram, &m01[0], length);
      write_reals(datagram, &m02[0], length);
      write_reals(datagram, &m10[0], length);
      write_reals(datagram, &m11[0], length);
      write_reals(datagram, &m12[0], length);
      write_reals(datagram, &m20[0], length);
      write_reals(datagram, &m21[0], length);
      write_reals(datagram, &m22[0], length);
    }
    return;
  }
#endif

  // First, convert the HPR's to quats.  We expect quats to have better FFT
  // consistency, and therefore compress better, even though they have an
  // extra component.

  // However, because the quaternion will be normalized, we don't even have to
  // write out all three components; any three can be used to determine the
  // fourth (provided we ensure consistency of sign).

  vector_stdfloat qr, qi, qj, qk;

  qr.reserve(length);
  qi.reserve(length);
  qj.reserve(length);
  qk.reserve(length);

  for (int i = 0; i < length; i++) {
    LMatrix3 mat;
    compose_matrix(mat, LVecBase3(1.0, 1.0, 1.0), LVecBase3(0.0, 0.0, 0.0),
                   array[i]);
    if (_transpose_quats) {
      mat.transpose_in_place();
    }

    LOrientation rot(mat);
    rot.normalize();  // This may not be necessary, but let's not take chances.

    if (rot.get_r() < 0) {
      // Since rot == -rot, we can flip the quarternion if need be to keep the
      // r component positive.  This has two advantages.  One, it makes it
      // possible to infer r completely given i, j, and k (since we know it
      // must be >= 0), and two, it helps protect against poor continuity
      // caused by inadvertent flipping of the quarternion's sign between
      // frames.

      // The choice of leaving r implicit rather than any of the other three
      // seems to work the best in terms of guaranteeing continuity.
      rot.set(-rot.get_r(), -rot.get_i(), -rot.get_j(), -rot.get_k());
    }

#ifdef NOTIFY_DEBUG
    if (mathutil_cat.is_warning()) {
      LMatrix3 mat2;
      rot.extract_to_matrix(mat2);
      if (!mat.almost_equal(mat2, 0.0001)) {
        LVecBase3 hpr1, hpr2;
        LVecBase3 scale, shear;
        decompose_matrix(mat, scale, shear, hpr1);
        decompose_matrix(mat2, scale, shear, hpr2);
        mathutil_cat.warning()
          << "Converted hpr to quaternion incorrectly!\n"
          << "  Source hpr: " << array[i] << ", or " << hpr1 << "\n";
        mathutil_cat.warning(false)
          << "  Quaternion: " << rot << "\n"
          << "  Which represents: hpr " << hpr2 << " scale "
          << scale << "\n";
      }
    }
#endif

    qr.push_back(rot.get_r());
    qi.push_back(rot.get_i());
    qj.push_back(rot.get_j());
    qk.push_back(rot.get_k());
  }

  // If quality is at least 102, we write all four quat components, instead of
  // just the three.  This is just for debugging.
#ifndef NDEBUG
  if (_quality >= 102) {
    if (length == 0) {
      write_reals(datagram, nullptr, length);
    } else {
      write_reals(datagram, &qr[0], length);
    }
  }
#endif
  if (length == 0) {
    write_reals(datagram, nullptr, length);
    write_reals(datagram, nullptr, length);
    write_reals(datagram, nullptr, length);
  } else {
    write_reals(datagram, &qi[0], length);
    write_reals(datagram, &qj[0], length);
    write_reals(datagram, &qk[0], length);
  }
}

/**
 * Reads the compression header that was written previously.  This fills in
 * the compression parameters necessary to correctly decompress the following
 * data.
 *
 * Returns true if the header is read successfully, false otherwise.
 */
bool FFTCompressor::
read_header(DatagramIterator &di, int bam_minor_version) {
  _bam_minor_version = bam_minor_version;
  _quality = di.get_int8();

  if (mathutil_cat.is_debug()) {
    mathutil_cat.debug()
      << "Found compressed data at quality level " << _quality << "\n";
  }

#ifndef HAVE_FFTW
  if (_quality <= 100) {
    mathutil_cat.error()
      << "FFTW library is not available; cannot read compressed data.\n";
    return false;
  }
#endif

  set_quality(_quality);

  if (_quality < 0) {
    _fft_offset = di.get_float64();
    _fft_factor = di.get_float64();
    _fft_exponent = di.get_float64();
  }

  return true;
}

/**
 * Reads an array of floating-point numbers.  The result is pushed onto the
 * end of the indicated vector, which is not cleared first; it is the user's
 * responsibility to ensure that the array is initially empty.  Returns true
 * if the data is read correctly, false if there is an error.
 */
bool FFTCompressor::
read_reals(DatagramIterator &di, vector_stdfloat &array) {
  int length = di.get_int32();

  if (_quality > 100) {
    array.reserve(array.size() + length);

    // Special case: lossless output.
    for (int i = 0; i < length; i++) {
      array.push_back(di.get_stdfloat());
    }
    return true;
  }

#ifndef HAVE_FFTW
  // If we don't have FFTW, we shouldn't get here.
  return false;

#else

  if (length == 0) {
    // Special case: do nothing.
    return true;
  }

  if (length == 1) {
    // Special case: just read in the one number.
    array.push_back(di.get_stdfloat());
    return true;
  }

  // Normal case: read in the FFT array, and convert it back to (nearly) the
  // original numbers.

  // First, check the reject_compression flag.  If it's set, we decided to
  // just write out the stream uncompressed.
  bool reject_compression = di.get_bool();
  if (reject_compression) {
    array.reserve(array.size() + length);
    for (int i = 0; i < length; i++) {
      array.push_back(di.get_stdfloat());
    }
    return true;
  }

  vector_double half_complex;
  half_complex.reserve(length);
  int num_read = 0;
  while (num_read < length) {
    num_read += read_run(di, half_complex);
  }
  nassertr(num_read == length, false);
  nassertr((int)half_complex.size() == length, false);

  int fft_length = length / 2 + 1;
  fftw_complex *fft_bins = (fftw_complex *)alloca(fft_length * sizeof(fftw_complex));

  int i;
  for (i = 0; i < fft_length; i++) {
    double scale_factor = get_scale_factor(i, fft_length);

    // For an explanation of this, see the compression code's comment about the
    // halfcomplex format.

    fft_bins[i][0] = half_complex[i] * scale_factor;
    if (i == 0) {
      // First bin doesn't store imaginary component
      fft_bins[i][1] = 0.0;
    } else if ((i == fft_length - 1) && !(length & 1)) {
      // Last bin doesn't store imaginary component with even lengths
      fft_bins[i][1] = 0.0;
    } else {
      fft_bins[i][1] = half_complex[length - i] * scale_factor;
    }
  }

  // This is for an in-place transform. It doesn't violate strict aliasing
  // rules because &fft_bins[0][0] is still a double pointer. This saves on
  // precious stack space.
  double *data = &fft_bins[0][0];

  // Note: This is an in-place DFT. `data` and `fft_bins` are aliases.
  fftw_plan plan = get_real_decompress_plan(length);
  fftw_execute_dft_c2r(plan, fft_bins, data);

  double scale = 1.0 / (double)length;
  array.reserve(array.size() + length);
  for (i = 0; i < length; i++) {
    array.push_back(data[i] * scale);
  }

  return true;
#endif
}

/**
 * Reads an array of HPR angles.  The result is pushed onto the end of the
 * indicated vector, which is not cleared first; it is the user's
 * responsibility to ensure that the array is initially empty.
 *
 * new_hpr is a temporary, transitional parameter.  If it is set false, the
 * hprs are decompressed according to the old, broken hpr calculation; if
 * true, the hprs are decompressed according to the new, correct hpr
 * calculation.
 */
bool FFTCompressor::
read_hprs(DatagramIterator &di, pvector<LVecBase3> &array, bool new_hpr) {
#ifndef NDEBUG
  if (_quality >= 104) {
    // If quality level is at least 104, we don't even convert hpr to quat.
    // This is just for debugging.
    vector_stdfloat h, p, r;
    bool okflag = true;
    okflag =
      read_reals(di, h) &&
      read_reals(di, p) &&
      read_reals(di, r);

    if (okflag) {
      nassertr(h.size() == p.size() && p.size() == r.size(), false);
      for (int i = 0; i < (int)h.size(); i++) {
        array.push_back(LVecBase3(h[i], p[i], r[i]));
      }
    }

    return okflag;
  }
  if (_quality >= 103) {
    // If quality level is 103, we read in a table of 3x3 rotation matrices.
    // This is just for debugging.
    vector_stdfloat
      m00, m01, m02,
      m10, m11, m12,
      m20, m21, m22;
    bool okflag = true;
    okflag =
      read_reals(di, m00) &&
      read_reals(di, m01) &&
      read_reals(di, m02) &&
      read_reals(di, m10) &&
      read_reals(di, m11) &&
      read_reals(di, m12) &&
      read_reals(di, m20) &&
      read_reals(di, m21) &&
      read_reals(di, m22);

    if (okflag) {
      for (int i = 0; i < (int)m00.size(); i++) {
        LMatrix3 mat(m00[i], m01[i], m02[i],
                     m10[i], m11[i], m12[i],
                     m20[i], m21[i], m22[i]);
        LVecBase3 scale, shear, hpr;
        if (new_hpr) {
          decompose_matrix(mat, scale, shear, hpr);
        } else {
          decompose_matrix_old_hpr(mat, scale, shear, hpr);
        }
        array.push_back(hpr);
      }
    }

    return okflag;
  }
#endif

  vector_stdfloat qr, qi, qj, qk;

  bool okflag = true;

#ifndef NDEBUG
  if (_quality >= 102) {
    okflag = read_reals(di, qr);
  }
#endif

  okflag =
    okflag &&
    read_reals(di, qi) &&
    read_reals(di, qj) &&
    read_reals(di, qk);

  if (okflag) {
    nassertr(qi.size() == qj.size() && qj.size() == qk.size(), false);

    array.reserve(array.size() + qi.size());
    for (int i = 0; i < (int)qi.size(); i++) {
      LOrientation rot;

      // Infer the r component from the remaining three.
      PN_stdfloat qr2 = 1.0 - (qi[i] * qi[i] + qj[i] * qj[i] + qk[i] * qk[i]);
      PN_stdfloat qr1 = qr2 < 0.0 ? 0.0 : sqrtf(qr2);

      rot.set(qr1, qi[i], qj[i], qk[i]);

#ifndef NDEBUG
      if (_quality >= 102) {
        // If we have written out all four components, use them.
        rot[0] = qr[i];

        if (!IS_THRESHOLD_EQUAL(qr[i], qr1, 0.001)) {
          mathutil_cat.warning()
            << "qr[" << i << "] = " << qr[i] << ", qr1 = " << qr1
            << ", diff is " << qr1 - qr[i] << "\n";
        }
      } else
#endif

      rot.normalize();      // Just for good measure.

      LMatrix3 mat;
      rot.extract_to_matrix(mat);
      if (_transpose_quats) {
        mat.transpose_in_place();
      }
      LVecBase3 scale, shear, hpr;
      if (new_hpr) {
        decompose_matrix(mat, scale, shear, hpr);
      } else {
        decompose_matrix_old_hpr(mat, scale, shear, hpr);
      }
      array.push_back(hpr);
    }
  }

  return okflag;
}

/**
 * Reads an array of HPR angles.  The result is pushed onto the end of the
 * indicated vector, which is not cleared first; it is the user's
 * responsibility to ensure that the array is initially empty.
 */
bool FFTCompressor::
read_hprs(DatagramIterator &di, pvector<LVecBase3> &array) {
  return read_hprs(di, array, true);
}


/**
 * Frees memory that has been allocated during past runs of the FFTCompressor.
 * This is an optional call, but it may be made from time to time to empty the
 * global cache that the compressor objects keep to facilitate fast
 * compression/decompression.
 */
void FFTCompressor::
free_storage() {
#ifdef HAVE_FFTW
  RealPlans::iterator pi;
  for (pi = _real_compress_plans.begin();
       pi != _real_compress_plans.end();
       ++pi) {
    fftw_destroy_plan((*pi).second);
  }
  _real_compress_plans.clear();

  for (pi = _real_decompress_plans.begin();
       pi != _real_decompress_plans.end();
       ++pi) {
    fftw_destroy_plan((*pi).second);
  }
  _real_decompress_plans.clear();
#endif
}

/**
 * Writes a sequence of integers that all require the same number of bits.
 * Returns the number of integers written, i.e.  run.size().
 */
int FFTCompressor::
write_run(Datagram &datagram, FFTCompressor::RunWidth run_width,
          const vector_double &run) {
  if (run.empty()) {
    return 0;
  }
  nassertr(run_width != RW_invalid, 0);

  if (run_width != RW_double) {
    // If the width is anything other than RW_double, we write a single byte
    // indicating the width and length of the upcoming run.

    if (run.size() <= RW_length_mask &&
        ((int)run_width | run.size()) != RW_double) {
      // If there are enough bits remaining in the byte, use them to indicate
      // the length of the run.  We have to be a little careful, however, not
      // to accidentally write a byte that looks like an RW_double flag.
      datagram.add_uint8((int)run_width | run.size());

    } else {
      // Otherwise, write zero as the length, to indicate that we'll write the
      // actual length in the following 16-bit word.
      datagram.add_uint8(run_width);

      // Assuming, of course, that the length fits within 16 bits.
      nassertr(run.size() < 65536, 0);
      nassertr(run.size() != 0, 0);

      datagram.add_uint16(run.size());
    }
  }

  // Now write the data itself.
  vector_double::const_iterator ri;
  switch (run_width) {
  case RW_0:
    // If it's a string of zeroes, we're done!
    break;

  case RW_8:
    for (ri = run.begin(); ri != run.end(); ++ri) {
      datagram.add_int8((int)*ri);
    }
    break;

  case RW_16:
    for (ri = run.begin(); ri != run.end(); ++ri) {
      datagram.add_int16((int)*ri);
    }
    break;

  case RW_32:
    for (ri = run.begin(); ri != run.end(); ++ri) {
      datagram.add_int32((int)*ri);
    }
    break;

  case RW_double:
    for (ri = run.begin(); ri != run.end(); ++ri) {
      // In the case of RW_double, we only write the numbers one at a time,
      // each time preceded by the RW_double flag.  Hopefully this will happen
      // only rarely.
      datagram.add_int8((int8_t)RW_double);
      datagram.add_float64(*ri);
    }
    break;

  default:
    break;
  }

  return run.size();
}

/**
 * Reads a sequence of integers that all require the same number of bits.
 * Returns the number of integers read.  It is the responsibility of the user
 * to clear the vector before calling this function, or the numbers read will
 * be appended to the end.
 */
int FFTCompressor::
read_run(DatagramIterator &di, vector_double &run) {
  uint8_t start = di.get_uint8();
  RunWidth run_width;
  int length;

  if ((start & 0xff) == RW_double) {
    // RW_double is a special case, and requires the whole byte.  In this
    // case, we don't encode a length, but assume it's only one.
    run_width = RW_double;
    length = 1;

  } else {
    run_width = (RunWidth)(start & RW_width_mask);
    length = start & RW_length_mask;
  }

  if (length == 0) {
    // If the length was zero, it means the actual length follows as a 16-bit
    // word.
    length = di.get_uint16();
  }
  nassertr(length != 0, 0);

  run.reserve(run.size() + length);

  int i;
  switch (run_width) {
  case RW_0:
    for (i = 0; i < length; i++) {
      run.push_back(0.0);
    }
    break;

  case RW_8:
    for (i = 0; i < length; i++) {
      run.push_back((double)(int)di.get_int8());
    }
    break;

  case RW_16:
    for (i = 0; i < length; i++) {
      run.push_back((double)(int)di.get_int16());
    }
    break;

  case RW_32:
    for (i = 0; i < length; i++) {
      run.push_back((double)(int)di.get_int32());
    }
    break;

  case RW_double:
    for (i = 0; i < length; i++) {
      run.push_back(di.get_float64());
    }
    break;

  default:
    break;
  }

  return length;
}

/**
 * Returns the appropriate scaling for the given bin in the FFT output.
 *
 * The scale factor is the value of one integer in the quantized data. As such,
 * greater bins (higher, more noticeable frequencies) have *lower* scaling
 * factors, which means greater precision.
 */
double FFTCompressor::
get_scale_factor(int i, int length) const {
  nassertr(i < length, 1.0);

  return _fft_offset +
    _fft_factor * pow((double)(length - i) / (double)(length), _fft_exponent);
}

/**
 * Returns a number between a and b, inclusive, according to the value of t
 * between 0 and 1, inclusive.
 */
double FFTCompressor::
interpolate(double t, double a, double b) {
  return a + t * (b - a);
}

/**
 * Returns a factor that indicates how erratically the values are changing.
 * The lower the result, the calmer the numbers, and the greater its
 * likelihood of being successfully compressed.
 */
PN_stdfloat FFTCompressor::
get_compressability(const PN_stdfloat *data, int length) const {
  // The result returned is actually the standard deviation of the table of
  // deltas between consecutive frames.  This number is larger if the frames
  // have wildly different values.

  if (length <= 2) {
    return 0.0;
  }

  PN_stdfloat sum = 0.0;
  PN_stdfloat sum2 = 0.0;
  for (int i = 1; i < length; i++) {
    PN_stdfloat delta = data[i] - data[i - 1];

    sum += delta;
    sum2 += delta * delta;
  }
  PN_stdfloat variance = (sum2 - (sum * sum) / (length - 1)) / (length - 2);
  if (variance < 0.0) {
    // This can only happen due to tiny roundoff error.
    return 0.0;
  }

  PN_stdfloat std_deviation = csqrt(variance);

  return std_deviation;
}



#ifdef HAVE_FFTW

/**
 * Returns a FFTW plan suitable for compressing a float array of the indicated
 * length.
 */
static fftw_plan
get_real_compress_plan(int length) {
  RealPlans::iterator pi;
  pi = _real_compress_plans.find(length);
  if (pi != _real_compress_plans.end()) {
    return (*pi).second;
  }

  fftw_plan plan;
  plan = fftw_plan_dft_r2c_1d(length, nullptr, nullptr, FFTW_ESTIMATE);
  _real_compress_plans.insert(RealPlans::value_type(length, plan));

  return plan;
}

/**
 * Returns a FFTW plan suitable for decompressing a float array of the
 * indicated length.
 */
static fftw_plan
get_real_decompress_plan(int length) {
  RealPlans::iterator pi;
  pi = _real_decompress_plans.find(length);
  if (pi != _real_decompress_plans.end()) {
    return (*pi).second;
  }

  fftw_plan plan;
  plan = fftw_plan_dft_c2r_1d(length, nullptr, nullptr, FFTW_ESTIMATE);
  _real_decompress_plans.insert(RealPlans::value_type(length, plan));

  return plan;
}

#endif
