#include "gestureEvent.h"

TypeHandle GestureEvent::_type_handle;
TypeHandle GestureEventList::_type_handle;

void GestureEventList::
add_magnification_event(double magnification, GesturePhase phase, int seq, double time) {
  GestureEvent me;
  me._type = GestureType::MAGNIFICATION;
  me._phase = phase;
  me._sequence = seq;
  me._time = time;

  me._gestureData.magnification = magnification;
  _events.push_back(me);
}

void GestureEventList::
output(std::ostream &out) const {
  if (_events.empty()) {
    out << "(no pointers)";
  } else {
    out << "(not implemented)";
  }
}

bool GestureEventList::
empty() const {
  return _events.empty();
}


int GestureEventList::
get_num_events() const {
  return _events.size();
}

GestureEvent GestureEventList::
get_event(int index) const {
  return _events[index];
}
