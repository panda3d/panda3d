#ifndef GESTUREEVENT_H
#define GESTUREEVENT_H

#include "pandabase.h"


enum class GesturePhase {
  UNKNOWN,
  BEGAN,
  CHANGED,
  ENDED,
  CANCELLED
};

enum class GestureType {
  UNKNOWN,
  MAGNIFICATION,
  ROTATION,
  SWIPE
};

class EXPCL_PANDA_EVENT GestureEvent {
public:

  GestureType _type = GestureType::UNKNOWN;
  GesturePhase _phase = GesturePhase::UNKNOWN;
  int _sequence = 0;
  double _time = 0.0;

  virtual ~GestureEvent() {}

  // Which one of these union values is valid depends on _type
  union {
    // Only valid if _type == MAGNIFICATION
    double magnification;
    // Only valid if _type == ROTATION
    double rotation;
    // Only valid if _type == SWIPE
    struct {
      double deltaX;
      double deltaY;
    } swipe;
  } _gestureData;

/**
 * TypedObject implementation
 */
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "GestureEvent",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

class EXPCL_PANDA_EVENT GestureEventList : public ParamValueBase {
PUBLISHED:
  GestureEventList() {};

  bool empty() const;
  int get_num_events() const;
  GestureEvent get_event(int index) const;
public:
  virtual void output(std::ostream &out) const;

  void add_magnification_event(double magnification, GesturePhase phase, int seq, double time);

private:
  typedef pdeque<GestureEvent> Events;
  Events _events;

/**
 * TypedObject implementation
 */

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ParamValueBase::init_type();
    register_type(_type_handle, "GestureEventList",
                  ParamValueBase::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
