/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmFileTypeAndroidWriter.cxx
 * @author rdb
 * @date 2018-02-10
 */

#include "pnmFileTypeAndroid.h"

#ifdef ANDROID

#include "config_pnmimagetypes.h"

#include <android/bitmap.h>
#include <jni.h>

// See android/graphics/Bitmap.java
enum class BitmapConfig : jint {
  ALPHA_8 = 1,
  RGB_565 = 3,
  ARGB_4444 = 4,
  ARGB_8888 = 5,
  RGBA_F16 = 6,
  HARDWARE = 7,
};

/**
 *
 */
PNMFileTypeAndroid::Writer::
Writer(PNMFileType *type, std::ostream *file, bool owns_file,
       CompressFormat format) :
  PNMWriter(type, file, owns_file),
  _format(format)
{
}

/**
 * Writes out an entire image all at once, including the header, based on the
 * image data stored in the given _x_size * _y_size array and alpha pointers.
 * (If the image type has no alpha channel, alpha is ignored.) Returns the
 * number of rows correctly written.
 *
 * It is the user's responsibility to fill in the header data via calls to
 * set_x_size(), set_num_channels(), etc., or copy_header_from(), before
 * calling write_data().
 *
 * It is important to delete the PNMWriter class after successfully writing
 * the data.  Failing to do this may result in some data not getting flushed!
 *
 * Derived classes need not override this if they instead provide
 * supports_streaming() and write_row(), below.
 */
int PNMFileTypeAndroid::Writer::
write_data(xel *array, xelval *alpha) {
  size_t num_pixels = (size_t)_x_size * (size_t)_y_size;

  Thread *current_thread = Thread::get_current_thread();
  JNIEnv *env = current_thread->get_jni_env();
  nassertr(env != nullptr, 0);

  // Create a Bitmap object.
  jobject bitmap =
    env->CallStaticObjectMethod(jni_PandaActivity,
                                jni_PandaActivity_createBitmap,
                                (jint)_x_size, (jint)_y_size,
                                BitmapConfig::ARGB_8888,
                                (jboolean)has_alpha());
  nassertr(bitmap != nullptr, 0);

  // Get a writable pointer to write our pixel data to.
  uint32_t *out;
  int rc = AndroidBitmap_lockPixels(env, bitmap, (void **)&out);
  if (rc != 0) {
    android_cat.error()
      << "Could not lock bitmap pixels (result code " << rc << ")\n";
    return 0;
  }

  if (_maxval == 255) {
    if (has_alpha() && alpha != nullptr) {
      for (size_t i = 0; i < num_pixels; ++i) {
        out[i] = (array[i].r)
               | (array[i].g << 8u)
               | (array[i].b << 16u)
               | (alpha[i] << 24u);
      }
    } else {
      for (size_t i = 0; i < num_pixels; ++i) {
        out[i] = (array[i].r)
               | (array[i].g << 8u)
               | (array[i].b << 16u)
               | 0xff000000u;
      }
    }
  } else {
    double ratio = 255.0 / _maxval;
    if (has_alpha() && alpha != nullptr) {
      for (size_t i = 0; i < num_pixels; ++i) {
        out[i] = ((uint32_t)(array[i].r * ratio))
               | ((uint32_t)(array[i].g * ratio) << 8u)
               | ((uint32_t)(array[i].b * ratio) << 16u)
               | ((uint32_t)(alpha[i] * ratio) << 24u);
      }
    } else {
      for (size_t i = 0; i < num_pixels; ++i) {
        out[i] = ((uint32_t)(array[i].r * ratio))
               | ((uint32_t)(array[i].g * ratio) << 8u)
               | ((uint32_t)(array[i].b * ratio) << 16u)
               | 0xff000000u;
      }
    }
  }

  // Finally, unlock the pixel data and compress it to the ostream.
  AndroidBitmap_unlockPixels(env, bitmap);
  jboolean res =
    env->CallStaticBooleanMethod(jni_PandaActivity,
                                 jni_PandaActivity_compressBitmap,
                                 bitmap, _format, 85, (jlong)_file);
  if (!res) {
    android_cat.error()
      << "Failed to compress bitmap.\n";
    return 0;
  }
  return _y_size;
}

/**
 * Returns true if this particular PNMWriter understands grayscale images.  If
 * this is false, then the rgb values of the xel array will be pre-filled with
 * the same value across all three channels, to allow the writer to simply
 * write out RGB data for a grayscale image.
 */
bool PNMFileTypeAndroid::Writer::
supports_grayscale() const {
  return false;
}

#endif  // ANDROID
