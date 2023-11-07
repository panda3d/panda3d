/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file recorderTable.cxx
 * @author drose
 * @date 2004-01-27
 */

#include "recorderTable.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "config_recorder.h"
#include "recorderController.h"
#include "indent.h"

using std::string;

TypeHandle RecorderTable::_type_handle;

/**
 *
 */
RecorderTable::
~RecorderTable() {
  Recorders::iterator ri;
  for (ri = _recorders.begin(); ri != _recorders.end(); ++ri) {
    unref_delete(ri->second);
  }
}

/**
 * Combines the data in the current table (presumably just read from disk, and
 * matching exactly with the disk data) with the data in the indicated table,
 * specified by the user (which may not exactly match the disk data).
 */
void RecorderTable::
merge_from(const RecorderTable &other) {
  Recorders::const_iterator ori;
  for (ori = other._recorders.begin(); ori != other._recorders.end(); ++ori) {
    const string &name = (*ori).first;
    RecorderBase *recorder = (*ori).second;

    Recorders::iterator ri = _recorders.find(name);
    if (ri == _recorders.end()) {
      // This may not be an error, since maybe the data isn't here yet, but
      // it'll be along later.
      recorder_cat.debug()
        << "No data for " << name << " in session.\n";

    } else if ((*ri).second->get_type() == recorder->get_type()) {
      // If we already had a recorder by that name with the same type, throw
      // it away (otherwise, keep the one we had before).
      if ((*ri).second != recorder) {
        recorder->ref();
        unref_delete((*ri).second);
        (*ri).second = recorder;
      }

    } else {
      recorder_cat.warning()
        << "Keeping recorder " << name << " of type "
        << (*ri).second->get_type() << " instead of recorder of type "
        << recorder->get_type() << "\n";
    }
  }

  // Now report any recorders in the session file that weren't specified by
  // the user.
  Recorders::const_iterator ri;
  for (ri = _recorders.begin(); ri != _recorders.end(); ++ri) {
    const string &name = (*ri).first;
    ori = other._recorders.find(name);
    if (ori == other._recorders.end()) {
      recorder_cat.warning()
        << "Ignoring " << name << " in session.\n";
    }
  }
}

/**
 * Calls record_frame on all recorders.
 */
void RecorderTable::
record_frame(BamWriter *manager, Datagram &dg) {
  Recorders::iterator ri;
  for (ri = _recorders.begin();
       ri != _recorders.end();
       ++ri) {
    RecorderBase *recorder = (*ri).second;
    recorder->record_frame(manager, dg);
  }
}

/**
 * Calls play_frame on all recorders.
 */
void RecorderTable::
play_frame(DatagramIterator &scan, BamReader *manager) {
  Recorders::iterator ri;
  for (ri = _recorders.begin();
       ri != _recorders.end();
       ++ri) {
    RecorderBase *recorder = (*ri).second;
    recorder->play_frame(scan, manager);
  }
}

/**
 * Sets the given flags on all recorders.
 */
void RecorderTable::
set_flags(short flags) {
  Recorders::iterator ri;
  for (ri = _recorders.begin();
       ri != _recorders.end();
       ++ri) {
    RecorderBase *recorder = (*ri).second;
    recorder->_flags |= flags;
  }
}

/**
 * Clears the given flags on all recorders.
 */
void RecorderTable::
clear_flags(short flags) {
  Recorders::iterator ri;
  for (ri = _recorders.begin();
       ri != _recorders.end();
       ++ri) {
    RecorderBase *recorder = (*ri).second;
    recorder->_flags &= ~flags;
  }
}

/**
 *
 */
void RecorderTable::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "RecorderTable:\n";

  Recorders::const_iterator ri;
  for (ri = _recorders.begin(); ri != _recorders.end(); ++ri) {
    const string &name = (*ri).first;
    RecorderBase *recorder = (*ri).second;
    indent(out, indent_level + 2)
      << name << " : " << recorder->get_type() << "\n";
  }
}

/**
 * Tells the BamReader how to create objects of type Lens.
 */
void RecorderTable::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void RecorderTable::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);

  dg.add_uint16(_recorders.size());
  Recorders::const_iterator ri;
  for (ri = _recorders.begin(); ri != _recorders.end(); ++ri) {
    const string &name = (*ri).first;
    RecorderBase *recorder = (*ri).second;
    dg.add_string(name);
    manager->write_handle(dg, recorder->get_type());
    recorder->write_recorder(manager, dg);
  }
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type Lens is encountered in the Bam file.  It should create the Lens and
 * extract its information from the file.
 */
TypedWritable *RecorderTable::
make_from_bam(const FactoryParams &params) {
  RecorderTable *table = new RecorderTable;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  table->fillin(scan, manager);

  return table;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new RecorderTable.
 */
void RecorderTable::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  int num_recorders = scan.get_uint16();
  for (int i = 0; i < num_recorders; i++) {
    string name = scan.get_string();
    TypeHandle type = manager->read_handle(scan);

    // Use the Factory to create a new recorder of the indicated type.
    FactoryParams fparams;
    fparams.add_param(new BamReaderParam(scan, manager));

    RecorderBase *recorder =
      RecorderController::get_factory()->make_instance_more_general(type, fparams);
    if (recorder == nullptr) {
      recorder_cat.error()
        << "Unable to create Recorder of type " << type << "\n";
      _error = true;

    } else {
      recorder->ref();
      bool inserted =
        _recorders.insert(Recorders::value_type(name, recorder)).second;
      nassertv(inserted);
    }
  }
}
