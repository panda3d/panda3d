// Filename: dataValve.cxx
// Created by:  drose (05Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "dataValve.h"

#include <dataRelation.h>
#include <buttonEventDataAttribute.h>
#include <buttonEventDataTransition.h>
#include <nodeAttributes.h>

TypeHandle DataValve::_type_handle;

TypeHandle DataValve::_button_events_type;

////////////////////////////////////////////////////////////////////
//     Function: DataValve::Control::is_on
//       Access: Published
//  Description: Returns true if the indicated Control is currently
//               on, given the particular DataNode it applies to.
//               (The DataNode is necessary to determine the current
//               state of the ModiferButtons.
////////////////////////////////////////////////////////////////////
bool DataValve::Control::
is_on(const DataValve &valve) const {
  switch (_state) {
  case S_on:
    return true;

  case S_off:
    return false;

  case S_buttons:
    return _mods == valve.get_modifier_buttons();
  }

  // Invalid state.
  nassertr(false, false);
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DataValve::Control::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void DataValve::Control::
output(ostream &out) const {
  switch (_state) {
  case S_on:
    out << "on";
    break;

  case S_off:
    out << "off";
    break;

  case S_buttons:
    out << _mods;
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DataValve::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
DataValve::
DataValve(const string &name) : DataNode(name) {
  _default_control = new Control;
}

////////////////////////////////////////////////////////////////////
//     Function: DataValve::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DataValve::
~DataValve() {
}

////////////////////////////////////////////////////////////////////
//     Function: DataValve::set_control
//       Access: Published
//  Description: Sets an overall Control on the indicated child node.
//               The nth child of this node, zero-based, will receive
//               data only while the indicated Control is on.  This
//               affects all types of data that is not specifically
//               mentioned by set_fine_control().
////////////////////////////////////////////////////////////////////
void DataValve::
set_control(int child_index, Control *control) {
  ensure_child_index(child_index);
  nassertv(child_index >= 0 && child_index < (int)_controls.size());
  _controls[child_index]._control = control;
}

////////////////////////////////////////////////////////////////////
//     Function: DataValve::clear_control
//       Access: Published
//  Description: Removes the overall Control, as well as all fine
//               controls, from the indicated child node.  The
//               indicated child will now be affected only by the
//               default control.  See set_control() and
//               set_fine_control().
////////////////////////////////////////////////////////////////////
void DataValve::
clear_control(int child_index) {
  nassertv(child_index >= 0);
  if (child_index < (int)_controls.size()) {
    _controls[child_index]._control.clear();
    _controls[child_index]._fine_controls.clear();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DataValve::has_control
//       Access: Published
//  Description: Returns true if the indicated child has a particular
//               Control set, or if it has *any* fine controls set.
//               If this returns false, the child is controlled only
//               by the default control.  See set_control() and
//               set_fine_control().
////////////////////////////////////////////////////////////////////
bool DataValve::
has_control(int child_index) const {
  nassertr(child_index >= 0, false);
  if (child_index < (int)_controls.size()) {
    return
      !_controls[child_index]._control.is_null() ||
      !_controls[child_index]._fine_controls.empty();
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DataValve::get_control
//       Access: Published
//  Description: Returns the control that has been set for the
//               indicated child, or NULL if no control has been set
//               (which implies the default control is in affect for
//               the indicated child).  Note that it is possible for
//               this function to return NULL even if has_control(),
//               above, returned true, since a fine control may have
//               been set without setting a particular general
//               control.
////////////////////////////////////////////////////////////////////
DataValve::Control *DataValve::
get_control(int child_index) const {
  nassertr(child_index >= 0, NULL);
  if (child_index < (int)_controls.size()) {
    return _controls[child_index]._control;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DataValve::set_fine_control
//       Access: Published
//  Description: Sets the "fine" control for the indicated child index
//               and the indicated data type.  This overrides the
//               control set by set_control() for this child, but only
//               for data of this type.  This can be used to send
//               data of different types to different children.
////////////////////////////////////////////////////////////////////
void DataValve::
set_fine_control(int child_index, TypeHandle data_type, Control *control) {
  ensure_child_index(child_index);
  nassertv(child_index >= 0 && child_index < (int)_controls.size());
  _controls[child_index]._fine_controls[data_type] = control;
}

////////////////////////////////////////////////////////////////////
//     Function: DataValve::clear_fine_control
//       Access: Published
//  Description: Removes the particular fine control set by
//               set_fine_control() for the indicated child index and
//               data type.  Data of this type will now be controlled
//               by the Control indicated by set_control() for this
//               child, or by set_default_control() if nothing is
//               indicated by set_control().
////////////////////////////////////////////////////////////////////
void DataValve::
clear_fine_control(int child_index, TypeHandle data_type) {
  nassertv(child_index >= 0);
  if (child_index < (int)_controls.size()) {
    _controls[child_index]._fine_controls.erase(data_type);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DataValve::has_fine_control
//       Access: Published
//  Description: Returns true if the indicated child has a particular
//               fine control set for the indicated data type, or
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool DataValve::
has_fine_control(int child_index, TypeHandle data_type) const {
  nassertr(child_index >= 0, false);
  if (child_index < (int)_controls.size()) {
    return _controls[child_index]._fine_controls.count(data_type) != 0;
  }
  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: DataValve::get_fine_control
//       Access: Published
//  Description: Returns the fine control that has been set for the
//               indicated child and data type, or NULL if no such
//               fine control has been set.
////////////////////////////////////////////////////////////////////
DataValve::Control *DataValve::
get_fine_control(int child_index, TypeHandle data_type) const {
  nassertr(child_index >= 0, NULL);
  if (child_index < (int)_controls.size()) {
    FineControls::const_iterator fci = 
      _controls[child_index]._fine_controls.find(data_type);
    if (fci != _controls[child_index]._fine_controls.end()) {
      return (*fci).second;
    }
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DataValve::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DataValve::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "DataValve " << get_name() << ": (default control is "
    << *_default_control << ")\n";

  size_t i;
  for (i = 0; i < _controls.size(); i++) {
    if (has_control(i)) {
      const Child &child = _controls[i];
      indent(out, indent_level + 2)
	<< "Child " << i << ": ";

      if (child._control.is_null()) {
	out << "(default)";
      } else {
	out << *child._control;
      }
      out << "\n";
      
      FineControls::const_iterator fci;
      for (fci = child._fine_controls.begin(); 
	   fci != child._fine_controls.end();
	   ++fci) {
	indent(out, indent_level + 4)
	  << (*fci).first << " " << *(*fci).second;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DataValve::ensure_child_index
//       Access: Private
//  Description: Guarantees that we have at least enough children in
//               the _control vector to describe the indicated
//               child_index.
////////////////////////////////////////////////////////////////////
void DataValve::
ensure_child_index(int child_index) {
  _controls.reserve(child_index + 1);
  while ((int)_controls.size() <= child_index) {
    _controls.push_back(Child());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DataValve::transmit_data
//       Access: Public, Virtual
//  Description: Called once before transmit_data_per_child() is
//               called for each child, this gives the node a chance
//               to process its inputs.
////////////////////////////////////////////////////////////////////
void DataValve::
transmit_data(NodeAttributes &data) {
  // Update our modifier buttons during the overall pass.
  const ButtonEventDataAttribute *b;
  if (get_attribute_into(b, data, _button_events_type)) {
    b->update_mods(_mods);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DataValve::transmit_data_per_child
//       Access: Public, Virtual
//  Description: Should be overridden in a derived class that wants to
//               send a different data stream to each child.
//               Normally, a node only overrides transmit_data(),
//               which takes a set of input data attributes and
//               generates a set of output data attributes.  A node
//               may also override transmit_data_per_child(), which is
//               called after transmit_data(), once per child; this
//               function may be used to send individual data
//               attributes to each child.
////////////////////////////////////////////////////////////////////
void DataValve::
transmit_data_per_child(NodeAttributes &data, int child_index) {
  // Consider which data this child wants to see.
  static const FineControls empty_controls;

  Control *control = _default_control;
  const FineControls *fine_controls = &empty_controls;

  if (child_index < (int)_controls.size()) {
    if (!_controls[child_index]._control.is_null()) {
      control = _controls[child_index]._control;
    }
    fine_controls = &_controls[child_index]._fine_controls;
  }

  if (control->is_on(*this)) {
    if (fine_controls->empty()) {
      // The control is on, and we have no further fine-tuning.  All
      // data goes to the indicated node.
      return;

    } else {
      // The control is on, and we have some fine-tuning.  Any data
      // type explicitly "off" should be yanked.
      FineControls::const_iterator fci;
      for (fci = fine_controls->begin(); fci != fine_controls->end(); ++fci) {
	TypeHandle data_type = (*fci).first;
	Control *fcontrol = (*fci).second;

	if (!fcontrol->is_on(*this)) {
	  data.clear_attribute(data_type);
	}
      }
    }

  } else {
    if (fine_controls->empty()) {
      // The control is off, and we have no further fine-tuning.  All
      // data gets eliminated.
      data.clear();

    } else {
      // The control is off, and we have some fine-tuning.  Any data
      // type explicitly "on" should be preserved.
      NodeAttributes temp;

      FineControls::const_iterator fci;
      for (fci = fine_controls->begin(); fci != fine_controls->end(); ++fci) {
	TypeHandle data_type = (*fci).first;
	Control *fcontrol = (*fci).second;

	if (fcontrol->is_on(*this)) {
	  NodeAttribute *attrib = data.get_attribute(data_type);
	  if (attrib != (NodeAttribute *)NULL) {
	    temp.set_attribute(data_type, attrib);
	  }
	}
      }

      data = temp;
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: DataValve::init_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
void DataValve::
init_type() {
  DataNode::init_type();
  register_type(_type_handle, "DataValve",
		DataNode::get_class_type());

  ButtonEventDataTransition::init_type();
  register_data_transition(_button_events_type, "ButtonEvents",
			   ButtonEventDataTransition::get_class_type());
}
