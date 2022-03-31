/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmReader.cxx
 * @author drose
 * @date 2000-06-14
 */

#include "pnmReader.h"
#include "virtualFileSystem.h"
#include "thread.h"

/**
 *
 */
PNMReader::
~PNMReader() {
  if (_owns_file) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

    // We're assuming here that the file was opened via VFS.  That may not
    // necessarily be the case, but we don't make that distinction.  However,
    // at the moment at least, that distinction doesn't matter, since
    // vfs->close_read_file() just deletes the file pointer anyway.
    vfs->close_read_file(_file);
  }
  _file = nullptr;
}

/**
 * This method will be called before read_data() or read_row() is called.  It
 * instructs the reader to initialize its data structures as necessary to
 * actually perform the read operation.
 *
 * After this call, _x_size and _y_size should reflect the actual size that
 * will be filled by read_data() (as possibly modified by set_read_size()).
 */
void PNMReader::
prepare_read() {
  if (!_is_valid) {
    return;
  }

  _x_shift = 0;
  _y_shift = 0;
  _orig_x_size = _x_size;
  _orig_y_size = _y_size;

  if (supports_read_row()) {
    // Maybe we can quick-filter each row down as we go.
    if (_has_read_size) {
      _x_shift = get_reduction_shift(_x_size, _read_x_size);
      _x_size = _x_size / (1 << _x_shift);
      _y_shift = get_reduction_shift(_y_size, _read_y_size);
      _y_size = _y_size / (1 << _y_shift);
    }
  }
}

/**
 * Returns true if this PNMFileType represents a floating-point image type,
 * false if it is a normal, integer type.  If this returns true, read_pfm() is
 * implemented instead of read_data().
 */
bool PNMReader::
is_floating_point() {
  return false;
}

/**
 * Reads floating-point data directly into the indicated PfmFile.  Returns
 * true on success, false on failure.
 */
bool PNMReader::
read_pfm(PfmFile &pfm) {
  return false;
}

/**
 * Reads in an entire image all at once, storing it in the pre-allocated
 * _x_size * _y_size array and alpha pointers.  (If the image type has no
 * alpha channel, alpha is ignored.)  Returns the number of rows correctly
 * read.
 *
 * Derived classes need not override this if they instead provide
 * supports_read_row() and read_row(), below.
 */
int PNMReader::
read_data(xel *array, xelval *alpha) {
  if (!is_valid()) {
    return 0;
  }

  if (_x_shift == 0 && _y_shift == 0) {
    // Read with no reduction.
    int y;
    for (y = 0; y < _y_size; ++y) {
      if (!read_row(array + y * _x_size, alpha + y * _x_size, _x_size, _y_size)) {
        Thread::consider_yield();
        return y;
      }
    }

  } else {
    int x_reduction = (1 << _x_shift);
    int y_reduction = (1 << _y_shift);

    int shift = _x_shift + _y_shift;

    // We need a temporary buffer, at least one row wide, with full-width
    // integers, for accumulating pixel data.
    int *accum_row_array = (int *)alloca(_orig_x_size * sizeof(int) * 3);
    int *accum_row_alpha = (int *)alloca(_orig_x_size * sizeof(int));

    // Each time we read a row, we will actually read the full row here,
    // before we filter it down into the above.
    xel *orig_row_array = (xel *)alloca(_orig_x_size * sizeof(xel));
    xelval *orig_row_alpha = (xelval *)alloca(_orig_x_size * sizeof(xelval));

    int y;
    for (y = 0; y < _y_size; ++y) {
      // Zero out the accumulation data, in preparation for holding the
      // results of the below.
      memset(accum_row_array, 0, _x_size * sizeof(int) * 3);
      if (has_alpha()) {
        memset(accum_row_alpha, 0, _x_size * sizeof(int));
      }

      for (int yi = 0; yi < y_reduction; ++yi) {
        // OK, read a row.  This reads the original, full-size row.
        if (!read_row(orig_row_array, orig_row_alpha, _orig_x_size, _orig_y_size)) {
          Thread::consider_yield();
          return y;
        }

        // Boil that row down to its proper, reduced size, and accumulate it
        // into the target row.
        xel *p = orig_row_array;
        int *q = accum_row_array;
        int *qstop = q + _x_size * 3;
        while (q < qstop) {
          for (int xi = 0; xi < x_reduction; ++xi) {
            q[0] += (*p).r;
            q[1] += (*p).g;
            q[2] += (*p).b;
            ++p;
          }
          q += 3;
        }
        if (has_alpha()) {
          // Now do it again for the alpha channel.
          xelval *p = orig_row_alpha;
          int *q = accum_row_alpha;
          int *qstop = q + _x_size;
          while (q < qstop) {
            for (int xi = 0; xi < x_reduction; ++xi) {
              (*q) += (*p);
              ++p;
            }
            ++q;
          }
        }
      }

      // OK, now copy the accumulated pixel data into the final result.
      xel *target_row_array = array + y * _x_size;
      xelval *target_row_alpha = alpha + y * _x_size;

      int *p = accum_row_array;
      xel *q = target_row_array;
      xel *qstop = q + _x_size;
      while (q < qstop) {
        (*q).r = (*p++) >> shift;
        (*q).g = (*p++) >> shift;
        (*q).b = (*p++) >> shift;
        ++q;
      }

      if (has_alpha()) {
        int *p = accum_row_alpha;
        xelval *q = target_row_alpha;
        xelval *qstop = q + _x_size;
        while (q < qstop) {
          (*q) = (*p++) >> shift;
          ++q;
        }
      }
    }
  }


  return _y_size;
}


/**
 * Returns true if this particular PNMReader is capable of returning the data
 * one row at a time, via repeated calls to read_row().  Returns false if the
 * only way to read from this file is all at once, via read_data().
 */
bool PNMReader::
supports_read_row() const {
  return false;
}

/**
 * If supports_read_row(), above, returns true, this function may be called
 * repeatedly to read the image, one horizontal row at a time, beginning from
 * the top.  Returns true if the row is successfully read, false if there is
 * an error or end of file.
 *
 * The x_size and y_size parameters are the value of _x_size and _y_size as
 * originally filled in by the constructor; it is the actual number of pixels
 * in the image.  (The _x_size and _y_size members may have been automatically
 * modified by the time this method is called if we are scaling on load, so
 * should not be used.)
 */
bool PNMReader::
read_row(xel *, xelval *, int, int) {
  return false;
}


/**
 * Returns true if this particular PNMReader can read from a general stream
 * (including pipes, etc.), or false if the reader must occasionally fseek()
 * on its input stream, and thus only disk streams are supported.
 */
bool PNMReader::
supports_stream_read() const {
  return false;
}

/**
 * Determines the reduction factor between the original size and the requested
 * size, returned as an exponent of power of 2 (that is, a bit shift).
 *
 * Only power-of-two reductions are supported, since those are common, easy,
 * and fast.  Other reductions will be handled in the higher level code.
 */
int PNMReader::
get_reduction_shift(int orig_size, int new_size) {
  if (new_size == 0) {
    return 0;
  }

  int reduction = std::max(orig_size / new_size, 1);

  int shift = 0;

  int r = 2;
  while (r <= reduction) {
    shift += 1;
    r <<= 1;
  }

  if ((orig_size % r) != 0) {
    // If the reduction isn't even, never mind.
    shift = 0;
  }

  return shift;
}
