// Filename: dcClass.h
// Created by:  drose (05Oct00)
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

#ifndef DCCLASS_H
#define DCCLASS_H

#include "dcbase.h"
#include "dcField.h"
#include "dcDeclaration.h"
#include "dcPython.h"

#ifdef WITHIN_PANDA
#include "pStatCollector.h"
#include "configVariableBool.h"

extern ConfigVariableBool dc_multiple_inheritance;
extern ConfigVariableBool dc_virtual_inheritance;
extern ConfigVariableBool dc_sort_inheritance_by_file;

#else  // WITHIN_PANDA

static const bool dc_multiple_inheritance = true;
static const bool dc_virtual_inheritance = true;
static const bool dc_sort_inheritance_by_file = false;

#endif  // WITHIN_PANDA

class HashGenerator;
class DCParameter;

////////////////////////////////////////////////////////////////////
//       Class : DCClass
// Description : Defines a particular DistributedClass as read from an
//               input .dc file.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCClass : public DCDeclaration {
public:
  DCClass(DCFile *dc_file, const string &name, 
          bool is_struct, bool bogus_class);
  ~DCClass();

PUBLISHED:
  virtual DCClass *as_class();
  virtual const DCClass *as_class() const;

  INLINE DCFile *get_dc_file() const;

  INLINE const string &get_name() const;
  INLINE int get_number() const;

  int get_num_parents() const;
  DCClass *get_parent(int n) const;
  
  bool has_constructor() const;
  DCField *get_constructor() const;

  int get_num_fields() const;
  DCField *get_field(int n) const;

  DCField *get_field_by_name(const string &name) const;
  DCField *get_field_by_index(int index_number) const;

  int get_num_inherited_fields() const;
  DCField *get_inherited_field(int n) const;

  INLINE bool is_struct() const;
  INLINE bool is_bogus_class() const;
  bool inherits_from_bogus_class() const;

  INLINE void start_generate();
  INLINE void stop_generate();

  virtual void output(ostream &out) const;
  
#ifdef HAVE_PYTHON
  bool has_class_def() const;
  void set_class_def(PyObject *class_def);
  PyObject *get_class_def() const;
  bool has_owner_class_def() const;
  void set_owner_class_def(PyObject *owner_class_def);
  PyObject *get_owner_class_def() const;

  void receive_update(PyObject *distobj, DatagramIterator &di) const;
  void receive_update_broadcast_required(PyObject *distobj, DatagramIterator &di) const;
  void receive_update_broadcast_required_owner(PyObject *distobj, DatagramIterator &di) const;
  void receive_update_all_required(PyObject *distobj, DatagramIterator &di) const;
  void receive_update_other(PyObject *distobj, DatagramIterator &di) const;

  void direct_update(PyObject *distobj, const string &field_name, 
                     const string &value_blob);
  void direct_update(PyObject *distobj, const string &field_name, 
                     const Datagram &datagram);
  bool pack_required_field(Datagram &datagram, PyObject *distobj, 
                           const DCField *field) const;
  bool pack_required_field(DCPacker &packer, PyObject *distobj, 
                           const DCField *field) const;



  Datagram client_format_update(const string &field_name,
                                DOID_TYPE do_id, PyObject *args) const;
  Datagram ai_format_update(const string &field_name, DOID_TYPE do_id, 
                            CHANNEL_TYPE to_id, CHANNEL_TYPE from_id, PyObject *args) const;
  Datagram ai_format_update_msg_type(const string &field_name, DOID_TYPE do_id, 
                            CHANNEL_TYPE to_id, CHANNEL_TYPE from_id, int msg_type, PyObject *args) const;
  Datagram ai_format_generate(PyObject *distobj, DOID_TYPE do_id, ZONEID_TYPE parent_id, ZONEID_TYPE zone_id,
                              CHANNEL_TYPE district_channel_id, CHANNEL_TYPE from_channel_id,
                              PyObject *optional_fields) const;
  Datagram client_format_generate_CMU(PyObject *distobj, DOID_TYPE do_id, 
                                      ZONEID_TYPE zone_id,                                                           PyObject *optional_fields) const;

#endif 

public:
  virtual void output(ostream &out, bool brief) const;
  virtual void write(ostream &out, bool brief, int indent_level) const;
  void output_instance(ostream &out, bool brief, const string &prename, 
                       const string &name, const string &postname) const;
  void generate_hash(HashGenerator &hashgen) const;
  void clear_inherited_fields();
  void rebuild_inherited_fields();

  bool add_field(DCField *field);
  void add_parent(DCClass *parent);
  void set_number(int number);

private:
  void shadow_inherited_field(const string &name);

#ifdef WITHIN_PANDA
  PStatCollector _class_update_pcollector;
  PStatCollector _class_generate_pcollector;
  static PStatCollector _update_pcollector;
  static PStatCollector _generate_pcollector;
#endif

  DCFile *_dc_file;

  string _name;
  bool _is_struct;
  bool _bogus_class;
  int _number;

  typedef pvector<DCClass *> Parents;
  Parents _parents;

  DCField *_constructor;

  typedef pvector<DCField *> Fields;
  Fields _fields, _inherited_fields;

  typedef pmap<string, DCField *> FieldsByName;
  FieldsByName _fields_by_name;

  typedef pmap<int, DCField *> FieldsByIndex;
  FieldsByIndex _fields_by_index;

#ifdef HAVE_PYTHON
  PyObject *_class_def;
  PyObject *_owner_class_def;
#endif

  friend class DCField;
};

#include "dcClass.I"

#endif
