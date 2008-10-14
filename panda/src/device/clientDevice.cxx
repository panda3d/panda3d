// Filename: clientDevice.cxx
// Created by:  drose (25Jan01)
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


#include "clientDevice.h"
#include "clientBase.h"

#include "indent.h"

TypeHandle ClientDevice::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ClientDevice::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
ClientDevice::
ClientDevice(ClientBase *client, TypeHandle device_type,
             const string &device_name) :
  _client(client),
  _device_type(device_type),
  _device_name(device_name)
{
  // We have to explicitly ref the client pointer, since we can't use
  // a PT(ClientBase) for circular include reasons.
  _client->ref();
  _is_connected = false;
}

////////////////////////////////////////////////////////////////////
//     Function: ClientDevice::Destructor
//       Access: Public, Virtual
//  Description: We don't actually call disconnect() at the
//               ClientDevice level destructor, because by the time
//               we get here we're already partly destructed.
//               Instead, we should call disconnect() from each
//               specific kind of derived class.
////////////////////////////////////////////////////////////////////
ClientDevice::
~ClientDevice() {
  nassertv(!_is_connected);

  // And now we explicitly unref the client pointer.
  unref_delete(_client);
}

////////////////////////////////////////////////////////////////////
//     Function: ClientDevice::disconnect
//       Access: Public
//  Description: Disconnects the ClientDevice from its ClientBase
//               object.  The device will stop receiving
//               updates.
//
//               Normally, you should not need to call this explicitly
//               (and it is probably a mistake to do so); it will
//               automatically be called when the ClientDevice object
//               destructs.
//
//               The lock should *not* be held while this call is
//               made; it will explicitly grab the lock itself.
////////////////////////////////////////////////////////////////////
void ClientDevice::
disconnect() {
  if (_is_connected) {
    acquire();
    bool disconnected =
      _client->disconnect_device(_device_type, _device_name, this);
    _is_connected = false;
    unlock();
    nassertv(disconnected);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClientDevice::poll
//       Access: Public
//  Description: Causes the connected ClientBase to poll all of its
//               clients, if necessary.  This will be a no-op if the
//               client is running in forked mode, or if it has
//               already polled everything this frame.
//
//               This should generally be called before accessing the
//               data in this ClientDevice to ensure that it is fresh.
////////////////////////////////////////////////////////////////////
void ClientDevice::
poll() {
  _client->poll();
}

////////////////////////////////////////////////////////////////////
//     Function: ClientDevice::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void ClientDevice::
output(ostream &out) const {
  out << get_type() << " " << get_device_name();
}

////////////////////////////////////////////////////////////////////
//     Function: ClientDevice::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void ClientDevice::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}
