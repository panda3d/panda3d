// Filename: dcSwitch.h
// Created by:  drose (23Jun04)
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

#ifndef DCSWITCH_H
#define DCSWITCH_H

#include "dcbase.h"
#include "dcDeclaration.h"
#include "dcPackerInterface.h"

class DCParameter;
class HashGenerator;
class DCField;

////////////////////////////////////////////////////////////////////
//       Class : DCSwitch
// Description : This represents a switch statement, which can appear
//               inside a class body and represents two or more
//               alternative unpacking schemes based on the first
//               field read.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCSwitch : public DCDeclaration {
public:
  DCSwitch(const string &name, DCParameter *key_parameter);
  virtual ~DCSwitch();

PUBLISHED:
  virtual DCSwitch *as_switch();
  virtual const DCSwitch *as_switch() const;

  const string &get_name() const;
  DCParameter *get_key_parameter() const;

  int get_num_cases() const;
  int get_case_by_value(const string &case_value) const;
  DCPackerInterface *get_case(int n) const;

  string get_value(int case_index) const;
  int get_num_fields(int case_index) const;
  DCField *get_field(int case_index, int n) const;
  DCField *get_field_by_name(int case_index, const string &name) const;

public:
  int add_case(const string &value);
  bool add_field(DCField *field);

  const DCPackerInterface *apply_switch(const char *value_data, size_t length) const;

  virtual void write(ostream &out, bool brief, int indent_level) const;
  void output_instance(ostream &out, bool brief, const string &prename, 
                       const string &name, const string &postname) const;
  void write_instance(ostream &out, bool brief, int indent_level,
                      const string &prename, const string &name,
                      const string &postname) const;
  virtual void generate_hash(HashGenerator &hashgen) const;
  virtual bool pack_default_value(DCPackData &pack_data, bool &pack_error) const;

  bool do_check_match_switch(const DCSwitch *other) const;

public:
  typedef pvector<DCField *> Fields;
  typedef pmap<string, DCField *> FieldsByName;

  class SwitchCase : public DCPackerInterface {
  public:
    SwitchCase(const string &name, const string &value);
    ~SwitchCase();
    virtual DCPackerInterface *get_nested_field(int n) const;

    bool add_field(DCField *field);
    bool do_check_match_switch_case(const SwitchCase *other) const;

  protected:
    virtual bool do_check_match(const DCPackerInterface *other) const;

  public:
    string _value;
    Fields _fields;
    FieldsByName _fields_by_name;
    bool _has_default_value;
  };

private:
  string _name;
  DCParameter *_key_parameter;

  typedef pvector<SwitchCase *> Cases;
  Cases _cases;

  typedef pmap<string, int> CasesByValue;
  CasesByValue _cases_by_value;
};

#endif
