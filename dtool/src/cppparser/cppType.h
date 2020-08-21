/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppType.h
 * @author drose
 * @date 1999-10-19
 */

#ifndef CPPTYPE_H
#define CPPTYPE_H

#include "dtoolbase.h"

#include "cppDeclaration.h"

#include <set>

class CPPType;
class CPPTypedefType;
class CPPTypeDeclaration;


// This is an STL function object used to uniquely order CPPType pointers.
class CPPTypeCompare {
public:
  bool operator () (CPPType *a, CPPType *b) const;
};

/**
 *
 */
class CPPType : public CPPDeclaration {
public:
  typedef std::vector<CPPTypedefType *> Typedefs;
  Typedefs _typedefs;

  CPPType(const CPPFile &file);

  virtual CPPType *resolve_type(CPPScope *current_scope,
                                CPPScope *global_scope);

  virtual bool is_tbd() const;
  virtual bool is_fundamental() const;
  virtual bool is_standard_layout() const;
  virtual bool is_trivial() const;
  virtual bool is_constructible(const CPPType *type) const;
  virtual bool is_default_constructible() const;
  virtual bool is_copy_constructible() const;
  virtual bool is_copy_assignable() const;
  virtual bool is_destructible() const;
  virtual bool is_parameter_expr() const;

  // Convenience methods.
  bool is_enum() const;
  bool is_const() const;
  bool is_reference() const;
  bool is_pointer() const;

  CPPType *remove_const();
  inline CPPType *remove_volatile() { return this; }
  inline CPPType *remove_cv() { return remove_const(); };
  CPPType *remove_reference();
  CPPType *remove_pointer();

  bool has_typedef_name() const;
  std::string get_typedef_name(CPPScope *scope = nullptr) const;

  virtual std::string get_simple_name() const;
  virtual std::string get_local_name(CPPScope *scope = nullptr) const;
  virtual std::string get_fully_scoped_name() const;
  virtual std::string get_preferred_name() const;
  int get_num_alt_names() const;
  std::string get_alt_name(int n) const;

  virtual bool is_incomplete() const;
  virtual bool is_convertible_to(const CPPType *other) const;
  virtual bool is_equivalent(const CPPType &other) const;

  void output_instance(std::ostream &out, const std::string &name,
                       CPPScope *scope) const;
  virtual void output_instance(std::ostream &out, int indent_level,
                               CPPScope *scope,
                               bool complete, const std::string &prename,
                               const std::string &name) const;

  virtual CPPType *as_type();


  static CPPType *new_type(CPPType *type);

  static void record_alt_name_for(const CPPType *type, const std::string &name);
  static std::string get_preferred_name_for(const CPPType *type);

  CPPTypeDeclaration *_declaration;
  bool _forcetype;

protected:
  typedef std::set<CPPType *, CPPTypeCompare> Types;
  static Types _types;

  typedef std::map<std::string, std::string> PreferredNames;
  static PreferredNames _preferred_names;

  typedef std::vector<std::string> Names;
  typedef std::map<std::string, Names> AltNames;
  static AltNames _alt_names;
};

#endif
