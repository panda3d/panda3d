/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcClassParameter.h
 * @author drose
 * @date 2004-06-18
 */

#ifndef DCCLASSPARAMETER_H
#define DCCLASSPARAMETER_H

#include "dcbase.h"
#include "dcParameter.h"

class DCClass;

/**
 * This represents a class (or struct) object used as a parameter itself.
 * This means that all the fields of the class get packed into the message.
 */
class EXPCL_DIRECT_DCPARSER DCClassParameter : public DCParameter {
public:
  DCClassParameter(const DCClass *dclass);
  DCClassParameter(const DCClassParameter &copy);

PUBLISHED:
  virtual DCClassParameter *as_class_parameter();
  virtual const DCClassParameter *as_class_parameter() const;
  virtual DCParameter *make_copy() const;
  virtual bool is_valid() const;

  const DCClass *get_class() const;

public:
  virtual DCPackerInterface *get_nested_field(int n) const;

  virtual void output_instance(std::ostream &out, bool brief, const std::string &prename,
                               const std::string &name, const std::string &postname) const;
  virtual void generate_hash(HashGenerator &hashgen) const;

protected:
  virtual bool do_check_match(const DCPackerInterface *other) const;
  virtual bool do_check_match_class_parameter(const DCClassParameter *other) const;
  virtual bool do_check_match_array_parameter(const DCArrayParameter *other) const;

private:
  typedef pvector<DCPackerInterface *> Fields;
  Fields _nested_fields;

  const DCClass *_dclass;
};

#endif
