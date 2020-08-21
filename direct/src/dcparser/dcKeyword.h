/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcKeyword.h
 * @author drose
 * @date 2005-07-22
 */

#ifndef DCKEYWORD_H
#define DCKEYWORD_H

#include "dcbase.h"
#include "dcDeclaration.h"

class DCParameter;
class HashGenerator;

/**
 * This represents a single keyword declaration in the dc file.  It is used to
 * define a communication property associated with a field, for instance
 * "broadcast" or "airecv".
 */
class EXPCL_DIRECT_DCPARSER DCKeyword : public DCDeclaration {
public:
  DCKeyword(const std::string &name, int historical_flag = ~0);
  virtual ~DCKeyword();

PUBLISHED:
  const std::string &get_name() const;

public:
  int get_historical_flag() const;
  void clear_historical_flag();

  virtual void output(std::ostream &out, bool brief) const;
  virtual void write(std::ostream &out, bool brief, int indent_level) const;
  void generate_hash(HashGenerator &hashgen) const;

private:
  const std::string _name;

  // This flag is only kept for historical reasons, so we can preserve the
  // file's hash code if no new flags are in use.
  int _historical_flag;
};

#endif
