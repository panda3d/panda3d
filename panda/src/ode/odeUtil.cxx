// Filename: odeUtil.cxx
// Created by:  joswilso (27Dec06)
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

#include "odeUtil.h"

#ifdef HAVE_PYTHON
  #include "py_panda.h"
  #include "typedReferenceCount.h"
  #ifndef CPPPARSER
    extern EXPCL_PANDAODE Dtool_PyTypedObject Dtool_OdeGeom;
  #endif
#endif

dReal OdeUtil::OC_infinity = dInfinity;
PyObject* OdeUtil::_python_callback = NULL;

////////////////////////////////////////////////////////////////////
//     Function: OdeUtil::get_connecting_joint
//       Access: Public, Static
//  Description: Returns the joint that connects the given bodies.
////////////////////////////////////////////////////////////////////
OdeJoint OdeUtil::
get_connecting_joint(const OdeBody &body1, const OdeBody &body2) {
  return OdeJoint(dConnectingJoint(body1.get_id(),body2.get_id()));
}

////////////////////////////////////////////////////////////////////
//     Function: OdeUtil::get_connecting_joint_list
//       Access: Public, Static
//  Description: Returns a collection of joints connecting the
//               specified bodies.
////////////////////////////////////////////////////////////////////
OdeJointCollection OdeUtil::
get_connecting_joint_list(const OdeBody &body1, const OdeBody &body2) {
  const int max_possible_joints = min(body1.get_num_joints(), body1.get_num_joints());
  
  dJointID *joint_list = (dJointID *)PANDA_MALLOC_ARRAY(max_possible_joints * sizeof(dJointID));
  int num_joints = dConnectingJointList(body1.get_id(), body2.get_id(),
          joint_list);
  OdeJointCollection joints;
  for (int i = 0; i < num_joints; i++) {
    joints.add_joint(OdeJoint(joint_list[i]));
  }
  
  PANDA_FREE_ARRAY(joint_list);
  return joints;
}

////////////////////////////////////////////////////////////////////
//     Function: OdeUtil::are_connected
//       Access: Public, Static
//  Description: Returns 1 if the given bodies are connected
//               by a joint, returns 0 otherwise.
////////////////////////////////////////////////////////////////////
int OdeUtil::
are_connected(const OdeBody &body1, const OdeBody &body2) {
  return dAreConnected(body1.get_id(),body2.get_id());
}

////////////////////////////////////////////////////////////////////
//     Function: OdeUtil::are_connected_excluding
//       Access: Public, Static
//  Description: Returns 1 if the given bodies are connected
//               by a joint that does not match the given
//               joint_type, returns 0 otherwise. This is useful
//               for deciding whether to add contact joints between
//               two bodies: if they are already connected by
//               non-contact joints then it may not be appropriate
//               to add contacts, however it is okay to add more
//               contact between bodies that already have contacts.
////////////////////////////////////////////////////////////////////
int OdeUtil::
are_connected_excluding(const OdeBody &body1,
                        const OdeBody &body2,
                        const int joint_type) {
  return dAreConnectedExcluding(body1.get_id(),
        body2.get_id(),
        joint_type);
}

////////////////////////////////////////////////////////////////////
//     Function: OdeUtil::collide
//       Access: Public, Static
//  Description: Given two geometry objects that potentially touch
//               (geom1 and geom2), generate contact information
//               for them. Returns a collection of OdeContacts.
////////////////////////////////////////////////////////////////////
OdeContactCollection OdeUtil::
collide(const OdeGeom &geom1, const OdeGeom &geom2, const short int max_contacts) {
  dContactGeom *contact_list = (dContactGeom *)PANDA_MALLOC_ARRAY(max_contacts * sizeof(dContactGeom));
  int num_contacts = dCollide(geom1.get_id(), geom2.get_id(), max_contacts, contact_list, sizeof(contact_list));
  OdeContactCollection contacts;
  for (int i = 0; i < num_contacts; i++) {
    PT(OdeContact) contact = new OdeContact();
    contact->set_geom(OdeContactGeom(contact_list[i]));
    contacts.add_contact(contact);
  }
  
  PANDA_FREE_ARRAY(contact_list);
  return contacts;
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: OdeUtil::collide2
//       Access: Public, Static
//  Description: Calls the callback for all potentially intersecting
//               pairs that contain one geom from geom1 and one geom
//               from geom2.
////////////////////////////////////////////////////////////////////
int OdeUtil::
collide2(const OdeGeom &geom1, const OdeGeom &geom2, PyObject* arg, PyObject* callback) {
  nassertr(callback != NULL, -1);
  if (!PyCallable_Check(callback)) {
    PyErr_Format(PyExc_TypeError, "'%s' object is not callable", callback->ob_type->tp_name);
    return -1;
  } else {
    _python_callback = (PyObject*) callback;
    Py_XINCREF(_python_callback);
    dSpaceCollide2(geom1.get_id(), geom2.get_id(), (void*) arg, &near_callback);
    Py_XDECREF(_python_callback);
    return 0;
  }
}

void OdeUtil::
near_callback(void *data, dGeomID o1, dGeomID o2) {
  ode_cat.spam() << "near_callback called, data: " << data << ", dGeomID1: " << o1 << ", dGeomID2: " << o2 << "\n";
  OdeGeom g1 (o1);
  OdeGeom g2 (o2);
  PyObject* p1 = DTool_CreatePyInstanceTyped(&g1, Dtool_OdeGeom, true, false, g1.get_type_index());
  PyObject* p2 = DTool_CreatePyInstanceTyped(&g2, Dtool_OdeGeom, true, false, g2.get_type_index());
  PyObject* result = PyEval_CallFunction(_python_callback, "OOO", (PyObject*) data, p1, p2);
  if (!result) {
    ode_cat.error() << "An error occurred while calling python function!\n";
    PyErr_Print();
  }
}
#endif

OdeGeom OdeUtil::
space_to_geom(const OdeSpace &space) {
  return OdeGeom((dGeomID)space.get_id());
}
