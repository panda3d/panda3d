// Filename: pStatViewLevel.h
// Created by:  drose (11Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PSTATVIEWLEVEL_H
#define PSTATVIEWLEVEL_H

#include <pandatoolbase.h>

#include <vector>

class PStatClientData;

////////////////////////////////////////////////////////////////////
// 	 Class : PStatViewLevel
// Description : This is a single level value, or band of color,
//               within a View.  It generally indicates the elapsed
//               time for a particular Collector within a given frame
//               for a particular thread.
////////////////////////////////////////////////////////////////////
class PStatViewLevel {
public:
  INLINE int get_collector() const;
  INLINE double get_time_alone() const;
  double get_net_time() const;

  void sort_children(const PStatClientData *client_data);

  int get_num_children() const;
  const PStatViewLevel *get_child(int n) const;

private:
  int _collector;
  double _time_alone;
  PStatViewLevel *_parent;

  typedef vector<PStatViewLevel *> Children;
  Children _children;

  friend class PStatView;
};

#include "pStatViewLevel.I"

#endif
