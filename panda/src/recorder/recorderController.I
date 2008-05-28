// Filename: recorderController.I
// Created by:  drose (24Jan04)
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
//     Function: RecorderController::get_start_time
//       Access: Published
//  Description: Returns the time (and date) at which the current
//               session was originally recorded (or, in recording
//               mode, the time at which the current session began).
////////////////////////////////////////////////////////////////////
INLINE time_t RecorderController::
get_start_time() const {
  return _header._start_time;
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderController::set_random_seed
//       Access: Published
//  Description: Indicates an arbitrary number to be recorded in the
//               session file as a random seed, should the application
//               wish to take advantage of it.  This must be set
//               before begin_record() is called.
////////////////////////////////////////////////////////////////////
INLINE void RecorderController::
set_random_seed(int random_seed) {
  _header._random_seed = random_seed;
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderController::get_random_seed
//       Access: Published
//  Description: Returns the random seed that was set by a previous
//               call to set_random_seed(), or the number read from
//               the session file after begin_playback() has been
//               called.
////////////////////////////////////////////////////////////////////
INLINE int RecorderController::
get_random_seed() const {
  return _header._random_seed;
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderController::is_recording
//       Access: Published
//  Description: Returns true if the controller has been opened for
//               output, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool RecorderController::
is_recording() const {
  return (_writer != (BamWriter *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderController::is_playing
//       Access: Published
//  Description: Returns true if the controller has been opened for
//               input, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool RecorderController::
is_playing() const {
  return (_reader != (BamReader *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderController::is_open
//       Access: Published
//  Description: Returns true if the controller has been opened for
//               either input or output, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool RecorderController::
is_open() const {
  return is_recording() || is_playing();
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderController::get_filename
//       Access: Published
//  Description: Returns the filename that was passed to the most
//               recent call to begin_record() or begin_playback().
////////////////////////////////////////////////////////////////////
INLINE const Filename &RecorderController::
get_filename() const {
  return _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderController::is_error
//       Access: Published
//  Description: Returns true if the controller has been opened for
//               input or output output and there is an error on the
//               stream, or false if the controller is closed or if
//               there is no problem.
////////////////////////////////////////////////////////////////////
INLINE bool RecorderController::
is_error() {
  return _dout.is_error() || _din.is_error();
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderController::get_clock_offset
//       Access: Published
//  Description: Returns the delta offset between the actual frame
//               time and the frame time written to the log.  This is
//               essentially the time at which the recording (or
//               playback) started.
////////////////////////////////////////////////////////////////////
INLINE double RecorderController::
get_clock_offset() const {
  return _clock_offset;
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderController::get_frame_offset
//       Access: Published
//  Description: Returns the delta offset between the actual frame
//               count and the frame count written to the log.  This is
//               essentially the frame number at which the recording
//               (or playback) started.
////////////////////////////////////////////////////////////////////
INLINE int RecorderController::
get_frame_offset() const {
  return _frame_offset;
}


////////////////////////////////////////////////////////////////////
//     Function: RecorderController::add_recorder
//       Access: Published
//  Description: Adds the named recorder to the set of recorders that
//               are in use.
//
//               If the controller is in recording mode, the named
//               recorder will begin recording its status to the
//               session file.  If the controller is in playback mode
//               and the name and type matches a recorder in the
//               session file, the recorder will begin receiving data.
////////////////////////////////////////////////////////////////////
INLINE void RecorderController::
add_recorder(const string &name, RecorderBase *recorder) {
  _user_table->add_recorder(name, recorder);
  _user_table_modified = true;

  // We can only add the state flag immediately if we are in recording
  // mode.  In playback mode, we're not sure yet whether the new
  // recorder state will actually be playing (we won't know until we
  // merge the tables in play_frame()).
  if (is_recording()) {
    recorder->_flags |= RecorderBase::F_recording;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderController::has_recorder
//       Access: Published
//  Description: Returns true if the named recorder has been added to
//               the table by a previous call to add_recorder(), false
//               otherwise. 
//
//               If the controller is in playback mode, this will also
//               return false for a recorder that was found in the
//               session file but was never explicitly added via
//               add_recorder(); see get_recorder().
////////////////////////////////////////////////////////////////////
INLINE bool RecorderController::
has_recorder(const string &name) const {
  return (_user_table->get_recorder(name) != (RecorderBase *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderController::get_recorder
//       Access: Published
//  Description: Returns the recorder with the indicated name, or NULL
//               if there is no such recorder.
//
//               If the controller is in playback mode, this may
//               return the recorder matching the indicated name as
//               read from the session file, even if it was never
//               added to the table by the user.  In this case,
//               has_recorder() may return false, but get_recorder()
//               will return a non-NULL value.
////////////////////////////////////////////////////////////////////
INLINE RecorderBase *RecorderController::
get_recorder(const string &name) const {
  RecorderBase *recorder = _user_table->get_recorder(name);
  if (is_playing() && recorder == (RecorderBase *)NULL) {
    recorder = _active_table->get_recorder(name);
  }
  return recorder;
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderController::remove_recorder
//       Access: Published
//  Description: Removes the named recorder from the table.  Returns
//               true if successful, false if there was no such
//               recorder.
//
//               If the controller is in recording mode, the named
//               recorder will stop recording.  If the controller is
//               in playback mode, the named recorder will
//               disassociate itself from the session file (but if the
//               session file still has data for this name, a default
//               recorder will take its place to decode the data from
//               the session file).
////////////////////////////////////////////////////////////////////
INLINE bool RecorderController::
remove_recorder(const string &name) {
  // If we are playing or recording, immediately remove the state flag
  // from the recorder.  (When we are playing, the state flag will get
  // removed automatically at the next call to play_frame(), but we
  // might as well be aggressive and remove it now.  When we are
  // recording, we have to remove it now.)
  if (is_recording() || is_playing()) {
    RecorderBase *recorder = _user_table->get_recorder(name);
    if (recorder != (RecorderBase *)NULL) {
      recorder->_flags &= ~(RecorderBase::F_recording | RecorderBase::F_playing);
    }
  }
  _user_table_modified = true;
  return _user_table->remove_recorder(name);
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderController::set_frame_tie
//       Access: Published
//  Description: Sets the frame_tie flag.
//
//               When this is true, sessions are played back
//               frame-for-frame, based on the frame count of the
//               recorded session.  This gives the most accurate
//               playback, but the playback rate will vary according
//               to the frame rate of the playback machine.
//
//               When this is false, sessions are played back at real
//               time, based on the clock of the recorded session.
//               This may introduce playback discrepencies if the
//               frames do not fall at exactly the same times as they
//               did in the original.
////////////////////////////////////////////////////////////////////
INLINE void RecorderController::
set_frame_tie(bool frame_tie) {
  _frame_tie = frame_tie;
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderController::get_frame_tie
//       Access: Published
//  Description: See set_frame_tie().
////////////////////////////////////////////////////////////////////
INLINE bool RecorderController::
get_frame_tie() const {
  return _frame_tie;
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderController::get_factory
//       Access: Public, Static
//  Description: Returns the global RecorderFactory for generating
//               TypedWritable objects
////////////////////////////////////////////////////////////////////
INLINE RecorderController::RecorderFactory *RecorderController::
get_factory() {
  if (_factory == (RecorderFactory *)NULL) {
    create_factory();
  }
  return _factory;
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderController::create_factory
//       Access: Private, Static
//  Description: Creates a new RecorderFactory for generating
//               TypedWritable objects
////////////////////////////////////////////////////////////////////
INLINE void RecorderController::
create_factory() {
  _factory = new RecorderFactory;
}
