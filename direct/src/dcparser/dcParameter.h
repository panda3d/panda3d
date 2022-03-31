/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcParameter.h
 * @author drose
 * @date 2004-06-15
 */

#ifndef DCPARAMETER_H
#define DCPARAMETER_H

#include "dcbase.h"
#include "dcField.h"
#include "dcNumericRange.h"

class DCSimpleParameter;
class DCClassParameter;
class DCArrayParameter;
class DCTypedef;
class HashGenerator;

/**
 * Represents the type specification for a single parameter within a field
 * specification.  This may be a simple type, or it may be a class or an array
 * reference.
 *
 * This may also be a typedef reference to another type, which has the same
 * properties as the referenced type, but a different name.
 */
class EXPCL_DIRECT_DCPARSER DCParameter : public DCField {
protected:
  DCParameter();
  DCParameter(const DCParameter &copy);
public:
  virtual ~DCParameter();

PUBLISHED:
  virtual DCParameter *as_parameter();
  virtual const DCParameter *as_parameter() const;
  virtual DCSimpleParameter *as_simple_parameter();
  virtual const DCSimpleParameter *as_simple_parameter() const;
  virtual DCClassParameter *as_class_parameter();
  virtual const DCClassParameter *as_class_parameter() const;
  virtual DCSwitchParameter *as_switch_parameter();
  virtual const DCSwitchParameter *as_switch_parameter() const;
  virtual DCArrayParameter *as_array_parameter();
  virtual const DCArrayParameter *as_array_parameter() const;

  virtual DCParameter *make_copy() const=0;
  virtual bool is_valid() const=0;

  const DCTypedef *get_typedef() const;

public:
  void set_typedef(const DCTypedef *dtypedef);
  virtual DCParameter *append_array_specification(const DCUnsignedIntRange &size);

  virtual void output(std::ostream &out, bool brief) const;
  virtual void write(std::ostream &out, bool brief, int indent_level) const;
  virtual void output_instance(std::ostream &out, bool brief, const std::string &prename,
                               const std::string &name, const std::string &postname) const=0;
  virtual void write_instance(std::ostream &out, bool brief, int indent_level,
                              const std::string &prename, const std::string &name,
                              const std::string &postname) const;
  void output_typedef_name(std::ostream &out, bool brief, const std::string &prename,
                           const std::string &name, const std::string &postname) const;
  void write_typedef_name(std::ostream &out, bool brief, int indent_level,
                          const std::string &prename, const std::string &name,
                          const std::string &postname) const;
  virtual void generate_hash(HashGenerator &hashgen) const;

private:
  const DCTypedef *_typedef;
};

#endif
