/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file jni_NativeIStream.cxx
 * @author rdb
 * @date 2013-01-22
 */

#include <jni.h>

#include <istream>

#if __GNUC__ >= 4
#define EXPORT_JNI extern "C" __attribute__((visibility("default")))
#else
#define EXPORT_JNI extern "C"
#endif

/**
 * Reads a single character from the istream.  Should return -1 on EOF.
 */
EXPORT_JNI jint
Java_org_panda3d_android_NativeIStream_nativeGet(JNIEnv *env, jclass clazz, jlong ptr) {
  std::istream *stream = (std::istream *) ptr;

  int ch = stream->get();
  return stream->good() ? ch : -1;
}

/**
 * Reads an array of bytes from the istream.  Returns the actual number of
 * bytes that were read.  Should return -1 on EOF.
 */
EXPORT_JNI jint
Java_org_panda3d_android_NativeIStream_nativeRead(JNIEnv *env, jclass clazz, jlong ptr, jbyteArray byte_array, jint offset, jint length) {
  std::istream *stream = (std::istream *) ptr;
  jbyte *buffer = (jbyte *) env->GetPrimitiveArrayCritical(byte_array, nullptr);
  if (buffer == nullptr) {
    return -1;
  }

  stream->read((char*) buffer + offset, length);
  env->ReleasePrimitiveArrayCritical(byte_array, buffer, 0);

  // We have to return -1 on EOF, otherwise it will keep trying to read.
  size_t count = stream->gcount();
  if (count == 0 && stream->eof()) {
    return -1;
  } else {
    return count;
  }
}

/**
 * Skips ahead N bytes in the stream.  Returns the actual number of skipped
 * bytes.
 */
EXPORT_JNI jlong
Java_org_panda3d_android_NativeIStream_nativeIgnore(JNIEnv *env, jclass clazz, jlong ptr, jlong offset) {
  std::istream *stream = (std::istream *) ptr;
  stream->ignore(offset);
  return stream->gcount();
}
