/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file functionWriter.h
 * @author drose
 * @date 2001-09-14
 */

#ifndef FUNCTIONWRITER_H
#define FUNCTIONWRITER_H

#include "dtoolbase.h"

/**
 * This is an abstract class that can be used by the various InterfaceMakers
 * to indicate a generic helper function or variable that needs to be written
 * to the generated source file.
 */
class FunctionWriter {
public:
  FunctionWriter();
  virtual ~FunctionWriter();

  const std::string &get_name() const;
  virtual int compare_to(const FunctionWriter &other) const;

  virtual void write_prototype(std::ostream &out);
  virtual void write_code(std::ostream &out);

protected:
  std::string _name;
};

#endif
