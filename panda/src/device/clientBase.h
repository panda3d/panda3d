/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file clientBase.h
 * @author jason
 * @date 2000-08-04
 */

#ifndef CLIENTBASE_H
#define CLIENTBASE_H

#include "pandabase.h"

#include "clientDevice.h"

#include "typedReferenceCount.h"
#include "luse.h"
#include "vector_string.h"
#include "vector_int.h"
#include "clockObject.h"
#include "pointerTo.h"
#include "coordinateSystem.h"

#ifdef OLD_HAVE_IPC
#include <ipc_thread.h>
#endif

#include "pmap.h"

/**
 * An abstract base class for a family of client device interfaces--including
 * trackers, buttons, dials, and other analog inputs.
 *
 * This provides a common interface to connect to such devices and extract
 * their data; it is used by TrackerNode etc.  to put these devices in the
 * data graph.
 */
class EXPCL_PANDA_DEVICE ClientBase : public TypedReferenceCount {
protected:
  ClientBase();

PUBLISHED:
  ~ClientBase();

  bool fork_asynchronous_thread(double poll_time);
  INLINE bool is_forked() const;
  INLINE bool poll();
  INLINE double get_last_poll_time() const;

  INLINE void set_coordinate_system(CoordinateSystem cs);
  INLINE CoordinateSystem get_coordinate_system() const;

public:
  PT(ClientDevice) get_device(TypeHandle device_type,
                              const std::string &device_name);

protected:
  virtual PT(ClientDevice) make_device(TypeHandle device_type,
                                       const std::string &device_name)=0;

  virtual bool disconnect_device(TypeHandle device_type,
                                 const std::string &device_name,
                                 ClientDevice *device);

  virtual void do_poll();

private:
  typedef pmap<std::string, ClientDevice *> DevicesByName;
  typedef pmap<TypeHandle, DevicesByName> Devices;
  Devices _devices;

  bool _forked;
  double _last_poll_time;
  int _last_poll_frame;
  CoordinateSystem _cs;

#ifdef OLD_HAVE_IPC
  int _sleep_time;
  thread *_client_thread;
  bool _shutdown;

  static void* st_callback(void *arg);
  void callback();
#endif


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "ClientBase",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class ClientDevice;
};

#include "clientBase.I"

#endif
