/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointerEventList.h
 * @author drose
 * @date 2002-03-12
 */

#ifndef POINTEREVENTLIST_H
#define POINTEREVENTLIST_H

#include "pandabase.h"

#include "pointerEvent.h"
#include "typedReferenceCount.h"
#include "eventParameter.h"
#include "vector_double.h"

class ModifierPointers;
class Datagram;
class DatagramIterator;

/**
 * Records a set of pointer events that happened recently.  This class is
 * usually used only in the data graph, to transmit the recent pointer
 * presses, but it may be used anywhere a list of PointerEvents is desired.
 */
class EXPCL_PANDA_EVENT PointerEventList : public ParamValueBase {
PUBLISHED:
  INLINE PointerEventList();

  INLINE size_t get_num_events() const;
  INLINE bool   get_in_window(size_t n) const;
  INLINE int    get_xpos(size_t n) const;
  INLINE int    get_ypos(size_t n) const;
  INLINE double get_dx(size_t n) const;
  INLINE double get_dy(size_t n) const;
  INLINE int    get_sequence(size_t n) const;
  INLINE double get_length(size_t n) const;
  INLINE double get_direction(size_t n) const;
  INLINE double get_rotation(size_t n) const;
  INLINE double get_time(size_t n) const;

  INLINE void   clear();
  INLINE void   pop_front();
  void add_event(const PointerData &data, int seq, double time);
  void add_event(bool in_win, int xpos, int ypos, int seq, double time);
  void add_event(bool in_win, int xpos, int ypos, double xdelta, double ydelta,
                 int seq, double time);

  bool   encircles(int x, int y) const;
  double total_turns(double sec) const;
  double match_pattern(const std::string &pattern, double rot, double seglen);

public:
  INLINE PointerEventList(const PointerEventList &copy);
  INLINE void operator = (const PointerEventList &copy);

  INLINE bool empty() const;
  INLINE const PointerEvent &get_event(size_t n) const;

  virtual void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;

private:
  void parse_pattern(const std::string &ascpat, vector_double &pattern);
  typedef pdeque<PointerEvent> Events;
  Events _events;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ParamValueBase::init_type();
    register_type(_type_handle, "PointerEventList",
                  ParamValueBase::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const PointerEventList &pointerlist) {
  pointerlist.output(out);
  return out;
}

#include "pointerEventList.I"

#endif
