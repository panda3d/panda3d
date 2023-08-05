/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file threadDummyImpl.cxx
 * @author drose
 * @date 2002-08-09
 */

#include "selectThreadImpl.h"

#ifdef THREAD_DUMMY_IMPL

#include "threadDummyImpl.h"
#include "thread.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#endif

#ifdef ANDROID
#include "config_express.h"
#include <jni.h>

static JavaVM *java_vm = nullptr;
#endif

/**
 *
 */
std::string ThreadDummyImpl::
get_unique_id() const {
  // In a single-threaded application, this is just the unique process ID.
  std::ostringstream strm;
#ifdef _WIN32
  strm << GetCurrentProcessId();
#else
  strm << getpid();
#endif
  return strm.str();
}

/**
 *
 */
Thread *ThreadDummyImpl::
get_current_thread() {
  return Thread::get_main_thread();
}

#ifdef ANDROID
/**
 * Attaches the thread to the Java virtual machine.  If this returns true, a
 * JNIEnv pointer can be acquired using get_jni_env().
 */
bool ThreadDummyImpl::
attach_java_vm() {
  assert(java_vm != nullptr);
  JNIEnv *env;
  JavaVMAttachArgs args;
  args.version = JNI_VERSION_1_2;
  args.name = "Main";
  args.group = nullptr;
  if (java_vm->AttachCurrentThread(&env, &args) != 0) {
    thread_cat.error()
      << "Failed to attach Java VM to thread ";
      _jni_env = nullptr;
    return false;
  }
  _jni_env = env;
  return true;
}

/**
 * Binds the Panda thread to the current thread, assuming that the current
 * thread is already a valid attached Java thread.  Called by JNI_OnLoad.
 */
void ThreadDummyImpl::
bind_java_thread() {
}
#endif  // ANDROID

#endif  // THREAD_DUMMY_IMPL
