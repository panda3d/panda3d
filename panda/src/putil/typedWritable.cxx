// Filename: typedWritable.cxx
// Created by:  jason (08Jun00)
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

#include "typedWritable.h"
#include "bamWriter.h"
#include "lightMutexHolder.h"

LightMutex TypedWritable::_bam_writers_lock;

TypeHandle TypedWritable::_type_handle;
TypedWritable* const TypedWritable::Null = (TypedWritable*)0L;

////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
TypedWritable::
~TypedWritable() {
  // Remove the object pointer from the BamWriters that reference it.
  if (_bam_writers != (BamWriters *)NULL) {
    BamWriters temp;
    {
      LightMutexHolder holder(_bam_writers_lock);
      _bam_writers->swap(temp);
      delete _bam_writers;
      _bam_writers = NULL;
    }
    BamWriters::iterator wi;
    for (wi = temp.begin(); wi != temp.end(); ++wi) {
      BamWriter *writer = (*wi);
      writer->object_destructs(this);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void TypedWritable::
write_datagram(BamWriter *, Datagram &) {
}

////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
//
//               This is the callback function that is made by the
//               BamReader at some later point, after all of the
//               required pointers have been filled in.  It is
//               necessary because there might be forward references
//               in a bam file; when we call read_pointer() in
//               fillin(), the object may not have been read from the
//               file yet, so we do not have a pointer available at
//               that time.  Thus, instead of returning a pointer,
//               read_pointer() simply reserves a later callback.
//               This function provides that callback.  The calling
//               object is responsible for keeping track of the number
//               of times it called read_pointer() and extracting the
//               same number of pointers out of the supplied vector,
//               and storing them appropriately within the object.
////////////////////////////////////////////////////////////////////
int TypedWritable::
complete_pointers(TypedWritable **, BamReader *) {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::require_fully_complete
//       Access: Public, Virtual
//  Description: Some objects require all of their nested pointers to
//               have been completed before the objects themselves can
//               be completed.  If this is the case, override this
//               method to return true, and be careful with circular
//               references (which would make the object unreadable
//               from a bam file).
////////////////////////////////////////////////////////////////////
bool TypedWritable::
require_fully_complete() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::finalize
//       Access: Public, Virtual
//  Description: Called by the BamReader to perform any final actions
//               needed for setting up the object after all objects
//               have been read and all pointers have been completed.
////////////////////////////////////////////////////////////////////
void TypedWritable::
finalize(BamReader *) {
}


////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::fillin
//       Access: Protected
//  Description: This internal function is intended to be called by
//               each class's make_from_bam() method to read in all of
//               the relevant data from the BamFile for the new
//               object.
//
//               It is defined at the TypedWritable level so that
//               derived classes may call up the inheritance chain
//               from their own fillin() method.
////////////////////////////////////////////////////////////////////
void TypedWritable::
fillin(DatagramIterator &, BamReader *) {
}

