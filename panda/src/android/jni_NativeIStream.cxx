// Filename: jni_NativeIStream.cxx
// Created by:  rdb (22Jan13)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include <jni.h>

#include <istream>

////////////////////////////////////////////////////////////////////
//     Function: NativeIStream::nativeGet
//       Access: Private, Static
//  Description: Reads a single character from the istream.
//               Should return -1 on EOF.
////////////////////////////////////////////////////////////////////
extern "C" jint
Java_org_panda3d_android_NativeIStream_nativeGet(JNIEnv *env, jclass clazz, jlong ptr) {
  std::istream *stream = (std::istream *) ptr;

  int ch = stream->get();
  return stream->good() ? ch : -1;
}

////////////////////////////////////////////////////////////////////
//     Function: NativeIStream::nativeRead
//       Access: Private, Static
//  Description: Reads an array of bytes from the istream.  Returns
//               the actual number of bytes that were read.
//               Should return -1 on EOF.
////////////////////////////////////////////////////////////////////
extern "C" jint
Java_org_panda3d_android_NativeIStream_nativeRead(JNIEnv *env, jclass clazz, jlong ptr, jbyteArray byte_array, jint offset, jint length) {
  std::istream *stream = (std::istream *) ptr;
  jbyte *buffer = (jbyte *) env->GetPrimitiveArrayCritical(byte_array, NULL);
  if (buffer == NULL) {
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

////////////////////////////////////////////////////////////////////
//     Function: NativeIStream::nativeIgnore
//       Access: Private, Static
//  Description: Skips ahead N bytes in the stream.  Returns the
//               actual number of skipped bytes.
////////////////////////////////////////////////////////////////////
extern "C" jlong
Java_org_panda3d_android_NativeIStream_nativeIgnore(JNIEnv *env, jclass clazz, jlong ptr, jlong offset) {
  std::istream *stream = (std::istream *) ptr;
  stream->ignore(offset);
  return stream->gcount();
}
