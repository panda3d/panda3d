/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcSwitch.h
 * @author drose
 * @date 2004-06-23
 */

#ifndef DCSWITCH_H
#define DCSWITCH_H

#include "dcbase.h"
#include "dcDeclaration.h"
#include "dcPackerInterface.h"

class DCParameter;
class HashGenerator;
class DCField;

/**
 * This represents a switch statement, which can appear inside a class body
 * and represents two or more alternative unpacking schemes based on the first
 * field read.
 */
class EXPCL_DIRECT_DCPARSER DCSwitch : public DCDeclaration {
public:
  DCSwitch(const std::string &name, DCField *key_parameter);
  virtual ~DCSwitch();

PUBLISHED:
  virtual DCSwitch *as_switch();
  virtual const DCSwitch *as_switch() const;

  const std::string &get_name() const;
  DCField *get_key_parameter() const;

  int get_num_cases() const;
  int get_case_by_value(const vector_uchar &case_value) const;
  DCPackerInterface *get_case(int n) const;
  DCPackerInterface *get_default_case() const;

  vector_uchar get_value(int case_index) const;
  int get_num_fields(int case_index) const;
  DCField *get_field(int case_index, int n) const;
  DCField *get_field_by_name(int case_index, const std::string &name) const;

public:
  bool is_field_valid() const;
  int add_case(const vector_uchar &value);
  void add_invalid_case();
  bool add_default();
  bool add_field(DCField *field);
  void add_break();

  const DCPackerInterface *apply_switch(const char *value_data, size_t length) const;

  virtual void output(std::ostream &out, bool brief) const;
  virtual void write(std::ostream &out, bool brief, int indent_level) const;
  void output_instance(std::ostream &out, bool brief, const std::string &prename,
                       const std::string &name, const std::string &postname) const;
  void write_instance(std::ostream &out, bool brief, int indent_level,
                      const std::string &prename, const std::string &name,
                      const std::string &postname) const;
  virtual void generate_hash(HashGenerator &hashgen) const;
  virtual bool pack_default_value(DCPackData &pack_data, bool &pack_error) const;

  bool do_check_match_switch(const DCSwitch *other) const;

public:
  typedef pvector<DCField *> Fields;
  typedef pmap<std::string, DCField *> FieldsByName;

  class SwitchFields : public DCPackerInterface {
  public:
    SwitchFields(const std::string &name);
    ~SwitchFields();
    virtual DCPackerInterface *get_nested_field(int n) const;

    bool add_field(DCField *field);
    bool do_check_match_switch_case(const SwitchFields *other) const;

    void output(std::ostream &out, bool brief) const;
    void write(std::ostream &out, bool brief, int indent_level) const;

  protected:
    virtual bool do_check_match(const DCPackerInterface *other) const;

  public:
    Fields _fields;
    FieldsByName _fields_by_name;
    bool _has_default_value;
  };

  class SwitchCase {
  public:
    SwitchCase(const vector_uchar &value, SwitchFields *fields);
    ~SwitchCase();

    bool do_check_match_switch_case(const SwitchCase *other) const;

  public:
    vector_uchar _value;
    SwitchFields *_fields;
  };

private:
  SwitchFields *start_new_case();

private:
  std::string _name;
  DCField *_key_parameter;

  typedef pvector<SwitchCase *> Cases;
  Cases _cases;
  SwitchFields *_default_case;

  // All SwitchFields created and used by the DCSwitch object are also stored
  // here; this is the vector that "owns" the pointers.
  typedef pvector<SwitchFields *> CaseFields;
  CaseFields _case_fields;

  // All nested DCField objects that have been added to one or more of the
  // above SwitchFields are also recorded here; this is the vector that "owns"
  // these pointers.
  Fields _nested_fields;

  // These are the SwitchFields that are currently being filled up during this
  // stage of the parser.  There might be more than one at a time, if we have
  // multiple cases being introduced in the middle of a series of fields
  // (without a break statement intervening).
  CaseFields _current_fields;
  bool _fields_added;

  // This map indexes into the _cases vector, above.
  typedef pmap<vector_uchar, int> CasesByValue;
  CasesByValue _cases_by_value;
};

#endif
