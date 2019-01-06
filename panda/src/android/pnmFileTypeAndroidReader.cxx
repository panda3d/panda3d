/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypeAndroidReader.cxx
 * @author rdb
 * @date 2013-01-22
 */

#include "pnmFileTypeAndroid.h"

#ifdef ANDROID

#include "config_pnmimagetypes.h"
#include "config_express.h"

#include <android/bitmap.h>
#include <jni.h>

// These tables linearly map 4-bit, 5-bit or 6-bit to 8-bit values.
static uint8_t scale_table_4[16];
static uint8_t scale_table_5[32];
static uint8_t scale_table_6[64];

static void init_scale_tables() {
  static bool initialized = false;
  if (!initialized) {
    int i;
    for (i = 0; i < 16; ++i) {
      scale_table_4[i] = 255 * i / 15;
    }
    for (i = 0; i < 32; ++i) {
      scale_table_5[i] = 255 * i / 31;
    }
    for (i = 0; i < 64; ++i) {
      scale_table_6[i] = 255 * i / 63;
    }
    initialized = true;
  }
}

static void conv_rgb565(uint16_t in, xel &out) {
  out.r = scale_table_5[(in >> 11) & 31];
  out.g = scale_table_6[(in >> 5) & 63];
  out.b = scale_table_5[in & 31];
}

static void conv_rgba4444(uint16_t in, xel &rgb, xelval &alpha) {
  rgb.r = scale_table_4[(in >> 12) & 0xF];
  rgb.g = scale_table_4[(in >> 8) & 0xF];
  rgb.b = scale_table_4[(in >> 4) & 0xF];
  alpha = scale_table_4[in & 0xF];
}

/**
 *
 */
PNMFileTypeAndroid::Reader::
Reader(PNMFileType *type, std::istream *file, bool owns_file, std::string magic_number) :
  PNMReader(type, file, owns_file), _bitmap(nullptr)
{
  // Hope we can putback() more than one character.
  for (std::string::reverse_iterator mi = magic_number.rbegin();
       mi != magic_number.rend(); ++mi) {
    _file->putback(*mi);
  };
  if (_file->fail()) {
    android_cat.error()
      << "Unable to put back magic number.\n";
    _is_valid = false;
    return;
  }

  std::streampos pos = _file->tellg();

  Thread *current_thread = Thread::get_current_thread();
  _env = current_thread->get_jni_env();
  nassertd(_env != nullptr) {
    _is_valid = false;
    return;
  }

  jobject opts = _env->CallStaticObjectMethod(jni_PandaActivity,
                                              jni_PandaActivity_readBitmapSize,
                                              (jlong) _file);
  _file->clear();
  _file->seekg(pos);
  if (_file->tellg() != pos) {
    android_cat.error()
      << "Unable to seek back to beginning.\n";
    _is_valid = false;
    return;
  }

  _x_size = _env->GetIntField(opts, jni_BitmapFactory_Options_outWidth);
  _y_size = _env->GetIntField(opts, jni_BitmapFactory_Options_outHeight);

  if (_x_size < 0 || _y_size < 0) {
    android_cat.error()
      << "Failed to read header of " << *this << "\n";
    _is_valid = false;
  }

  // Apparently we have to know this even though we don't yet.
  _num_channels = 4;
  _maxval = 255;

  if (android_cat.is_debug()) {
    android_cat.debug()
      << "Reading " << *this << "\n";
  }
}

/**
 *
 */
PNMFileTypeAndroid::Reader::
~Reader() {
  if (_bitmap != nullptr) {
    _env->DeleteGlobalRef(_bitmap);
  }
}

/**
 * This method will be called before read_data() or read_row() is called.  It
 * instructs the reader to initialize its data structures as necessary to
 * actually perform the read operation.
 *
 * After this call, _x_size and _y_size should reflect the actual size that
 * will be filled by read_data() (as possibly modified by set_read_size()).
 */
void PNMFileTypeAndroid::Reader::
prepare_read() {
  _sample_size = 2;
  _orig_x_size = _x_size;
  _orig_y_size = _y_size;

  if (_has_read_size && _read_x_size != 0 && _read_y_size != 0) {
    int x_reduction = _orig_x_size / _read_x_size;
    int y_reduction = _orig_y_size / _read_y_size;

    _sample_size = std::max(std::min(x_reduction, y_reduction), 1);
  }

  _bitmap = _env->CallStaticObjectMethod(jni_PandaActivity,
                                         jni_PandaActivity_readBitmap,
                                         (jlong) _file, _sample_size);

  if (_bitmap == nullptr) {
    android_cat.error()
      << "Failed to read " << *this << "\n";
    _is_valid = false;
    return;
  }

  _bitmap = _env->NewGlobalRef(_bitmap);

  AndroidBitmapInfo info;
  if (AndroidBitmap_getInfo(_env, _bitmap, &info) < 0) {
    android_cat.error()
      << "Failed to get info of " << *this << "\n";
    _is_valid = false;
    return;
  }

  _x_size = info.width;
  _y_size = info.height;
  _format = info.format;
  _stride = info.stride;

  // Note: we could be setting maxval more appropriately, but this only causes
  // texture.cxx to end up rescaling it later.  Best to do the scaling
  // ourselves, using efficient tables.
  _maxval = 255;

  switch (info.format) {
    case ANDROID_BITMAP_FORMAT_RGBA_8888:
      _num_channels = 4;
      android_cat.debug()
        << "Bitmap has format RGBA_8888\n";
      break;
    case ANDROID_BITMAP_FORMAT_RGB_565:
      _num_channels = 3;
      android_cat.debug()
        << "Bitmap has format RGB_565\n";
      break;
    case ANDROID_BITMAP_FORMAT_RGBA_4444:
      _num_channels = 4;
      android_cat.debug()
        << "Bitmap has format RGBA_4444\n";
      break;
    case ANDROID_BITMAP_FORMAT_A_8:
      _num_channels = 1;
      android_cat.debug()
        << "Bitmap has format A_8\n";
      break;
    default:
      android_cat.error()
        << "Unsupported bitmap format!\n";
      _num_channels = 0;
      _is_valid = false;
      break;
  }
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
int PNMFileTypeAndroid::Reader::
read_data(xel *rgb, xelval *alpha) {
  if (!_is_valid) {
    return 0;
  }
  void *ptr;
  if (AndroidBitmap_lockPixels(_env, _bitmap, &ptr) < 0) {
    android_cat.error()
      << "Failed to lock bitmap for reading.\n";
    return 0;
  }

  switch (_format) {
    case ANDROID_BITMAP_FORMAT_RGBA_8888: {
      nassertr(_stride == _x_size * 4, 0);
      uint8_t *data = (uint8_t *) ptr;
      for (int y = 0; y < _y_size; ++y) {
        for (int x = 0; x < _x_size; ++x) {
          rgb[x].r = data[0];
          rgb[x].g = data[1];
          rgb[x].b = data[2];
          alpha[x] = data[3];
          data += 4;
        }
        rgb += _x_size;
        alpha += _y_size;
      }
      break;
    }
    case ANDROID_BITMAP_FORMAT_RGB_565: {
      nassertr(_stride == _x_size * 2, 0);
      init_scale_tables();

      uint16_t *data = (uint16_t *) ptr;
      for (int y = 0; y < _y_size; ++y) {
        for (int x = 0; x < _x_size; ++x) {
          conv_rgb565(data[x], rgb[x]);
        }
        data += _x_size;
        rgb += _x_size;
      }
      break;
    }
    case ANDROID_BITMAP_FORMAT_RGBA_4444: {
      nassertr(_stride == _x_size * 2, 0);
      init_scale_tables();

      uint16_t *data = (uint16_t *) ptr;
      for (int y = 0; y < _y_size; ++y) {
        for (int x = 0; x < _x_size; ++x) {
          conv_rgba4444(data[x], rgb[x], alpha[x]);
        }
        data += _x_size;
        rgb += _x_size;
        alpha += _x_size;
      }
      break;
    }
    case ANDROID_BITMAP_FORMAT_A_8: {
      nassertr(_stride == _x_size, 0);
      uint8_t *data = (uint8_t *) ptr;
      for (int y = 0; y < _y_size; ++y) {
        for (int x = 0; x < _x_size; ++x) {
          alpha[x] = data[x];
        }
        data += _x_size;
        alpha += _x_size;
      }
      break;
    }
    default:
      AndroidBitmap_unlockPixels(_env, _bitmap);
      return 0;
  }

  AndroidBitmap_unlockPixels(_env, _bitmap);
  return _y_size;
}

#endif  // ANDROID
