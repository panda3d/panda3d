// Filename: dcClass.h
// Created by:  drose (05Oct00)
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

#ifndef DCCLASS_H
#define DCCLASS_H

#include "dcbase.h"
#include "dcField.h"
#include "dcDeclaration.h"
#include "dcPython.h"

class HashGenerator;
class DCParameter;

////////////////////////////////////////////////////////////////////
//       Class : DCClass
// Description : Defines a particular DistributedClass as read from an
//               input .dc file.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCClass : public DCDeclaration {
PUBLISHED:
  const string &get_name() const;
  int get_number() const;

  bool has_parent() const;
  DCClass *get_parent() const;

  int get_num_fields() const;
  DCField *get_field(int n) const;
  DCField *get_field_by_name(const string &name) const;

  int get_num_inherited_fields() const;
  DCField *get_inherited_field(int n) const;

  int get_num_parameters() const;
  DCParameter *get_parameter(int n) const;
  DCParameter *get_parameter_by_name(const string &name) const;

  int get_num_inherited_parameters() const;
  DCParameter *get_inherited_parameter(int n) const;

  bool is_struct() const;
  bool is_bogus_class() const;

#ifdef HAVE_PYTHON
  void set_class_def(PyObject *class_def);
  PyObject *get_class_def() const;

  void receive_update(PyObject *distobj, DatagramIterator &iterator) const;
  void receive_update_broadcast_required(PyObject *distobj, DatagramIterator &iterator) const;
  void receive_update_all_required(PyObject *distobj, DatagramIterator &iterator) const;
  void receive_update_other(PyObject *distobj, DatagramIterator &iterator) const;
  void direct_update(PyObject *distobj, const string &field_name, 
                     const string &value_blob);
  void direct_update(PyObject *distobj, const string &field_name, 
                     const Datagram &datagram);
  bool pack_required_field(Datagram &dg, PyObject *distobj, 
                           DCField *field) const;


  Datagram client_format_update(const string &field_name, int do_id, 
                                PyObject *args) const;
  Datagram ai_format_update(const string &field_name, int do_id, 
                            int to_id, int from_id, PyObject *args) const;
  Datagram ai_format_generate(PyObject *distobj, int do_id, int zone_id,
                              int district_id, int from_channel_id,
                              PyObject *optional_fields) const;
#endif 

public:
  DCClass(const string &name, bool is_struct, bool bogus_class);
  ~DCClass();

  virtual void write(ostream &out, bool brief, int indent_level) const;
  void generate_hash(HashGenerator &hashgen) const;

  bool add_field(DCField *field);
  bool add_parameter(DCParameter *parameter);
  void add_parent(DCClass *parent);
  void set_number(int number);

private:
  string _name;
  bool _is_struct;
  bool _bogus_class;
  int _number;

  typedef pvector<DCClass *> Parents;
  Parents _parents;

  typedef pvector<DCField *> Fields;
  Fields _fields;

  typedef pmap<string, DCField *> FieldsByName;
  FieldsByName _fields_by_name;

  typedef pvector<DCParameter *> Parameters;
  Parameters _parameters;

  typedef pmap<string, DCParameter *> ParametersByName;
  ParametersByName _parameters_by_name;

#ifdef HAVE_PYTHON
  PyObject *_class_def;
#endif
};

#endif
