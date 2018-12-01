/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mouseRecorder.cxx
 * @author drose
 * @date 2004-01-24
 */

#include "mouseRecorder.h"
#include "recorderController.h"
#include "dataNodeTransmit.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle MouseRecorder::_type_handle;

/**
 *
 */
MouseRecorder::
MouseRecorder(const std::string &name) :
  DataNode(name)
{
  _pixel_xy_input = define_input("pixel_xy", EventStoreVec2::get_class_type());
  _pixel_size_input = define_input("pixel_size", EventStoreVec2::get_class_type());
  _xy_input = define_input("xy", EventStoreVec2::get_class_type());
  _button_events_input = define_input("button_events", ButtonEventList::get_class_type());

  _pixel_xy_output = define_output("pixel_xy", EventStoreVec2::get_class_type());
  _pixel_size_output = define_output("pixel_size", EventStoreVec2::get_class_type());
  _xy_output = define_output("xy", EventStoreVec2::get_class_type());
  _button_events_output = define_output("button_events", ButtonEventList::get_class_type());

  _live_button_events = new ButtonEventList;
  _save_button_events = new ButtonEventList;

  _pixel_xy = new EventStoreVec2(LPoint2(0.0f, 0.0f));
  _xy = new EventStoreVec2(LPoint2(0.0f, 0.0f));
}

/**
 *
 */
MouseRecorder::
~MouseRecorder() {
}

/**
 * Records the most recent data collected into the indicated datagram, and
 * returns true if there is any interesting data worth recording, or false if
 * the datagram is meaningless.
 */
void MouseRecorder::
record_frame(BamWriter *manager, Datagram &dg) {
  nassertv(is_recording());
  dg.add_bool(_has_mouse);
  if (_has_mouse) {
    _mouse_xy.write_datagram(dg);
    _mouse_pixel_xy.write_datagram(dg);
  }
  _save_button_events->write_datagram(manager, dg);
  _save_button_events->clear();
}


/**
 * Reloads the most recent data collected from the indicated datagram.
 */
void MouseRecorder::
play_frame(DatagramIterator &scan, BamReader *manager) {
  nassertv(is_playing());
  _has_mouse = scan.get_bool();
  if (_has_mouse) {
    _mouse_xy.read_datagram(scan);
    _mouse_pixel_xy.read_datagram(scan);
  }
  ButtonEventList button_events;
  button_events.fillin(scan, manager);
  _save_button_events->add_events(button_events);
}

/**
 *
 */
void MouseRecorder::
output(std::ostream &out) const {
  DataNode::output(out);
}

/**
 *
 */
void MouseRecorder::
write(std::ostream &out, int indent_level) const {
  DataNode::write(out, indent_level);
}

/**
 * The virtual implementation of transmit_data().  This function receives an
 * array of input parameters and should generate an array of output
 * parameters.  The input parameters may be accessed with the index numbers
 * returned by the define_input() calls that were made earlier (presumably in
 * the constructor); likewise, the output parameters should be set with the
 * index numbers returned by the define_output() calls.
 */
void MouseRecorder::
do_transmit_data(DataGraphTraverser *, const DataNodeTransmit &input,
                 DataNodeTransmit &output) {
  bool has_mouse = false;
  LPoint2 mouse_xy;
  LPoint2 mouse_pixel_xy;

  _live_button_events->clear();

  if (is_playing()) {
    // If we're playing back data, copy in the data from a previous call to
    // play_frame().
    has_mouse = _has_mouse;
    mouse_xy = _mouse_xy;
    mouse_pixel_xy = _mouse_pixel_xy;
    _live_button_events->add_events(*_save_button_events);
    _save_button_events->clear();

  } else {
    // If we're not playing back data, query the data from the data graph

    if (input.has_data(_xy_input)) {
      // The mouse is within the window.  Get the current mouse position.
      const EventStoreVec2 *xy;
      DCAST_INTO_V(xy, input.get_data(_xy_input).get_ptr());
      mouse_xy = xy->get_value();
      DCAST_INTO_V(xy, input.get_data(_pixel_xy_input).get_ptr());
      mouse_pixel_xy = xy->get_value();
      has_mouse = true;
    }

    // Look for button events.
    if (input.has_data(_button_events_input)) {
      const ButtonEventList *button_events;
      DCAST_INTO_V(button_events, input.get_data(_button_events_input).get_ptr());
      _live_button_events->add_events(*button_events);
    }
  }

  // Now rebuild the output data for our children.

  if (has_mouse) {
    // Transmit the mouse position.
    _pixel_xy->set_value(_mouse_pixel_xy);
    _xy->set_value(_mouse_xy);
    output.set_data(_xy_output, EventParameter(_xy));
    output.set_data(_pixel_xy_output, EventParameter(_pixel_xy));
  }

  if (_live_button_events->get_num_events() != 0) {
    output.set_data(_button_events_output, EventParameter(_live_button_events));
  }

  if (is_recording()) {
    // Save data for the record.
    _has_mouse = has_mouse;
    _mouse_xy = mouse_xy;
    _mouse_pixel_xy = mouse_pixel_xy;
    _save_button_events->add_events(*_live_button_events);
  }

  // We always pass the pixel_size data through.
  output.set_data(_pixel_size_output, input.get_data(_pixel_size_input));
}

/**
 * Tells the BamReader how to create objects of type Lens.
 */
void MouseRecorder::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
  RecorderController::get_factory()->register_factory(get_class_type(), make_recorder);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void MouseRecorder::
write_datagram(BamWriter *manager, Datagram &dg) {
  DataNode::write_datagram(manager, dg);
}

/**
 * Writes the contents of this object to the datagram for encoding in the
 * session file.  This is very similar to write_datagram() for TypedWritable
 * objects, but it is used specifically to write the Recorder object when
 * generating the session file.  In many cases, it will be the same as
 * write_datagram().
 */
void MouseRecorder::
write_recorder(BamWriter *manager, Datagram &dg) {
  RecorderBase::write_recorder(manager, dg);
  DataNode::write_recorder(manager, dg);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type Lens is encountered in the Bam file.  It should create the Lens and
 * extract its information from the file.
 */
TypedWritable *MouseRecorder::
make_from_bam(const FactoryParams &params) {
  MouseRecorder *node = new MouseRecorder("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

/**
 * This is similar to make_from_bam(), but it is designed for loading the
 * RecorderBase object from the session log created by a RecorderController.
 */
RecorderBase *MouseRecorder::
make_recorder(const FactoryParams &params) {
  MouseRecorder *node = new MouseRecorder("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin_recorder(scan, manager);

  return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new MouseRecorder.
 */
void MouseRecorder::
fillin(DatagramIterator &scan, BamReader *manager) {
  DataNode::fillin(scan, manager);
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new MouseRecorder.
 */
void MouseRecorder::
fillin_recorder(DatagramIterator &scan, BamReader *manager) {
  RecorderBase::fillin_recorder(scan, manager);
  DataNode::fillin_recorder(scan, manager);
}
