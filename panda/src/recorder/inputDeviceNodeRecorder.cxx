/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file InputDeviceNodeRecorder.cxx
 * @author jb
 * @date 2023-10-10
 */

#include "inputDeviceNodeRecorder.h"
#include "recorderController.h"
#include "dataNodeTransmit.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "inputDevice.h"

TypeHandle InputDeviceNodeRecorder::_type_handle;

/**
 *
 */
InputDeviceNodeRecorder::
InputDeviceNodeRecorder(const std::string &name, InputDevice *device) :
  DataNode(name),
  _device(device)
{
  _button_events_input = define_input("button_events", ButtonEventList::get_class_type());
  _axis_events_input = define_input("axis_events", AxisEventList::get_class_type());

  _button_events_output = define_output("button_events", ButtonEventList::get_class_type());

  _live_button_events = new ButtonEventList;
  _save_button_events = new ButtonEventList;
  _live_axis_events = new AxisEventList;
  _save_axis_events = new AxisEventList;
}

/**
 *
 */
InputDeviceNodeRecorder::
~InputDeviceNodeRecorder() {
}

/**
 * Records the most recent data collected into the indicated datagram, and
 * returns true if there is any interesting data worth recording, or false if
 * the datagram is meaningless.
 */
void InputDeviceNodeRecorder::
record_frame(BamWriter *manager, Datagram &dg) {
  nassertv(is_recording());
  _save_button_events->write_datagram(manager, dg);
  _save_button_events->clear();
  _save_axis_events->write_datagram(manager, dg);
  _save_axis_events->clear();
}


/**
 * Reloads the most recent data collected from the indicated datagram.
 */
void InputDeviceNodeRecorder::
play_frame(DatagramIterator &scan, BamReader *manager) {
  nassertv(is_playing());
  ButtonEventList button_events;
  button_events.fillin(scan, manager);
  _save_button_events->add_events(button_events);
  AxisEventList axis_events;
  axis_events.fillin(scan, manager);
  _save_axis_events->add_events(axis_events);
}

/**
 *
 */
void InputDeviceNodeRecorder::
output(std::ostream &out) const {
  DataNode::output(out);
}

/**
 *
 */
void InputDeviceNodeRecorder::
write(std::ostream& out, int indent_level) const {
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
void InputDeviceNodeRecorder::
do_transmit_data(DataGraphTraverser *, const DataNodeTransmit &input,
                 DataNodeTransmit &output) {
  _live_button_events->clear();
  _live_axis_events->clear();

  if (is_playing()) {
    // If we're playing back data, copy in the data from a previous call to
    // play_frame().
    _live_button_events->add_events(*_save_button_events);
    _save_button_events->clear();

    // Disable updates of axis values from the connected InputDevice
    _device->enable_axis_updates(false);

    // Set the recorded axis values on the InputDevice
    for (int i = 0; i < _save_axis_events->get_num_events(); i++) {
      AxisEvent ae = _save_axis_events->get_event(i);
      _device->set_axis_value(ae.index, ae.value);
    }
    _save_axis_events->clear();

  } else {
    // If we're not playing back data, query the data from the data graph

    // Look for button events.
    if (input.has_data(_button_events_input)) {
      const ButtonEventList *button_events;
      DCAST_INTO_V(button_events, input.get_data(_button_events_input).get_ptr());
      _live_button_events->add_events(*button_events);
    }

    // Enable updates af axis values from the connected InputDevice
    _device->enable_axis_updates(true);
    // Look for axis events.
    if (input.has_data(_axis_events_input)) {
      const AxisEventList *axis_events;
      DCAST_INTO_V(axis_events, input.get_data(_axis_events_input).get_ptr());
      _live_axis_events->add_events(*axis_events);
    }
  }

  // Now rebuild the output data for our children.

  if (_live_button_events->get_num_events() != 0) {
    output.set_data(_button_events_output, EventParameter(_live_button_events));
  }

  if (is_recording()) {
    // Save data for the record.
    _save_button_events->add_events(*_live_button_events);
    _save_axis_events->add_events(*_live_axis_events);
  }
}

/**
 * Tells the BamReader how to create objects of type Lens.
 */
void InputDeviceNodeRecorder::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
  RecorderController::get_factory()->register_factory(get_class_type(), make_recorder);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void InputDeviceNodeRecorder::
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
void InputDeviceNodeRecorder::
write_recorder(BamWriter *manager, Datagram &dg) {
  RecorderBase::write_recorder(manager, dg);
  DataNode::write_recorder(manager, dg);  // hier
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type Lens is encountered in the Bam file.  It should create the Lens and
 * extract its information from the file.
 */
TypedWritable *InputDeviceNodeRecorder::
make_from_bam(const FactoryParams &params) {
  InputDeviceNodeRecorder *node = new InputDeviceNodeRecorder("", nullptr);
  DatagramIterator scan;
  BamReader* manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

/**
 * This is similar to make_from_bam(), but it is designed for loading the
 * RecorderBase object from the session log created by a RecorderController.
 */
RecorderBase *InputDeviceNodeRecorder::
make_recorder(const FactoryParams &params) {
  InputDeviceNodeRecorder *node = new InputDeviceNodeRecorder("", nullptr);
  BamReaderParam *param = DCAST(BamReaderParam, params.get_param(0));

  node->fillin_recorder(param->get_iterator(), param->get_manager());

  return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new InputDeviceNodeRecorder.
 */
void InputDeviceNodeRecorder::
fillin(DatagramIterator &scan, BamReader *manager) {
  DataNode::fillin(scan, manager);
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new InputDeviceNodeRecorder.
 */
void InputDeviceNodeRecorder::
fillin_recorder(DatagramIterator &scan, BamReader *manager) {
  RecorderBase::fillin_recorder(scan, manager);
  DataNode::fillin_recorder(scan, manager);
}
