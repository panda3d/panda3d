// Filename: recorderTable.cxx
// Created by:  drose (27Jan04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "recorderTable.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "config_recorder.h"
#include "indent.h"

TypeHandle RecorderTable::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: RecorderTable::merge_from
//       Access: Public
//  Description: Combines the data in the current table (presumably
//               just read from disk, and matching exactly with the
//               disk data) with the data in the indicated table,
//               specified by the user (which may not exactly match
//               the disk data).
////////////////////////////////////////////////////////////////////
void RecorderTable::
merge_from(const RecorderTable &other) {
  Recorders::const_iterator ori;
  for (ori = other._recorders.begin(); ori != other._recorders.end(); ++ori) {
    const string &name = (*ori).first;
    RecorderBase *recorder = (*ori).second;

    Recorders::iterator ri = _recorders.find(name);
    if (ri == _recorders.end()) {
      // This may not be an error, since maybe the data isn't here
      // yet, but it'll be along later.
      recorder_cat.debug()
        << "No data for " << name << " in session.\n";

    } else if ((*ri).second->get_type() == recorder->get_type()) {
      // If we already had a recorder by that name with the same type,
      // throw it away (otherwise, keep the one we had before).
      (*ri).second = recorder;

    } else {
      recorder_cat.warning()
        << "Keeping recorder " << name << " of type " 
        << (*ri).second->get_type() << " instead of recorder of type " 
        << recorder->get_type() << "\n";
    }
  }

  // Now report any recorders in the session file that weren't
  // specified by the user.
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

////////////////////////////////////////////////////////////////////
//     Function: RecorderTable::add_recorder
//       Access: Published
//  Description: Adds the named recorder to the set of recorders.
////////////////////////////////////////////////////////////////////
void RecorderTable::
add_recorder(const string &name, RecorderBase *recorder) {
  _recorders[name] = recorder;
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderTable::get_recorder
//       Access: Published
//  Description: Returns the recorder with the indicated name, or NULL
//               if there is no such recorder.
////////////////////////////////////////////////////////////////////
RecorderBase *RecorderTable::
get_recorder(const string &name) const {
  Recorders::const_iterator ri = _recorders.find(name);
  if (ri != _recorders.end()) {
    return (*ri).second;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderTable::remove_recorder
//       Access: Published
//  Description: Removes the named recorder from the table.  Returns
//               true if successful, false if there was no such
//               recorder.
////////////////////////////////////////////////////////////////////
bool RecorderTable::
remove_recorder(const string &name) {
  Recorders::iterator ri = _recorders.find(name);
  if (ri != _recorders.end()) {
    _recorders.erase(ri);
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderTable::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void RecorderTable::
write(ostream &out, int indent_level) const {
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

////////////////////////////////////////////////////////////////////
//     Function: RecorderTable::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               Lens.
////////////////////////////////////////////////////////////////////
void RecorderTable::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderTable::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: RecorderTable::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type Lens is encountered
//               in the Bam file.  It should create the Lens
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *RecorderTable::
make_from_bam(const FactoryParams &params) {
  RecorderTable *table = new RecorderTable;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  table->fillin(scan, manager);

  return table;
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderTable::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new RecorderTable.
////////////////////////////////////////////////////////////////////
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

    PT(RecorderBase) recorder = 
      RecorderController::get_factory()->make_instance_more_general(type, fparams);
    if (recorder == (RecorderBase *)NULL) {
      recorder_cat.error()
        << "Unable to create Recorder of type " << type << "\n";
      _error = true;

    } else {
      bool inserted =
        _recorders.insert(Recorders::value_type(name, recorder)).second;
      nassertv(inserted);
    }
  }
}
