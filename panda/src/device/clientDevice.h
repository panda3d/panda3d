// Filename: clientDevice.h
// Created by:  drose (25Jan01)
// 
////////////////////////////////////////////////////////////////////

#ifndef CLIENTDEVICE_H
#define CLIENTDEVICE_H

#include <pandabase.h>

#include <typedReferenceCount.h>

#ifdef HAVE_IPC
#include <ipc_mutex.h>
#endif

class ClientBase;

////////////////////////////////////////////////////////////////////
//       Class : ClientDevice
// Description : Any of a number of different devices that might be
//               attached to a ClientBase, including trackers, etc.
//               This is an abstract interface; the actual
//               implementations are in ClientTrackerDevice, etc.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ClientDevice : public TypedReferenceCount {
protected:
  ClientDevice(ClientBase *client, TypeHandle device_type, 
	       const string &device_name);

public:
  virtual ~ClientDevice();

  INLINE ClientBase *get_client() const;
  INLINE TypeHandle get_device_type() const;
  INLINE const string &get_device_name() const;

  INLINE bool is_connected() const;
  void disconnect();

  void poll();
  INLINE void lock();
  INLINE void unlock();

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

private:
  ClientBase *_client;
  TypeHandle _device_type;
  string _device_name;
  bool _is_connected;

#ifdef HAVE_IPC
  mutex _lock;
#endif

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "ClientDevice",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;

  friend class ClientBase;
};

INLINE ostream &operator <<(ostream &out, const ClientDevice &device) {
  device.output(out);
  return out;
}

#include "clientDevice.I"

#endif
