// Filename: asyncUtility.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef ASYNCUTILITY_H
#define ASYNCUTILITY_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>
#include <notify.h>
#include <typedef.h>

#ifdef HAVE_IPC
#include <ipc_mutex.h>
#include <ipc_condition.h>
#include <ipc_thread.h>
#endif


////////////////////////////////////////////////////////////////////
//       Class : AsyncUtility 
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS AsyncUtility {
public:
  AsyncUtility(float frequency = 0.2);
  virtual ~AsyncUtility(void);

  void create_thread(void);

  INLINE void set_frequency(float frequency);
  INLINE float get_frequency(void) const;

protected:
  void destroy_thread(void);
  static void* st_callback(void *arg);
  void callback(void);
  virtual bool process_request(void) = 0;
  void nap(void) const;

protected:
  int _next_token;
  bool _shutdown;
  bool _threaded;
  float _frequency;
  bool _threads_enabled;

#ifdef HAVE_IPC
  mutex _lock;
  condition_variable *_request_cond;
  thread *_thread;
#endif
};

#include "asyncUtility.I"

#endif
