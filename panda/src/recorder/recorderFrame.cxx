/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file recorderFrame.cxx
 * @author drose
 * @date 2004-01-28
 */

#include "recorderFrame.h"
#include "recorderTable.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "config_recorder.h"

TypeHandle RecorderFrame::_type_handle;

/**
 * Once the raw data has been read in from the session file, and the table has
 * been decoded, decode the raw data and call play_frame on each recorder.
 */
void RecorderFrame::
play_frame(BamReader *manager) {
  DatagramIterator scan(_data, _data_pos);
  _table->play_frame(scan, manager);

  // We expect to use up all of the data in the datagram.
  nassertv(scan.get_remaining_size() == 0);
}

/**
 * Tells the BamReader how to create objects of type Lens.
 */
void RecorderFrame::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void RecorderFrame::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);
  dg.add_float64(_timestamp);
  dg.add_uint32(_frame);

  // Write the table out if it has changed.
  dg.add_bool(_table_changed);
  if (_table_changed) {
    // As a kludge, we create a new table pointer to write out.  Otherwise,
    // the pointer won't change and it may not write out the changes.  Later,
    // we need to add a facility to the bam writer to detect when a
    // TypedWritable has changed and should be rewritten.
    _local_table = *_table;
    manager->write_pointer(dg, &_local_table);
  }

  _table->record_frame(manager, dg);
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
int RecorderFrame::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritable::complete_pointers(p_list, manager);

  if (_table_changed) {
    _table = DCAST(RecorderTable, p_list[pi++]);
  }

  return pi;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type Lens is encountered in the Bam file.  It should create the Lens and
 * extract its information from the file.
 */
TypedWritable *RecorderFrame::
make_from_bam(const FactoryParams &params) {
  RecorderFrame *frame = new RecorderFrame;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  frame->fillin(scan, manager);

  return frame;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new RecorderFrame.
 */
void RecorderFrame::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  _timestamp = scan.get_float64();
  _frame = scan.get_uint32();
  _table_changed = scan.get_bool();
  _table = nullptr;
  if (_table_changed) {
    manager->read_pointer(scan);
  }

  // We can't decode the data in the frame until we have (a) gotten back the
  // table pointer, or (b) been told who our owning RecorderController is.  So
  // we'll just save the raw data for now and come back to it.
  _data = scan.get_datagram();
  _data_pos = scan.get_current_index();
}
