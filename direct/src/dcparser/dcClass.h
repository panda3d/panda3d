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

#ifdef WITHIN_PANDA
#include "pStatCollector.h"
#include "configVariableBool.h"

extern ConfigVariableBool dc_multiple_inheritance;

#else  // WITHIN_PANDA

static const bool dc_multiple_inheritance = true;

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

  const string &get_name() const;
  int get_number() const;

  bool has_parent() const;
  DCClass *get_parent() const;
  
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

  INLINE void start_generate();
  INLINE void stop_generate();

  virtual void output(ostream &out) const;
  
#ifdef HAVE_PYTHON
  bool has_class_def() const;
  void set_class_def(PyObject *class_def);
  PyObject *get_class_def() const;

  void receive_update(PyObject *distobj, DatagramIterator &di) const;
  void receive_update_broadcast_required(PyObject *distobj, DatagramIterator &di) const;
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
                                int do_id, PyObject *args) const;
  Datagram ai_format_update(const string &field_name, int do_id, 
                            CHANNEL_TYPE to_id, CHANNEL_TYPE from_id, PyObject *args) const;
  Datagram ai_format_generate(PyObject *distobj, int do_id, int parent_id, int zone_id,
                              CHANNEL_TYPE district_channel_id, CHANNEL_TYPE from_channel_id,
                              PyObject *optional_fields) const;
  Datagram client_format_generate(PyObject *distobj, int do_id, int zone_id,                              
                                  PyObject *optional_fields) const;

  Datagram ai_database_generate_context(unsigned int context_id, unsigned int parent_id, unsigned int zone_id,
                                CHANNEL_TYPE database_server_id, CHANNEL_TYPE from_channel_id) const;
  
#endif 

public:
  virtual void output(ostream &out, bool brief) const;
  virtual void write(ostream &out, bool brief, int indent_level) const;
  void output_instance(ostream &out, bool brief, const string &prename, 
                       const string &name, const string &postname) const;
  void generate_hash(HashGenerator &hashgen) const;

  bool add_field(DCField *field);
  void add_parent(DCClass *parent);
  void set_number(int number);

private:
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
  Fields _fields;

  typedef pmap<string, DCField *> FieldsByName;
  FieldsByName _fields_by_name;

  typedef pmap<int, DCField *> FieldsByIndex;
  FieldsByIndex _fields_by_index;

#ifdef HAVE_PYTHON
  PyObject *_class_def;
#endif

  friend class DCField;
};

#include "dcClass.I"

#endif
