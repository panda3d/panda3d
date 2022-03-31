/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file jni_NativeOStream.cxx
 * @author rdb
 * @date 2018-02-10
 */

#include <jni.h>

#include <ostream>

#if __GNUC__ >= 4
#define EXPORT_JNI extern "C" __attribute__((visibility("default")))
#else
#define EXPORT_JNI extern "C"
#endif

/**
 * Flushes the stream.
 */
EXPORT_JNI void
Java_org_panda3d_android_NativeOStream_nativeFlush(JNIEnv *env, jclass clazz, jlong ptr) {
  std::ostream *stream = (std::ostream *)ptr;

  stream->flush();
}

/**
 * Writes a single character to the ostream.
 */
EXPORT_JNI void
Java_org_panda3d_android_NativeOStream_nativePut(JNIEnv *env, jclass clazz, jlong ptr, int b) {
  std::ostream *stream = (std::ostream *)ptr;

  stream->put((char)(b & 0xff));
}

/**
 * Writes an array of bytes to the ostream.
 */
EXPORT_JNI void
Java_org_panda3d_android_NativeOStream_nativeWrite(JNIEnv *env, jclass clazz, jlong ptr, jbyteArray byte_array, jint offset, jint length) {
  std::ostream *stream = (std::ostream *)ptr;

  jbyte *buffer = (jbyte *)alloca(length);
  env->GetByteArrayRegion(byte_array, offset, length, buffer);
  stream->write((char *)buffer, length);
}
