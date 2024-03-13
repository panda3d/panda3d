/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatThreadData.h
 * @author drose
 * @date 2000-07-08
 */

#ifndef PSTATTHREADDATA_H
#define PSTATTHREADDATA_H

#include "pandatoolbase.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "referenceCount.h"

#include "pdeque.h"

class PStatCollectorDef;
class PStatFrameData;
class PStatClientData;
class PStatClientVersion;

/**
 * A collection of FrameData structures for recently-received frames within a
 * particular thread.  This holds the raw data as reported by the client, and
 * it automatically handles frames received out-of-order or skipped.  You can
 * ask for a particular frame by frame number or time and receive the data for
 * the nearest frame.
 */
class PStatThreadData : public ReferenceCount {
public:
  PStatThreadData(const PStatClientData *client_data);
  ~PStatThreadData();

  INLINE const PStatClientData *get_client_data() const;

  INLINE bool is_empty() const;

  int get_latest_frame_number() const;
  int get_oldest_frame_number() const;
  bool has_frame(int frame_number) const;
  const PStatFrameData &get_frame(int frame_number) const;

  double get_latest_time() const;
  double get_oldest_time() const;
  const PStatFrameData &get_frame_at_time(double time) const;
  int get_frame_number_at_time(double time, int hint = -1) const;
  int get_frame_number_after(double time, int start_at = 0) const;

  const PStatFrameData &get_latest_frame() const;

  bool get_elapsed_frames(int &then_i, int &now_i) const;
  double get_frame_rate() const;


  void set_history(double time);
  double get_history() const;
  bool prune_history(double time);

  void record_new_frame(int frame_number, PStatFrameData *frame_data);

  void write_datagram(Datagram &dg) const;
  void read_datagram(DatagramIterator &scan, PStatClientVersion *version);

private:
  void compute_elapsed_frames() const;

  const PStatClientData *_client_data;

  typedef pdeque<PStatFrameData *> Frames;
  Frames _frames;
  int _first_frame_number;
  double _history;

  // Cached values, updated by compute_elapsed_frames().
  mutable bool _computed_elapsed_frames;
  mutable bool _got_elapsed_frames;
  mutable int _then_i;
  mutable int _now_i;

  static PStatFrameData _null_frame;
};

#include "pStatThreadData.I"

#endif
