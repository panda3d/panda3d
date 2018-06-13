/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file typedWritable.cxx
 * @author jason
 * @date 2000-06-08
 */

#include "typedWritable.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagramBuffer.h"
#include "lightMutexHolder.h"
#include "bam.h"

TypeHandle TypedWritable::_type_handle;
TypedWritable* const TypedWritable::Null = nullptr;

/**
 *
 */
TypedWritable::
~TypedWritable() {
  // Remove the object pointer from the BamWriters that reference it.
  BamWriterLink *link;
  do {
    link = (BamWriterLink *)AtomicAdjust::get_ptr(_bam_writers);
    if (link == nullptr) {
      // List is unlocked and empty - no writers to remove.
      return;
    }
    link = (BamWriterLink *)(((uintptr_t)link) & ~(uintptr_t)0x1);
  } while (link != AtomicAdjust::
    compare_and_exchange_ptr(_bam_writers, (void *)link, nullptr));

  while (link != nullptr) {
    BamWriterLink *next_link = link->_next;
    link->_writer->object_destructs(this);
    delete link;
    link = next_link;
  }
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void TypedWritable::
write_datagram(BamWriter *, Datagram &) {
}

/**
 * Called by the BamWriter when this object has not itself been modified
 * recently, but it should check its nested objects for updates.
 */
void TypedWritable::
update_bam_nested(BamWriter *) {
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 *
 * This is the callback function that is made by the BamReader at some later
 * point, after all of the required pointers have been filled in.  It is
 * necessary because there might be forward references in a bam file; when we
 * call read_pointer() in fillin(), the object may not have been read from the
 * file yet, so we do not have a pointer available at that time.  Thus,
 * instead of returning a pointer, read_pointer() simply reserves a later
 * callback.  This function provides that callback.  The calling object is
 * responsible for keeping track of the number of times it called
 * read_pointer() and extracting the same number of pointers out of the
 * supplied vector, and storing them appropriately within the object.
 */
int TypedWritable::
complete_pointers(TypedWritable **, BamReader *) {
  return 0;
}

/**
 * Some objects require all of their nested pointers to have been completed
 * before the objects themselves can be completed.  If this is the case,
 * override this method to return true, and be careful with circular
 * references (which would make the object unreadable from a bam file).
 */
bool TypedWritable::
require_fully_complete() const {
  return false;
}

/**
 * This internal function is intended to be called by each class's
 * make_from_bam() method to read in all of the relevant data from the BamFile
 * for the new object.  It is also called directly by the BamReader to re-read
 * the data for an object that has been placed on the stream for an update.
 */
void TypedWritable::
fillin(DatagramIterator &, BamReader *) {
}


/**
 * Called by the BamReader to perform any final actions needed for setting up
 * the object after all objects have been read and all pointers have been
 * completed.
 */
void TypedWritable::
finalize(BamReader *) {
}

/**
 * Returns the pointer cast to a ReferenceCount pointer, if it is in fact of
 * that type.
 */
ReferenceCount *TypedWritable::
as_reference_count() {
  return nullptr;
}

/**
 * Converts the TypedWritable object into a single stream of data using a
 * BamWriter, and stores that data in the indicated string.  Returns true on
 * success, false on failure.
 *
 * This is a convenience method particularly useful for cases when you are
 * only serializing a single object.  If you have many objects to process, it
 * is more efficient to use the same BamWriter to serialize all of them
 * together.
 */
bool TypedWritable::
encode_to_bam_stream(vector_uchar &data, BamWriter *writer) const {
  data.clear();

  DatagramBuffer buffer;
  if (writer == nullptr) {
    // Create our own writer.

    if (!buffer.write_header(_bam_header)) {
      return false;
    }

    BamWriter writer(&buffer);
    if (!writer.init()) {
      return false;
    }

    if (!writer.write_object(this)) {
      return false;
    }
  } else {
    // Use the existing writer.
    writer->set_target(&buffer);
    bool result = writer->write_object(this);
    writer->set_target(nullptr);
    if (!result) {
      return false;
    }
  }

  buffer.swap_data(data);
  return true;
}

/**
 * Reads the bytes created by a previous call to encode_to_bam_stream(), and
 * extracts the single object on those bytes.  Returns true on success, false
 * on error.
 *
 * This variant sets the TypedWritable and ReferenceCount pointers separately;
 * both are pointers to the same object.  The reference count is not
 * incremented; it is the caller's responsibility to manage the reference
 * count.
 *
 * Note that this method cannot be used to retrieve objects that do not
 * inherit from ReferenceCount, because these objects cannot persist beyond
 * the lifetime of the BamReader that reads them.  To retrieve these objects
 * from a bam stream, you must construct a BamReader directly.
 *
 * If you happen to know that the particular object in question inherits from
 * TypedWritableReferenceCount or PandaNode, consider calling the variant of
 * decode_from_bam_stream() defined for those methods, which presents a
 * simpler interface.
 */
bool TypedWritable::
decode_raw_from_bam_stream(TypedWritable *&ptr, ReferenceCount *&ref_ptr,
                           vector_uchar data, BamReader *reader) {

  DatagramBuffer buffer(std::move(data));

  if (reader == nullptr) {
    // Create a local reader.
    std::string head;
    if (!buffer.read_header(head, _bam_header.size())) {
      return false;
    }

    if (head != _bam_header) {
      return false;
    }

    BamReader reader(&buffer);
    if (!reader.init()) {
      return false;
    }

    if (!reader.read_object(ptr, ref_ptr)) {
      return false;
    }

    if (!reader.resolve()) {
      return false;
    }

    if (ref_ptr == nullptr) {
      // Can't support non-reference-counted objects.
      return false;
    }

    // Protect the pointer from accidental deletion when the BamReader goes
    // away.
    ref_ptr->ref();

  } else {
    // Use the existing reader.
    reader->set_source(&buffer);
    if (!reader->read_object(ptr, ref_ptr)) {
      reader->set_source(nullptr);
      return false;
    }

    if (!reader->resolve()) {
      reader->set_source(nullptr);
      return false;
    }

    if (ref_ptr == nullptr) {
      // Can't support non-reference-counted objects.
      reader->set_source(nullptr);
      return false;
    }

    // This BamReader isn't going away, but we have to balance the unref()
    // below.
    ref_ptr->ref();
    reader->set_source(nullptr);
  }


  // Now decrement the ref count, without deleting the object.  This may
  // reduce the reference count to zero, but that's OK--we trust the caller to
  // manage the reference count from this point on.
  ref_ptr->unref();
  return true;
}

/**
 * Called by the BamWriter to add itself to this TypedWritable's list of
 * BamWriters, so that it can receive notification whenever this object
 * destructs.  This method may be safely called from any thread.
 */
void TypedWritable::
add_bam_writer(BamWriter *writer) {
  nassertv(writer != nullptr);

  BamWriterLink *begin;
  BamWriterLink *new_link = new BamWriterLink;
  new_link->_writer = writer;

  // Assert that we got at least a 2-byte aligned pointer from new.
  nassertv(((uintptr_t)new_link & (uintptr_t)0x1) == 0);

  // This spins if the lower bit is 1, ie.  if the pointer is locked.
  do {
    begin = (BamWriterLink *)AtomicAdjust::get_ptr(_bam_writers);
    begin = (BamWriterLink *)(((uintptr_t)begin) & ~(uintptr_t)0x1);
    new_link->_next = begin;
  } while (begin != AtomicAdjust::
    compare_and_exchange_ptr(_bam_writers, (void *)begin, (void *)new_link));
}

/**
 * The converse of add_bam_writer.  This method may be safely called from any
 * thread.
 */
void TypedWritable::
remove_bam_writer(BamWriter *writer) {
  nassertv(writer != nullptr);

  BamWriterLink *begin;

  // Grab the head pointer and lock it in one atomic operation.  We lock it by
  // tagging the pointer.
  do {
    begin = (BamWriterLink *)AtomicAdjust::get_ptr(_bam_writers);
    begin = (BamWriterLink *)(((uintptr_t)begin) & ~(uintptr_t)0x1);
    if (begin == nullptr) {
      // The list is empty, nothing to remove.
      return;
    }
  } while (begin != AtomicAdjust::
    compare_and_exchange_ptr(_bam_writers, (void *)begin,
                       (void *)((uintptr_t)begin | (uintptr_t)0x1)));

  // Find the writer in the list.
  BamWriterLink *prev_link = nullptr;
  BamWriterLink *link = begin;

  while (link != nullptr && link->_writer != writer) {
    prev_link = link;
    link = link->_next;
  }

  if (link == nullptr) {
    // Not found.  Just unlock and leave.
    _bam_writers = (void *)begin;
    return;
  }

  if (prev_link == nullptr) {
    // It's the first link.  Replace and unlock in one atomic op.
    _bam_writers = (void *)link->_next;
  } else {
    prev_link->_next = link->_next;
    _bam_writers = (void *)begin; // Unlock
  }

  delete link;
}
