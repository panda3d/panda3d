/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file recorderController.cxx
 * @author drose
 * @date 2004-01-24
 */

#include "recorderController.h"
#include "recorderFrame.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "config_recorder.h"
#include "bam.h"
#include "clockObject.h"

TypeHandle RecorderController::_type_handle;
RecorderController::RecorderFactory *RecorderController::_factory = nullptr;

/**
 *
 */
RecorderController::
RecorderController() {
  _clock_offset = 0.0;
  _frame_offset = 0;
  _writer = nullptr;
  _reader = nullptr;
  _frame_tie = true;
  _user_table = new RecorderTable;
  _user_table_modified = false;
  _file_table = nullptr;
  _active_table = nullptr;
  _eof = false;
}

/**
 *
 */
RecorderController::
~RecorderController() {
  close();
  delete _user_table;
}

/**
 * Begins recording data to the indicated filename.  All of the recorders in
 * use should already have been added.
 */
bool RecorderController::
begin_record(const Filename &filename) {
  close();
  _filename = filename;
  ClockObject *global_clock = ClockObject::get_global_clock();
  _clock_offset = global_clock->get_frame_time();
  _frame_offset = global_clock->get_frame_count();

  time(&_header._start_time);

  if (!_dout.open(_filename)) {
    recorder_cat.error() << "Unable to open " << _filename << "\n";
    return false;
  }

  if (!_dout.write_header(_bam_header)) {
    recorder_cat.error() << "Unable to write to " << _filename << "\n";
    return false;
  }

  _writer = new BamWriter(&_dout);

  if (!_writer->init()) {
    close();
    return false;
  }

  // Write out the header information.
  _writer->write_object(&_header);

  _user_table_modified = true;

  // Tell all of our recorders that they're live now.
  _user_table->set_flags(RecorderBase::F_recording);

  recorder_cat.info()
    << "Recording session to " << _filename << "\n";

  return true;
}

/**
 * Begins playing back data from the indicated filename.  All of the recorders
 * in use should already have been added, although this may define additional
 * recorders if they are present in the file (these new recorders will not be
 * used).  This may also undefine recorders that were previously added but are
 * not present in the file.
 */
bool RecorderController::
begin_playback(const Filename &filename) {
  close();
  _filename = filename;
  ClockObject *global_clock = ClockObject::get_global_clock();
  _clock_offset = global_clock->get_frame_time();
  _frame_offset = global_clock->get_frame_count();

  if (!_din.open(_filename)) {
    recorder_cat.error() << "Unable to open " << _filename << "\n";
    return false;
  }

  std::string head;
  if (!_din.read_header(head, _bam_header.size()) || head != _bam_header) {
    recorder_cat.error() << "Unable to read " << _filename << "\n";
    return false;
  }

  _reader = new BamReader(&_din);
  if (!_reader->init()) {
    close();
    return false;
  }

  _user_table_modified = true;
  _active_table = new RecorderTable;
  _eof = false;

  // Start out by reading the RecorderHeader.
  TypedWritable *object = _reader->read_object();

  if (object == nullptr ||
      !object->is_of_type(RecorderHeader::get_class_type())) {
    recorder_cat.error()
      << _filename << " does not contain a recorded session.\n";
    close();
    return false;
  }

  if (!_reader->resolve()) {
    recorder_cat.warning()
      << "Unable to resolve header data.\n";
  }

  RecorderHeader *new_header = DCAST(RecorderHeader, object);
  _header = (*new_header);
  delete new_header;

  // Now read the first frame.
  _next_frame = read_frame();
  if (_next_frame == nullptr) {
    recorder_cat.error()
      << _filename << " does not contain any frames.\n";
    close();
    return false;
  }

  recorder_cat.info()
    << "Playing back session from " << _filename << "\n";

  return true;
}

/**
 * Finishes recording data to the indicated filename.
 */
void RecorderController::
close() {
  if (_writer != nullptr) {
    delete _writer;
    _writer = nullptr;

    // Tell all of our recorders that they're no longer recording.
    _user_table->clear_flags(RecorderBase::F_recording);
  }
  if (_reader != nullptr) {
    delete _reader;
    _reader = nullptr;

    // Tell all of our recorders that they're no longer playing.
    _active_table->clear_flags(RecorderBase::F_playing);
  }
  _dout.close();
  _din.close();

  if (_file_table != nullptr) {
    delete _file_table;
    _file_table = nullptr;
  }

  if (_active_table != nullptr) {
    delete _active_table;
    _active_table = nullptr;
  }
}

/**
 * Gets the next frame of data from all of the active recorders and adds it to
 * the output file.
 */
void RecorderController::
record_frame() {
  if (is_recording()) {
    ClockObject *global_clock = ClockObject::get_global_clock();
    double now = global_clock->get_frame_time() - _clock_offset;
    int frame = global_clock->get_frame_count() - _frame_offset;

    RecorderFrame data(now, frame, _user_table_modified, _user_table);
    _user_table_modified = false;

    _writer->write_object(&data);
  }
}

/**
 * Gets the next frame of data from all of the active recorders and adds it to
 * the output file.
 */
void RecorderController::
play_frame() {
  if (is_playing()) {
    if (_eof) {
      close();
      return;
    }

    ClockObject *global_clock = ClockObject::get_global_clock();
    double now = global_clock->get_frame_time() - _clock_offset;
    int frame = global_clock->get_frame_count() - _frame_offset;

    while (_next_frame != nullptr) {
      if (_frame_tie) {
        if (frame < _next_frame->_frame) {
          // We haven't reached the next frame yet.
          return;
        }

        // Insist that the clock runs at the same rate as it did in the
        // previous session.
        // global_clock->set_frame_time(_next_frame->_timestamp +
        // _clock_offset); global_clock->set_real_time(_next_frame->_timestamp
        // + _clock_offset);

        // Hmm, that's crummy.  Just keep the clock offset up-to-date.
        _clock_offset = global_clock->get_frame_time() - _next_frame->_timestamp;

      } else {
        if (now < _next_frame->_timestamp) {
          // We haven't reached the next frame yet.
          return;
        }

        // Keep our frame_offset up-to-date.
        _frame_offset = global_clock->get_frame_count() - _next_frame->_frame;
      }

      if (_next_frame->_table_changed && _file_table != _next_frame->_table) {
        delete _file_table;
        _file_table = _next_frame->_table;
      }

      if (_next_frame->_table_changed || _user_table_modified) {
        // We're about to change the active table.  Temporarily disable the
        // playing flag on the currently-active recorders.
        _active_table->clear_flags(RecorderBase::F_playing);
        delete _active_table;
        _active_table = new RecorderTable(*_file_table);
        _active_table->merge_from(*_user_table);
        _user_table_modified = false;

        // Now reenable the playing flag on the newly-active recorders.
        _active_table->set_flags(RecorderBase::F_playing);
      }

      _next_frame->_table = _active_table;
      _next_frame->play_frame(_reader);

      delete _next_frame;
      _next_frame = read_frame();
    }

    if (_reader->is_eof()) {
      recorder_cat.info()
        << "End of recorded session.\n";
    } else {
      recorder_cat.error()
        << "Unable to read datagram from recorded session.\n";
    }
    _eof = true;
  }
}


/**
 * Loads the next frame data from the playback session file.  Returns the
 * frame data pointer on success, or NULL on failure.
 */
RecorderFrame *RecorderController::
read_frame() {
  TypedWritable *object = _reader->read_object();

  if (object == nullptr ||
      !object->is_of_type(RecorderFrame::get_class_type())) {
    return nullptr;
  }

  if (!_reader->resolve()) {
    recorder_cat.warning()
      << "Unable to resolve frame data.\n";
  }

  return DCAST(RecorderFrame, object);
}
