// Filename: dcParameter.h
// Created by:  drose (15Jun04)
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

#ifndef DCPARAMETER_H
#define DCPARAMETER_H

#include "dcbase.h"
#include "dcField.h"

class DCSimpleParameter;
class DCClassParameter;
class DCArrayParameter;
class DCTypedef;
class HashGenerator;

////////////////////////////////////////////////////////////////////
//       Class : DCParameter
// Description : Represents the type specification for a single
//               parameter within a field specification.  This may be
//               a simple type, or it may be a class or an array
//               reference.
//
//               This may also be a typedef reference to another type,
//               which has the same properties as the referenced type,
//               but a different name.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT DCParameter : public DCField {
protected:
  DCParameter();
  DCParameter(const DCParameter &copy);
public:
  virtual ~DCParameter();

PUBLISHED:
  virtual DCParameter *as_parameter();
  virtual DCSimpleParameter *as_simple_parameter();
  virtual DCClassParameter *as_class_parameter();
  virtual DCSwitchParameter *as_switch_parameter();
  virtual DCArrayParameter *as_array_parameter();

  virtual DCParameter *make_copy() const=0;
  virtual bool is_valid() const=0;

  const DCTypedef *get_typedef() const;

public:
  void set_typedef(const DCTypedef *dtypedef);

  virtual void output(ostream &out, bool brief) const;
  virtual void write(ostream &out, bool brief, int indent_level) const;
  virtual void output_instance(ostream &out, bool brief, const string &prename, 
                               const string &name, const string &postname) const=0;
  virtual void write_instance(ostream &out, bool brief, int indent_level,
                              const string &prename, const string &name,
                              const string &postname) const;
  void output_typedef_name(ostream &out, bool brief, const string &prename, 
                           const string &name, const string &postname) const;
  void write_typedef_name(ostream &out, bool brief, int indent_level,
                          const string &prename, const string &name, 
                          const string &postname) const;
  virtual void generate_hash(HashGenerator &hashgen) const;

private:
  const DCTypedef *_typedef;
};

#endif
