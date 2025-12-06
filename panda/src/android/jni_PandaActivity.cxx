/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file jni_PandaActivity.cxx
 * @author rdb
 * @date 2025-11-09
 */

#include <jni.h>
#include <sys/mman.h>
#include <unistd.h>

#if __GNUC__ >= 4
#define EXPORT_JNI extern "C" __attribute__((visibility("default")))
#else
#define EXPORT_JNI extern "C"
#endif

/**
 *
 */
EXPORT_JNI jlong
Java_org_panda3d_android_PandaActivity_nativeMmap(JNIEnv* env, jclass, jint fd, jlong off, jlong len) {
  // Align the offset down to the page size boundary.
  size_t page_size = getpagesize();
  off_t aligned = off & ~((off_t)page_size - 1);
  size_t delta = (size_t)(off - aligned);

  void *ptr = mmap(nullptr, (size_t)len + delta, PROT_READ, MAP_PRIVATE, fd, aligned);
  if (ptr != MAP_FAILED && ptr != nullptr) {
    return (jlong)((uintptr_t)ptr + delta);
  } else {
    return (jlong)0;
  }
}

/**
 * Calls the given function pointer, passing the given data pointer.
 */
EXPORT_JNI void
Java_org_panda3d_android_PandaActivity_nativeThreadEntry(JNIEnv* env, jobject self, jlong func, jlong data) {
  ((void (*)(void *))(void *)func)((void *)data);
}
