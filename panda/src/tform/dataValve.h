// Filename: dataValve.h
// Created by:  drose (05Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef DATAVALVE_H
#define DATAVALVE_H

#include <pandabase.h>

#include <dataNode.h>
#include <modifierButtons.h>
#include <referenceCount.h>
#include <pointerTo.h>

////////////////////////////////////////////////////////////////////
//       Class : DataValve
// Description : This DataNode can be used to selectively control
//               which child node or nodes receive certain data graph
//               events, based on the type of data and possibly on the
//               combination of modifier buttons being held down by
//               the user.
//
//               It's particularly useful when used in conjunction
//               with one or more Trackball interfaces, for instance,
//               to move around various different objects according to
//               the key combination being held down.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DataValve : public DataNode {
PUBLISHED:
  DataValve(const string &name = "");
  virtual ~DataValve();

  // The Control is used extensively within the DataValve class to
  // manage the data flow; see the comments on the Control
  // constructor.
  class EXPCL_PANDA Control : public ReferenceCount {
  PUBLISHED:
    INLINE Control();

    INLINE void set_on();
    INLINE void set_off();
    INLINE void set_buttons(const ModifierButtons &mods);

    bool is_on(const DataValve &valve) const;

    void output(ostream &out) const;

  private:
    enum State {
      S_on,
      S_off,
      S_buttons
    };
    State _state;
    ModifierButtons _mods;
  };

  INLINE void set_default_control(Control *control);
  INLINE Control *get_default_control() const;

  void set_control(int child_index, Control *control);
  void clear_control(int child_index);
  bool has_control(int child_index) const;
  Control *get_control(int child_index) const;

  void set_fine_control(int child_index, TypeHandle data_type, Control *control);
  void clear_fine_control(int child_index, TypeHandle data_type);
  bool has_fine_control(int child_index, TypeHandle data_type) const;
  Control *get_fine_control(int child_index, TypeHandle data_type) const;

  INLINE void set_modifier_buttons(const ModifierButtons &mods);
  INLINE const ModifierButtons &get_modifier_buttons() const;

public:
  virtual void write(ostream &out, int indent_level = 0) const;

private:
  void ensure_child_index(int child_index);

  ModifierButtons _mods;
  PT(Control) _default_control;

  typedef map<TypeHandle, PT(Control)> FineControls;

  class Child {
  public:
    PT(Control) _control;
    FineControls _fine_controls;
  };

  typedef vector<Child> Controls;
  Controls _controls;


////////////////////////////////////////////////////////////////////
// From parent class DataNode
////////////////////////////////////////////////////////////////////
public:
  virtual void
  transmit_data(NodeAttributes &data);

  virtual void
  transmit_data_per_child(NodeAttributes &data, int child_index);

  // inputs
  static TypeHandle _button_events_type;


public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const DataValve::Control &control) {
  control.output(out);
  return out;
}

#include "dataValve.I"

#endif
