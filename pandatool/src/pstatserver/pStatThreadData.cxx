/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatThreadData.cxx
 * @author drose
 * @date 2000-07-09
 */

#include "pStatThreadData.h"

#include "pStatFrameData.h"
#include "pStatCollectorDef.h"
#include "config_pstatclient.h"


PStatFrameData PStatThreadData::_null_frame;

/**
 *
 */
PStatThreadData::
PStatThreadData(const PStatClientData *client_data) :
  _client_data(client_data)
{
  _first_frame_number = 0;
  _history = pstats_history;
  _computed_elapsed_frames = false;
}

/**
 *
 */
PStatThreadData::
~PStatThreadData() {
}

/**
 * Returns the frame number of the most recent frame stored in the data.
 */
int PStatThreadData::
get_latest_frame_number() const {
  nassertr(!_frames.empty(), 0);
  return _first_frame_number + _frames.size() - 1;
}

/**
 * Returns the frame number of the oldest frame still stored in the data.
 */
int PStatThreadData::
get_oldest_frame_number() const {
  nassertr(!_frames.empty(), 0);
  return _first_frame_number;
}

/**
 * Returns true if we have received data for the indicated frame number from
 * the client and we still have it stored, or false otherwise.
 */
bool PStatThreadData::
has_frame(int frame_number) const {
  int rel_frame = frame_number - _first_frame_number;

  return (rel_frame >= 0 && rel_frame < (int)_frames.size() &&
          _frames[rel_frame] != nullptr);
}

/**
 * Returns a FrameData structure associated with the indicated frame number.
 * If the frame data has not yet been received from the client, returns the
 * newest frame older than the requested frame.
 */
const PStatFrameData &PStatThreadData::
get_frame(int frame_number) const {
  int rel_frame = frame_number - _first_frame_number;
  int num_frames = _frames.size();
  if (rel_frame >= num_frames) {
    rel_frame = num_frames - 1;
  }

  while (rel_frame >= 0 && _frames[rel_frame] == nullptr) {
    rel_frame--;
  }
  if (rel_frame < 0) {
    // No frame data that old.  Return the oldest frame we've got.
    rel_frame = 0;
    while (rel_frame < num_frames &&
           _frames[rel_frame] == nullptr) {
      rel_frame++;
    }
  }

  if (rel_frame >= 0 && rel_frame < num_frames) {
    PStatFrameData *frame = _frames[rel_frame];
    nassertr(frame != nullptr, _null_frame);
    nassertr(frame->get_start() >= 0.0, _null_frame);
    return *frame;
  }

  nassertr(_null_frame.get_start() >= 0.0, _null_frame);
  return _null_frame;
}

/**
 * Returns the timestamp (in seconds elapsed since connection) of the latest
 * available frame.
 */
double PStatThreadData::
get_latest_time() const {
  nassertr(!_frames.empty(), 0.0);
  return _frames.back()->get_start();
}

/**
 * Returns the timestamp (in seconds elapsed since connection) of the oldest
 * available frame.
 */
double PStatThreadData::
get_oldest_time() const {
  nassertr(!_frames.empty(), 0.0);
  return _frames.front()->get_start();
}

/**
 * Returns the FrameData structure associated with the latest frame not later
 * than the indicated time.
 */
const PStatFrameData &PStatThreadData::
get_frame_at_time(double time) const {
  return get_frame(get_frame_number_at_time(time));
}

/**
 * Returns the frame number of the latest frame not later than the indicated
 * time.  If no frames were found, returns get_oldest_frame_number() - 1.
 *
 * If the hint is nonnegative, it represents a frame number (for which
 * has_frame() must return true) we believe the correct answer to be after and
 * close by, which may speed the search for the frame.
 */
int PStatThreadData::
get_frame_number_at_time(double time, int hint) const {
  hint -= _first_frame_number;
  if (hint >= 0 && hint < (int)_frames.size()) {
    if (_frames[hint] != nullptr &&
        _frames[hint]->get_start() <= time) {
      // The hint might be right.  Scan forward from there.
      int i = hint + 1;
      while (i < (int)_frames.size() &&
             (_frames[i] == nullptr ||
              _frames[i]->get_start() <= time)) {
        if (_frames[i] != nullptr) {
          hint = i;
        }
        ++i;
      }
      return _first_frame_number + hint;
    }
  }

  // The hint is wrong, we have to search the entire set.
  int first_i = 0;
  int last_i = _frames.size() - 1;
  while (first_i < last_i && _frames[first_i] == nullptr) {
    ++first_i;
  }
  while (first_i < last_i && _frames[last_i] == nullptr) {
    --last_i;
  }

  if (first_i >= last_i) {
    // There are no frames.
    return _first_frame_number - 1;
  }

  // Take a guess by interpolating based on the first and last frame times.
  double first = _frames[first_i]->get_start();
  if (time < first) {
    return _first_frame_number - 1;
  }

  double last = _frames[last_i]->get_start();
  double t = (time - first) / (last - first);
  hint = std::max(0, (int)(t * (last_i - first_i))) + first_i;

  // Find a frame around the guess that has data.
  if (_frames[hint] == nullptr) {
    if (hint <= first_i) {
      hint = first_i;
    }
    else do {
      // Skip backward until we find a frame with data.
      --hint;
    } while (_frames[hint] == nullptr);
  }

  if (_frames[hint]->get_start() <= time) {
    // Search forward.
    for (int i = hint + 1; i < (int)_frames.size(); ++i) {
      const PStatFrameData *frame = _frames[i];
      if (frame != nullptr) {
        if (frame->get_start() <= time) {
          hint = i;
        } else {
          break;
        }
      }
    }
    return _first_frame_number + hint;
  }

  // Search backward.
  for (int i = hint - 1; i >= 0; --i) {
    const PStatFrameData *frame = _frames[i];
    if (frame != nullptr && frame->get_start() <= time) {
      return _first_frame_number + i;
    }
  }

  return _first_frame_number - 1;
}

/**
 * Returns the frame number of the first frame later than the indicated time
 * and start frame number.
 */
int PStatThreadData::
get_frame_number_after(double time, int start_at) const {
  int i = std::max(0, start_at - _first_frame_number);
  double end = get_frame(i).get_end();

  while (end < time) {
    ++i;
    if ((size_t)i >= _frames.size()) {
      break;
    }
    if (_frames[i] != nullptr) {
      end = _frames[i]->get_end();
    }
  }

  return _first_frame_number + i;
}

/**
 * Returns the FrameData associated with the most recent frame.
 */
const PStatFrameData &PStatThreadData::
get_latest_frame() const {
  nassertr(!_frames.empty(), _null_frame);
  return *_frames.back();
}

/**
 * Computes the oldest frame number not older than pstats_average_time
 * seconds, and the newest frame number.  Handy for computing average frame
 * rate over a time.  Returns true if there is any data in that range, false
 * otherwise.
 */
bool PStatThreadData::
get_elapsed_frames(int &then_i, int &now_i) const {
  if (!_computed_elapsed_frames) {
    compute_elapsed_frames();
  }

  now_i = _now_i;
  then_i = _then_i;
  return _got_elapsed_frames;
}

/**
 * Computes the average frame rate over the past pstats_average_time seconds,
 * by counting up the number of frames elapsed in that time interval.
 */
double PStatThreadData::
get_frame_rate() const {
  int then_i, now_i;
  if (!get_elapsed_frames(then_i, now_i)) {
    return 0.0f;
  }

  int num_frames = now_i - then_i + 1;
  double now = _frames[now_i - _first_frame_number]->get_end();
  double elapsed_time = (now - _frames[then_i - _first_frame_number]->get_start());
  return (double)num_frames / elapsed_time;
}


/**
 * Sets the number of seconds worth of frames that will be retained by the
 * ThreadData structure as each new frame is added.  This affects how old the
 * oldest frame that may be queried is.
 */
void PStatThreadData::
set_history(double time) {
  _history = time;
}

/**
 * Returns the number of seconds worth of frames that will be retained by the
 * ThreadData structure as each new frame is added.  This affects how old the
 * oldest frame that may be queried is.
 */
double PStatThreadData::
get_history() const {
  return _history;
}

/**
 * Given a timestamp representing the time of the latest known frame, removes
 * any frames older than the configured history.  Returns true if the data is
 * now empty.
 */
bool PStatThreadData::
prune_history(double time) {
  double oldest_allowable_time = time - _history;
  while (!_frames.empty() &&
         (_frames.front() == nullptr ||
          _frames.front()->is_time_empty() ||
          _frames.front()->get_start() < oldest_allowable_time)) {
    delete _frames.front();
    _frames.pop_front();
    _first_frame_number++;
  }

  return _frames.empty();
}

/**
 * Makes room for and stores a new frame's worth of data.  Calling this
 * function may cause old frame data to be discarded to make room, according
 * to the amount of time set up via set_history().
 *
 * The pointer will become owned by the PStatThreadData object and will be
 * freed on destruction.
 */
void PStatThreadData::
record_new_frame(int frame_number, PStatFrameData *frame_data) {
  nassertv(frame_data != nullptr);
  nassertv(!frame_data->is_empty());

  // First, remove all the old frames that fall outside of our history window.
  // Then, add enough empty frame definitions to account for the latest frame
  // number.  This might involve some skips, since we don't guarantee that we
  // get all the frames in order or even at all.
  if (prune_history(frame_data->get_start())) {
    _first_frame_number = frame_number;
    _frames.push_back(nullptr);
  }
  else {
    while (_first_frame_number + (int)_frames.size() <= frame_number) {
      _frames.push_back(nullptr);
    }
  }

  int index = frame_number - _first_frame_number;

  // It's possible to receive frames out of order.
  while (index < 0) {
    _frames.push_front(nullptr);
    ++index;
    --_first_frame_number;
  }

  nassertv(index >= 0 && index < (int)_frames.size());

  if (_frames[index] != nullptr) {
    nout << "Got repeated frame data for frame " << frame_number << "\n";
    delete _frames[index];
  }

  _frames[index] = frame_data;
  _computed_elapsed_frames = false;
}

/**
 * Writes the thread data to a datagram.
 */
void PStatThreadData::
write_datagram(Datagram &dg) const {
  int frame_number = _first_frame_number;

  for (PStatFrameData *frame_data : _frames) {
    if (frame_data != nullptr) {
      dg.add_int32(frame_number);
      frame_data->write_datagram(dg);
    }
    ++frame_number;
  }
  dg.add_int32(-1);
}

/**
 * Restores the thread data from a datagram.
 */
void PStatThreadData::
read_datagram(DatagramIterator &scan, PStatClientVersion *version) {
  int frame_number;
  while ((frame_number = scan.get_int32()) != -1) {
    PStatFrameData *frame_data = new PStatFrameData;
    frame_data->read_datagram(scan, version);

    record_new_frame(frame_number, frame_data);
  }

  compute_elapsed_frames();
}

/**
 * Computes the frame numbers returned by get_elapsed_frames().
 */
void PStatThreadData::
compute_elapsed_frames() const {
  if (_frames.empty()) {
    // No frames in the data at all.
    _got_elapsed_frames = false;
  }
  else {
    _now_i = _frames.size() - 1;
    while (_now_i > 0 && _frames[_now_i] == nullptr) {
      _now_i--;
    }
    if (_now_i < 0) {
      // No frames have any real data.
      _got_elapsed_frames = false;
    }
    else {
      nassertv(_frames[_now_i] != nullptr);

      double now = _frames[_now_i]->get_end();
      double then = now - pstats_average_time;

      int old_i = _now_i;
      _then_i = _now_i;

      while (old_i >= 0) {
        const PStatFrameData *frame = _frames[old_i];
        if (frame != nullptr) {
          if (frame->get_start() > then) {
            _then_i = old_i;
          } else {
            break;
          }
        }
        old_i--;
      }

      nassertv(_then_i >= 0);
      nassertv(_frames[_then_i] != nullptr);
      _got_elapsed_frames = true;

      _now_i += _first_frame_number;
      _then_i += _first_frame_number;
    }
  }

  _computed_elapsed_frames = true;
}
