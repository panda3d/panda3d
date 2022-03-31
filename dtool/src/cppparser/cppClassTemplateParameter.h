/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppClassTemplateParameter.h
 * @author drose
 * @date 1999-10-28
 */

#ifndef CPPCLASSTEMPLATEPARAMETER_H
#define CPPCLASSTEMPLATEPARAMETER_H

#include "dtoolbase.h"

#include "cppType.h"

class CPPIdentifier;

/**
 *
 */
class CPPClassTemplateParameter : public CPPType {
public:
  CPPClassTemplateParameter(CPPIdentifier *ident,
                            CPPType *default_type = nullptr);

  virtual bool is_fully_specified() const;
  virtual void output(std::ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;
  virtual SubType get_subtype() const;

  virtual CPPClassTemplateParameter *as_class_template_parameter();

  CPPIdentifier *_ident;
  CPPType *_default_type;
  bool _packed;

protected:
  virtual bool is_equal(const CPPDeclaration *other) const;
  virtual bool is_less(const CPPDeclaration *other) const;
};

#endif
