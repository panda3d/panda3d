// Filename: pStatFrameData.h
// Created by:  drose (10Jul00)
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

#ifndef PSTATFRAMEDATA_H
#define PSTATFRAMEDATA_H

#include "pandabase.h"

#include "pnotify.h"

#include "pvector.h"

class Datagram;
class DatagramIterator;
class PStatClientVersion;
class PStatClient;

////////////////////////////////////////////////////////////////////
//       Class : PStatFrameData
// Description : Contains the raw timing and level data for a single
//               frame.  This is a sequence of start/stop events, as
//               well as a table of level values, associated with a
//               number of collectors within a single frame.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PSTATCLIENT PStatFrameData {
public:
  INLINE bool is_time_empty() const;
  INLINE bool is_level_empty() const;
  INLINE bool is_empty() const;
  INLINE void clear();
  INLINE void swap(PStatFrameData &other);

  INLINE void add_start(int index, double time);
  INLINE void add_stop(int index, double time);
  INLINE void add_level(int index, double level);

  void sort_time();

  INLINE double get_start() const;
  INLINE double get_end() const;
  INLINE double get_net_time() const;

  INLINE int get_num_events() const;
  INLINE int get_time_collector(int n) const;
  INLINE bool is_start(int n) const;
  INLINE double get_time(int n) const;

  INLINE int get_num_levels() const;
  INLINE int get_level_collector(int n) const;
  INLINE double get_level(int n) const;

  bool write_datagram(Datagram &destination, PStatClient *client) const;
  void read_datagram(DatagramIterator &source, PStatClientVersion *version);

private:
  class DataPoint {
  public:
    INLINE bool operator < (const DataPoint &other) const;

    int _index;
    double _value;
  };
  typedef pvector<DataPoint> Data;

  Data _time_data, _level_data;
};

#include "pStatFrameData.I"

#endif

