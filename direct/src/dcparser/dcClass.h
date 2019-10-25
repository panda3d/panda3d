/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcClass.h
 * @author drose
 * @date 2000-10-05
 */

#ifndef DCCLASS_H
#define DCCLASS_H

#include "dcbase.h"
#include "dcField.h"
#include "dcDeclaration.h"

#ifdef WITHIN_PANDA
#include "pStatCollector.h"
#include "configVariableBool.h"
#include "extension.h"
#include "datagramIterator.h"

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

/**
 * Defines a particular DistributedClass as read from an input .dc file.
 */
class EXPCL_DIRECT_DCPARSER DCClass : public DCDeclaration {
public:
  DCClass(DCFile *dc_file, const std::string &name,
          bool is_struct, bool bogus_class);
  ~DCClass();

PUBLISHED:
  virtual DCClass *as_class();
  virtual const DCClass *as_class() const;

  INLINE DCFile *get_dc_file() const;

  INLINE const std::string &get_name() const;
  INLINE int get_number() const;

  int get_num_parents() const;
  DCClass *get_parent(int n) const;

  bool has_constructor() const;
  DCField *get_constructor() const;

  int get_num_fields() const;
  DCField *get_field(int n) const;

  DCField *get_field_by_name(const std::string &name) const;
  DCField *get_field_by_index(int index_number) const;

  int get_num_inherited_fields() const;
  DCField *get_inherited_field(int n) const;

  INLINE bool is_struct() const;
  INLINE bool is_bogus_class() const;
  bool inherits_from_bogus_class() const;

  INLINE void start_generate();
  INLINE void stop_generate();

  virtual void output(std::ostream &out) const;

  EXTENSION(bool has_class_def() const);
  EXTENSION(void set_class_def(PyObject *class_def));
  EXTENSION(PyObject *get_class_def() const);
  EXTENSION(bool has_owner_class_def() const);
  EXTENSION(void set_owner_class_def(PyObject *owner_class_def));
  EXTENSION(PyObject *get_owner_class_def() const);

  EXTENSION(void receive_update(PyObject *distobj, DatagramIterator &di) const);
  EXTENSION(void receive_update_broadcast_required(PyObject *distobj, DatagramIterator &di) const);
  EXTENSION(void receive_update_broadcast_required_owner(PyObject *distobj, DatagramIterator &di) const);
  EXTENSION(void receive_update_all_required(PyObject *distobj, DatagramIterator &di) const);
  EXTENSION(void receive_update_other(PyObject *distobj, DatagramIterator &di) const);

  EXTENSION(void direct_update(PyObject *distobj, const std::string &field_name,
                               const vector_uchar &value_blob));
  EXTENSION(void direct_update(PyObject *distobj, const std::string &field_name,
                               const Datagram &datagram));
  EXTENSION(bool pack_required_field(Datagram &datagram, PyObject *distobj,
                                     const DCField *field) const);
  EXTENSION(bool pack_required_field(DCPacker &packer, PyObject *distobj,
                                     const DCField *field) const);



  EXTENSION(Datagram client_format_update(const std::string &field_name,
                                          DOID_TYPE do_id, PyObject *args) const);
  EXTENSION(Datagram ai_format_update(const std::string &field_name,
                                      DOID_TYPE do_id,
                                      CHANNEL_TYPE to_id, CHANNEL_TYPE from_id,
                                      PyObject *args) const);
  EXTENSION(Datagram ai_format_update_msg_type(const std::string &field_name,
                                               DOID_TYPE do_id,
                                               CHANNEL_TYPE to_id,
                                               CHANNEL_TYPE from_id,
                                               int msg_type,
                                               PyObject *args) const);
  EXTENSION(Datagram ai_format_generate(PyObject *distobj, DOID_TYPE do_id,
                                        ZONEID_TYPE parent_id,
                                        ZONEID_TYPE zone_id,
                                        CHANNEL_TYPE district_channel_id,
                                        CHANNEL_TYPE from_channel_id,
                                        PyObject *optional_fields) const);
  EXTENSION(Datagram client_format_generate_CMU(PyObject *distobj, DOID_TYPE do_id,
                                                ZONEID_TYPE zone_id,
                                                PyObject *optional_fields) const);


public:
  virtual void output(std::ostream &out, bool brief) const;
  virtual void write(std::ostream &out, bool brief, int indent_level) const;
  void output_instance(std::ostream &out, bool brief, const std::string &prename,
                       const std::string &name, const std::string &postname) const;
  void generate_hash(HashGenerator &hashgen) const;
  void clear_inherited_fields();
  void rebuild_inherited_fields();

  bool add_field(DCField *field);
  void add_parent(DCClass *parent);
  void set_number(int number);

private:
  void shadow_inherited_field(const std::string &name);

#ifdef WITHIN_PANDA
  mutable PStatCollector _class_update_pcollector;
  mutable PStatCollector _class_generate_pcollector;
  static PStatCollector _update_pcollector;
  static PStatCollector _generate_pcollector;
#endif

  DCFile *_dc_file;

  std::string _name;
  bool _is_struct;
  bool _bogus_class;
  int _number;

  typedef pvector<DCClass *> Parents;
  Parents _parents;

  DCField *_constructor;

  typedef pvector<DCField *> Fields;
  Fields _fields, _inherited_fields;

  typedef pmap<std::string, DCField *> FieldsByName;
  FieldsByName _fields_by_name;

  typedef pmap<int, DCField *> FieldsByIndex;
  FieldsByIndex _fields_by_index;

  // See pandaNode.h for an explanation of this trick
  class PythonClassDefs : public ReferenceCount {
  public:
    virtual ~PythonClassDefs() {};
  };
  PT(PythonClassDefs) _python_class_defs;

  friend class DCField;
#ifdef WITHIN_PANDA
  friend class Extension<DCClass>;
#endif
};

#include "dcClass.I"

#endif
