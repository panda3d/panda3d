/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file socketStreamRecorder.cxx
 * @author drose
 * @date 2004-01-28
 */

#include "socketStreamRecorder.h"
#include "recorderController.h"
#include "bamReader.h"
#include "bamWriter.h"

#ifdef HAVE_OPENSSL

TypeHandle SocketStreamRecorder::_type_handle;

/**
 * Receives a datagram over the socket by expecting a little-endian 16-bit
 * byte count as a prefix.  If the socket stream is non-blocking, may return
 * false if the data is not available; otherwise, returns false only if the
 * socket closes.
 */
bool SocketStreamRecorder::
receive_datagram(Datagram &dg) {
  if (is_playing()) {
    // If we're playing back data, the datagrams come only from the queue, not
    // from the live connection.
    if (!_data.empty()) {
      dg = _data.front();
      _data.pop_front();
      return true;
    }

    return false;

  } else {
    // If we're not in playback mode, forward the request to the connection.
    bool got_data = false;
    if (_stream != nullptr) {
      got_data = _stream->receive_datagram(dg);
    }

    if (got_data && is_recording()) {
      // If we're in recording mode, save the data.
      _data.push_back(dg);
    }

    return got_data;
  }
}

/**
 * Records the most recent data collected into the indicated datagram, and
 * returns true if there is any interesting data worth recording, or false if
 * the datagram is meaningless.
 */
void SocketStreamRecorder::
record_frame(BamWriter *manager, Datagram &dg) {
  nassertv(is_recording());
  dg.add_bool(_closed);
  dg.add_uint16(_data.size());
  for (Data::iterator di = _data.begin(); di != _data.end(); ++di) {
    dg.add_string((*di).get_message());
  }
  _data.clear();
}


/**
 * Reloads the most recent data collected from the indicated datagram.
 */
void SocketStreamRecorder::
play_frame(DatagramIterator &scan, BamReader *manager) {
  nassertv(is_playing());
  _closed = scan.get_bool();

  int num_packets = scan.get_uint16();
  for (int i = 0; i < num_packets; i++) {
    _data.push_back(Datagram(scan.get_blob()));
  }
}

/**
 * Tells the BamReader how to create objects of type Lens.
 */
void SocketStreamRecorder::
register_with_read_factory() {
  RecorderController::get_factory()->register_factory(get_class_type(), make_recorder);
}

/**
 * Writes the contents of this object to the datagram for encoding in the
 * session file.  This is very similar to write_datagram() for TypedWritable
 * objects, but it is used specifically to write the Recorder object when
 * generating the session file.  In many cases, it will be the same as
 * write_datagram().
 */
void SocketStreamRecorder::
write_recorder(BamWriter *manager, Datagram &dg) {
  RecorderBase::write_recorder(manager, dg);
}

/**
 * This is similar to make_from_bam(), but it is designed for loading the
 * RecorderBase object from the session log created by a RecorderController.
 */
RecorderBase *SocketStreamRecorder::
make_recorder(const FactoryParams &params) {
  SocketStreamRecorder *node = new SocketStreamRecorder;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin_recorder(scan, manager);

  return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new SocketStreamRecorder.
 */
void SocketStreamRecorder::
fillin_recorder(DatagramIterator &scan, BamReader *manager) {
  RecorderBase::fillin_recorder(scan, manager);
}

#endif  // HAVE_OPENSSL
