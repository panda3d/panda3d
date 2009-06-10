// Filename: typedWritable.I
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

////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TypedWritable::
TypedWritable() : _bam_writers(NULL) {
}

////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TypedWritable::
TypedWritable(const TypedWritable &) : _bam_writers(NULL) {
}

////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void TypedWritable::
operator = (const TypedWritable &) {
}

////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::mark_bam_modified
//       Access: Public
//  Description: Increments the bam_modified counter, so that this
//               object will be invalidated and retransmitted on any
//               open bam streams.  This should normally not need to
//               be called by user code; it should be called
//               internally when the object has been changed in a way
//               that legitimately requires its retransmission to any
//               connected clients.
////////////////////////////////////////////////////////////////////
INLINE void TypedWritable::
mark_bam_modified() {
  ++_bam_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: TypedWritable::get_bam_modified
//       Access: Public
//  Description: Returns the current bam_modified counter.  This
//               counter is normally incremented automatically
//               whenever the object is modified.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq TypedWritable::
get_bam_modified() const {
  return _bam_modified;
}
