/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file threadPosixImpl.cxx
 * @author drose
 * @date 2006-02-09
 */

#include "threadPosixImpl.h"
#include "selectThreadImpl.h"

#ifdef THREAD_POSIX_IMPL

#include "thread.h"
#include "pointerTo.h"
#include "config_pipeline.h"
#include <sched.h>

// Used for getrusage().
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

#ifdef ANDROID
#include "config_express.h"
#include <jni.h>

static JavaVM *java_vm = nullptr;
#endif

__thread Thread *ThreadPosixImpl::_current_thread = nullptr;
static patomic_flag _main_thread_known = ATOMIC_FLAG_INIT;

/**
 *
 */
ThreadPosixImpl::
~ThreadPosixImpl() {
  if (thread_cat->is_debug()) {
    thread_cat.debug()
      << "Deleting thread " << _parent_obj->get_name() << "\n";
  }

  _mutex.lock();

  if (!_detached) {
    pthread_detach(_thread);
    _detached = true;
  }

  _mutex.unlock();
}

/**
 * Called for the main thread only, which has been already started, to fill in
 * the values appropriate to that thread.
 */
void ThreadPosixImpl::
setup_main_thread() {
  _status = S_running;
  _thread = pthread_self();
}

/**
 *
 */
bool ThreadPosixImpl::
start(ThreadPriority priority, bool joinable) {
  _mutex.lock();
  if (thread_cat->is_debug()) {
    thread_cat.debug() << "Starting " << *_parent_obj << "\n";
  }

  nassertd(_status == S_new) {
    _mutex.unlock();
    return false;
  }

  _joinable = joinable;
  _status = S_start_called;
  _detached = false;

  pthread_attr_t attr;
  pthread_attr_init(&attr);

  if (!_joinable) {
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    _detached = true;
  }

  int result = pthread_attr_setstacksize(&attr, thread_stack_size);
  if (result != 0) {
    thread_cat->warning()
      << "Unable to set stack size.\n";
  }

  // Ensure the thread has "system" scope, which should ensure it can run in
  // parallel with other threads.
  result = pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
  if (result != 0) {
    thread_cat->warning()
      << "Unable to set system scope.\n";
  }

  struct sched_param param;
  int current_policy = SCHED_OTHER;
  result = pthread_attr_setschedpolicy(&attr, current_policy);
  if (result != 0) {
    thread_cat->warning()
      << "Unable to set scheduling policy.\n";

  }

  result = 0;
  switch (priority) {
  case TP_low:
    param.sched_priority = sched_get_priority_min(current_policy);
    result = pthread_attr_setschedparam(&attr, &param);
    break;

  case TP_high:
  case TP_urgent:
    param.sched_priority = sched_get_priority_max(current_policy);
    result = pthread_attr_setschedparam(&attr, &param);
    break;

  case TP_normal:
  default:
    break;
  }

  if (result != 0) {
    thread_cat->warning()
      << "Unable to specify thread priority.\n";
  }

  // Increment the parent object's reference count first.  The thread will
  // eventually decrement it when it terminates.
  _parent_obj->ref();
  result = pthread_create(&_thread, &attr, &root_func, (void *)this);

  pthread_attr_destroy(&attr);

  if (result != 0) {
    // Oops, we couldn't start the thread.  Be sure to decrement the reference
    // count we incremented above, and return false to indicate failure.
    unref_delete(_parent_obj);
    _mutex.unlock();
    return false;
  }

  // Thread was successfully started.
  _mutex.unlock();
  return true;
}

/**
 * Blocks the calling process until the thread terminates.  If the thread has
 * already terminated, this returns immediately.
 */
void ThreadPosixImpl::
join() {
  _mutex.lock();
  if (!_detached) {
    _mutex.unlock();
    void *return_val;
    pthread_join(_thread, &return_val);
    _detached = true;
    return;
  }
  _mutex.unlock();
}

/**
 *
 */
std::string ThreadPosixImpl::
get_unique_id() const {
  std::ostringstream strm;
  strm << getpid() << "." << (uintptr_t)_thread;

  return strm.str();
}

/**
 * Associates the indicated Thread object with the currently-executing thread.
 * You should not call this directly; use Thread::bind_thread() instead.
 */
void ThreadPosixImpl::
bind_thread(Thread *thread) {
  if (_current_thread == nullptr && thread == Thread::get_main_thread()) {
    _main_thread_known.test_and_set(std::memory_order_relaxed);
  }
  _current_thread = thread;
#ifdef ANDROID
  bind_java_thread();
#endif
}

#ifdef ANDROID
/**
 * Attaches the thread to the Java virtual machine.  If this returns true, a
 * JNIEnv pointer can be acquired using get_jni_env().
 */
bool ThreadPosixImpl::
attach_java_vm() {
  assert(java_vm != nullptr);

  JNIEnv *env;
  std::string thread_name = _parent_obj->get_name();
  JavaVMAttachArgs args;
  args.version = JNI_VERSION_1_2;
  args.name = thread_name.c_str();
  args.group = nullptr;
  if (java_vm->AttachCurrentThread(&env, &args) != 0) {
    thread_cat.error()
      << "Failed to attach Java VM to thread "
      << _parent_obj->get_name() << "!\n";
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
void ThreadPosixImpl::
bind_java_thread() {
  Thread *thread = Thread::get_current_thread();
  nassertv(thread != nullptr);

  assert(java_vm != nullptr);

  // Get the JNIEnv for this Java thread, and store it on the corresponding
  // Panda thread object.
  JNIEnv *env;
  if (java_vm->GetEnv((void **)&env, JNI_VERSION_1_4) == JNI_OK) {
    nassertv(thread->_impl._jni_env == nullptr || thread->_impl._jni_env == env);
    thread->_impl._jni_env = env;
  } else {
    thread_cat->error()
      << "Called bind_java_thread() on thread "
      << *thread << ", which is not attached to Java VM!\n";
  }
}
#endif  // ANDROID

/**
 * Returns the number of context switches that occurred on the current thread.
 * The first number is the total number of context switches reported by the OS,
 * and the second number is the number of involuntary context switches (ie. the
 * thread was scheduled out by the OS), if known, otherwise zero.
 * Returns true if context switch information was available, false otherwise.
 */
bool ThreadPosixImpl::
get_context_switches(size_t &total, size_t &involuntary) {
#ifdef RUSAGE_THREAD
  struct rusage usage;
  if (getrusage(RUSAGE_THREAD, &usage) == 0) {
    total = (size_t)usage.ru_nvcsw;
    involuntary = (size_t)usage.ru_nivcsw;
    return true;
  }
#endif
  return false;
}

/**
 * The entry point of each thread.
 */
void *ThreadPosixImpl::
root_func(void *data) {
  TAU_REGISTER_THREAD();
  {
    // TAU_PROFILE("void ThreadPosixImpl::root_func()", " ", TAU_USER);

    ThreadPosixImpl *self = (ThreadPosixImpl *)data;
    _current_thread = self->_parent_obj;

    {
      self->_mutex.lock();
      nassertd(self->_status == S_start_called) {
        self->_mutex.unlock();
        return nullptr;
      }

      self->_status = S_running;
      self->_mutex.unlock();
    }

#ifdef ANDROID
    // Attach the Java VM to allow calling Java functions in this thread.
    self->attach_java_vm();
#endif

    self->_parent_obj->thread_main();

    if (thread_cat->is_debug()) {
      thread_cat.debug()
        << "Terminating thread " << self->_parent_obj->get_name()
        << ", count = " << self->_parent_obj->get_ref_count() << "\n";
    }

    {
      self->_mutex.lock();
      nassertd(self->_status == S_running) {
        self->_mutex.unlock();
        return nullptr;
      }
      self->_status = S_finished;
      self->_mutex.unlock();
    }

#ifdef ANDROID
    // We cannot let the thread end without detaching it.
    if (self->_jni_env != nullptr) {
      java_vm->DetachCurrentThread();
      self->_jni_env = nullptr;
    }
#endif

    // Now drop the parent object reference that we grabbed in start(). This
    // might delete the parent object, and in turn, delete the ThreadPosixImpl
    // object.
    unref_delete(self->_parent_obj);
  }

  return nullptr;
}

/**
 * Called by get_current_thread() if the current therad pointer is null; checks
 * whether it might be the main thread.
 */
Thread *ThreadPosixImpl::
init_current_thread() {
  Thread *thread = _current_thread;
  if (!_main_thread_known.test_and_set(std::memory_order_relaxed)) {
    thread = Thread::get_main_thread();
    _current_thread = thread;
  }
  nassertr(thread != nullptr, nullptr);
  return thread;
}

#ifdef ANDROID
/**
 * Called by Java when loading this library from the Java virtual machine.
 */
jint JNI_OnLoad(JavaVM *jvm, void *reserved) {
  // Store the JVM pointer globally.
  java_vm = jvm;

  ThreadPosixImpl::bind_java_thread();
  return JNI_VERSION_1_4;
}
#endif  // ANDROID

#endif  // THREAD_POSIX_IMPL
