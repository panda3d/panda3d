// Filename: pStatFrameData.h
// Created by:  drose (10Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PSTATFRAMEDATA_H
#define PSTATFRAMEDATA_H

#include <pandabase.h>

#include <notify.h>

#include <vector>

class Datagram;
class DatagramIterator;

////////////////////////////////////////////////////////////////////
// 	 Class : PStatFrameData
// Description : Defines the raw timing data for a single frame.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PStatFrameData {
public:
  INLINE bool is_empty() const;
  INLINE void clear();
  INLINE void add_start(int index, double time);
  INLINE void add_stop(int index, double time);

  INLINE double get_start() const;
  INLINE double get_end() const;
  INLINE double get_net_time() const;

  INLINE int get_num_events() const;
  INLINE int get_collector(int n) const;
  INLINE double get_time(int n) const;

  void write_datagram(Datagram &destination) const;
  void read_datagram(DatagramIterator &source);

private:  
  class DataPoint {
  public:
    int _index;
    double _time;
  };
  typedef vector<DataPoint> Data;

  Data _data;
};

#include "pStatFrameData.I"

#endif

