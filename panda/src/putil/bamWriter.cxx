// Filename: bamWriter.cxx
// Created by:  jason (08Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"
#include "notify.h"

#include "typedWritable.h"
#include "config_util.h"
#include "bam.h"
#include "bamWriter.h"
#include "bamReader.h"

#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BamWriter::
BamWriter(DatagramSink *sink) :
  _target(sink)
{
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BamWriter::
~BamWriter() {
  // Tell all the TypedWritables whose pointer we are still keeping to
  // forget about us.
  StateMap::iterator si;
  for (si = _state_map.begin(); si != _state_map.end(); ++si) {
    TypedWritable *object = (TypedWritable *)(*si).first;
    TypedWritable::BamWriters::iterator wi = 
      find(object->_bam_writers.begin(), object->_bam_writers.end(), this);
    nassertv(wi != object->_bam_writers.end());
    object->_bam_writers.erase(wi);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::init
//       Access: Public
//  Description: Initializes the BamWriter prior to writing any
//               objects to its output stream.  This includes writing
//               out the Bam header.
//
//               This returns true if the BamWriter successfully
//               initialized, false otherwise.
////////////////////////////////////////////////////////////////////
bool BamWriter::
init() {
  // Initialize the next object and PTA ID's.  These start counting at
  // 1, since 0 is reserved for NULL.
  _next_object_id = 1;
  _long_object_id = false;
  _next_pta_id = 1;
  _long_pta_id = false;

  // Write out the current major and minor BAM file version numbers.
  Datagram header;

  header.add_uint16(_bam_major_ver);
  header.add_uint16(_bam_minor_ver);

  if (!_target->put_datagram(header)) {
    util_cat.error()
      << "Unable to write Bam header.\n";
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::write_object
//       Access: Public
//  Description: Writes a single object to the Bam file, so that the
//               BamReader::read_object() can later correctly restore
//               the object and all its pointers.
//
//               This implicitly also writes any additional objects
//               this object references (if they haven't already been
//               written), so that pointers may be fully resolved.
//
//               This may be called repeatedly to write a sequence of
//               objects to the Bam file, but typically (especially
//               for scene graph files, indicated with the .bam
//               extension), only one object is written directly from
//               the Bam file: the root of the scene graph.  The
//               remaining objects will all be written recursively by
//               the first object.
//
//               Returns true if the object is successfully written,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool BamWriter::
write_object(const TypedWritable *object) {
  nassertr(_object_queue.empty(), false);

  int object_id = enqueue_object(object);
  nassertr(object_id != 0, false);

  // If there are any freed objects to indicate, write them out now.
  if (!_freed_object_ids.empty()) {
    Datagram dg;
    write_handle(dg, BamReader::_remove_flag);

    FreedObjectIds::iterator fi;
    for (fi = _freed_object_ids.begin(); fi != _freed_object_ids.end(); ++fi) {
      write_object_id(dg, (*fi));
    }
    _freed_object_ids.clear();

    if (!_target->put_datagram(dg)) {
      util_cat.error()
        << "Unable to write data to output.\n";
      return false;
    }
  }

  // Now we write out all the objects in the queue, in order.  The
  // first one on the queue will, of course, be this object we just
  // queued up, but each object we write may append more to the queue.
  while (!_object_queue.empty()) {
    object = _object_queue.front();
    _object_queue.pop_front();

    // Look up the object in the map.  It had better be there!
    StateMap::iterator si = _state_map.find(object);
    nassertr(si != _state_map.end(), false);

    int object_id = (*si).second._object_id;
    bool already_written = (*si).second._written;

    Datagram dg;

    if (!already_written) {
      // The first time we write a particular object, we do so by
      // writing its TypeHandle (which had better not be
      // TypeHandle::none(), since that's our code for a
      // previously-written object), followed by the object ID number,
      // followed by the object definition.

      TypeHandle type = object->get_type();
      nassertr(type != TypeHandle::none(), false);

      // Determine what the nearest kind of type is that the reader
      // will be able to handle, and write that instead.
      TypeHandle registered_type = 
        BamReader::get_factory()->find_registered_type(type);
      if (registered_type == TypeHandle::none()) {
        // We won't be able to read this type again.
        util_cat.warning()
          << "Objects of type " << type << " cannot be read; bam file is invalid.\n";
      } else if (registered_type != type) {
        util_cat.info()
          << "Writing " << registered_type << " instead of " << type << "\n";
        type = registered_type;

      } else if (util_cat.is_debug()) {
        util_cat.debug()
          << "Writing " << type << " object id " << object_id
          << " to bam file\n";
      }

      write_handle(dg, type);
      write_object_id(dg, object_id);

      // We cast the const pointer to non-const so that we may call
      // write_datagram() on it.  Really, write_datagram() should be a
      // const method anyway, but there may be times when a class
      // object wants to update some transparent cache value during
      // writing or something like that, so it's more convenient to
      // cheat and define it as a non-const method.
      ((TypedWritable *)object)->write_datagram(this, dg);

      (*si).second._written = true;

    } else {
      // On subsequent times when we write a particular object, we
      // write simply TypeHandle::none(), followed by the object ID.
      // The occurrence of TypeHandle::none() is an indicator to the
      // BamReader that this is a previously-written object.

      write_handle(dg, TypeHandle::none());
      write_object_id(dg, object_id);
    }

    if (!_target->put_datagram(dg)) {
      util_cat.error()
        << "Unable to write data to output.\n";
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::has_object
//       Access: Public
//  Description: Returns true if the object has previously been
//               written (or at least requested to be written) to the
//               bam file, or false if we've never heard of it before.
////////////////////////////////////////////////////////////////////
bool BamWriter::
has_object(const TypedWritable *object) const {
  StateMap::const_iterator si = _state_map.find(object);
  return (si != _state_map.end());
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::write_pointer
//       Access: Public
//  Description: The interface for writing a pointer to another object
//               to a Bam file.  This is intended to be called by the
//               various objects that write themselves to the Bam
//               file, within the write_datagram() method.
//
//               This writes the pointer out in such a way that the
//               BamReader will be able to restore the pointer later.
//               If the pointer is to an object that has not yet
//               itself been written to the Bam file, that object will
//               automatically be written.
////////////////////////////////////////////////////////////////////
void BamWriter::
write_pointer(Datagram &packet, const TypedWritable *object) {
  // If the pointer is NULL, we always simply write a zero for an
  // object ID and leave it at that.
  if (object == (const TypedWritable *)NULL) {
    write_object_id(packet, 0);

  } else {
    StateMap::iterator si = _state_map.find(object);
    if (si == _state_map.end()) {
      // We have not written this pointer out yet.  This means we must
      // queue the object definition up for later.
      int object_id = enqueue_object(object);
      write_object_id(packet, object_id);

    } else {
      // We have already assigned this pointer an ID; thus, we can
      // simply write out the ID.
      write_object_id(packet, (*si).second._object_id);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::write_cdata
//       Access: Public
//  Description: Writes out the indicated CycleData object.  This
//               should be used by classes that store some or all of
//               their data within a CycleData subclass, in support of
//               pipelining.  This will call the virtual
//               CycleData::write_datagram() method to do the actual
//               writing.
////////////////////////////////////////////////////////////////////
void BamWriter::
write_cdata(Datagram &packet, const PipelineCyclerBase &cycler) {
  const CycleData *cdata = cycler.read();
  cdata->write_datagram(this, packet);
  cycler.release_read(cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::register_pta
//       Access: Public
//  Description: Prepares to write a PointerToArray to the Bam file,
//               unifying references to the same pointer across the
//               Bam file.
//
//               The writing object should call this prior to writing
//               out a PointerToArray.  It will return true if the
//               same pointer has been written previously, in which
//               case the writing object need do nothing further; or
//               it will return false if this particular pointer has
//               not yet been written, in which case the writing
//               object must then write out the contents of the array.
//
//               Also see the WRITE_PTA() macro, which consolidates
//               the work that must be done to write a PTA.
////////////////////////////////////////////////////////////////////
bool BamWriter::
register_pta(Datagram &packet, const void *ptr) {
  if (ptr == (const void *)NULL) {
    // A zero for the PTA ID indicates a NULL pointer.  This is a
    // special case.
    write_pta_id(packet, 0);

    // We return false to indicate the user must now write out the
    // "definition" of the NULL pointer.  This is necessary because of
    // a quirk in the BamReader's design, which forces callers to read
    // the definition of every NULL pointer.  Presumably, the caller
    // will be able to write the definition in a concise way that will
    // clearly indicate a NULL pointer; in the case of a
    // PointerToArray, this will generally be simply a zero element
    // count.
    return false;
  }

  PTAMap::iterator pi = _pta_map.find(ptr);
  if (pi == _pta_map.end()) {
    // We have not encountered this pointer before.
    int pta_id = _next_pta_id;
    _next_pta_id++;

    bool inserted = _pta_map.insert(PTAMap::value_type(ptr, pta_id)).second;
    nassertr(inserted, false);

    write_pta_id(packet, pta_id);

    // Return false to indicate the caller must now write out the
    // array definition.
    return false;

  } else {
    // We have encountered this pointer before.
    int pta_id = (*pi).second;
    write_pta_id(packet, pta_id);

    // Return true to indicate the caller need do nothing further.
    return true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::write_handle
//       Access: Public
//  Description: Writes a TypeHandle to the file in such a way that
//               the BamReader can read the same TypeHandle later via
//               read_handle().
////////////////////////////////////////////////////////////////////
void BamWriter::
write_handle(Datagram &packet, TypeHandle type) {
  // We encode TypeHandles within the Bam file by writing a unique
  // index number for each one to the file.  When we write a
  // particular TypeHandle for the first time, we assign it a new
  // index number and then immediately follow it by its definition;
  // when we write the same TypeHandle on subsequent times we only
  // write the index number.

  // The unique number we choose is actually the internal index number
  // of the TypeHandle.  Why not?
  int index = type.get_index();

  // Also make sure the index number fits within a PN_uint16.
  nassertv(index <= 0xffff);

  packet.add_uint16(index);

  if (index != 0) {
    bool inserted = _types_written.insert(index).second;

    if (inserted) {
      // This is the first time this TypeHandle has been written, so
      // also write out its definition.
      packet.add_string(type.get_name());

      // We also need to write the derivation of the TypeHandle, in case
      // the program reading this file later has never heard of this
      // type before.
      int num_parent_classes = type.get_num_parent_classes();
      nassertv(num_parent_classes <= 255);  // Good grief!
      packet.add_uint8(num_parent_classes);
      for (int i = 0; i < num_parent_classes; i++) {
        write_handle(packet, type.get_parent_class(i));
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::object_destructs
//       Access: Private
//  Description: This is called by the TypedWritable destructor.  It
//               should remove the pointer from any structures that
//               keep a reference to it, and also write a flag to the
//               bam file (if it is open) so that a reader will know
//               the object id will no longer be used.
////////////////////////////////////////////////////////////////////
void BamWriter::
object_destructs(TypedWritable *object) {
  StateMap::iterator si = _state_map.find(object);
  if (si != _state_map.end()) {
    // We ought to have written out the object by the time it
    // destructs, or we're in trouble when we do write it out.
    nassertv((*si).second._written);

    int object_id = (*si).second._object_id;
    _freed_object_ids.push_back(object_id);

    _state_map.erase(si);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::write_object_id
//       Access: Private
//  Description: Writes the indicated object id to the datagram.
////////////////////////////////////////////////////////////////////
void BamWriter::
write_object_id(Datagram &dg, int object_id) {
  if (_long_object_id) {
    dg.add_uint32(object_id);
    
  } else {
    dg.add_uint16(object_id);
    // Once we fill up our uint16, we write all object id's
    // thereafter with a uint32.
    if (object_id == 0xffff) {
      _long_object_id = true;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::write_pta_id
//       Access: Private
//  Description: Writes the indicated pta id to the datagram.
////////////////////////////////////////////////////////////////////
void BamWriter::
write_pta_id(Datagram &dg, int pta_id) {
  if (_long_pta_id) {
    dg.add_uint32(pta_id);
    
  } else {
    dg.add_uint16(pta_id);
    // Once we fill up our uint16, we write all pta id's
    // thereafter with a uint32.
    if (pta_id == 0xffff) {
      _long_pta_id = true;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::enqueue_object
//       Access: Private
//  Description: Assigns an object ID to the object and queues it up
//               for later writing to the Bam file.
//
//               The return value is the object ID, or 0 if there is
//               an error.
////////////////////////////////////////////////////////////////////
int BamWriter::
enqueue_object(const TypedWritable *object) {
  Datagram dg;

  nassertr(object != TypedWritable::Null, 0);

  // No object should ever be written out that is not registered as a
  // child of TypedWritable.  The only way this can happen is if
  // someone failed to initialize their type correctly in init_type().
#ifndef NDEBUG
  if (!object->is_of_type(TypedWritable::get_class_type())) {
    util_cat.error()
      << "Type " << object->get_type() 
      << " does not indicate inheritance from TypedWritable.\n"
      << "(this is almost certainly an oversight in " << object->get_type()
      << "::init_type().)\n";
  }
#endif

  // We need to assign a unique index number to every object we write
  // out.  Has this object been assigned a number yet?
  int object_id;

  StateMap::iterator si = _state_map.find(object);
  if (si == _state_map.end()) {
    // No, it hasn't, so assign it the next number in sequence
    // arbitrarily.
    object_id = _next_object_id;

    bool inserted =
      _state_map.insert(StateMap::value_type(object, StoreState(_next_object_id))).second;
    nassertr(inserted, false);
    ((TypedWritable *)object)->_bam_writers.push_back(this);
    _next_object_id++;

  } else {
    // Yes, it has; get the object ID.
    object_id = (*si).second._object_id;
  }

  _object_queue.push_back(object);
  return object_id;
}
