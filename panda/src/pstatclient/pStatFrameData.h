// Filename: pStatFrameData.h
// Created by:  drose (10Jul00)
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

#ifndef PSTATFRAMEDATA_H
#define PSTATFRAMEDATA_H

#include "pandabase.h"

#include "notify.h"

#include "pvector.h"

class Datagram;
class DatagramIterator;
class PStatClientVersion;

////////////////////////////////////////////////////////////////////
//       Class : PStatFrameData
// Description : Contains the raw timing and level data for a single
//               frame.  This is a sequence of start/stop events, as
//               well as a table of level values, associated with a
//               number of collectors within a single frame.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PStatFrameData {
public:
  INLINE bool is_time_empty() const;
  INLINE bool is_level_empty() const;
  INLINE bool is_empty() const;
  INLINE void clear();

  INLINE void add_start(int index, float time);
  INLINE void add_stop(int index, float time);
  INLINE void add_level(int index, float level);

  INLINE float get_start() const;
  INLINE float get_end() const;
  INLINE float get_net_time() const;

  INLINE int get_num_events() const;
  INLINE int get_time_collector(int n) const;
  INLINE bool is_start(int n) const;
  INLINE float get_time(int n) const;

  INLINE int get_num_levels() const;
  INLINE int get_level_collector(int n) const;
  INLINE float get_level(int n) const;

  void write_datagram(Datagram &destination) const;
  void read_datagram(DatagramIterator &source, PStatClientVersion *version);

private:
  class DataPoint {
  public:
    int _index;
    float _value;
  };
  typedef pvector<DataPoint> Data;

  Data _time_data, _level_data;
};

#include "pStatFrameData.I"

#endif

