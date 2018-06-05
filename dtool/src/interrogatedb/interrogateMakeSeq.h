/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogateMakeSeq.h
 * @author drose
 * @date 2009-09-15
 */

#ifndef INTERROGATEMAKESEQ_H
#define INTERROGATEMAKESEQ_H

#include "dtoolbase.h"

#include "interrogateComponent.h"

class IndexRemapper;

/**
 * Represents a synthetic method created via the MAKE_SEQ() macro.
 */
class EXPCL_INTERROGATEDB InterrogateMakeSeq : public InterrogateComponent {
public:
  INLINE InterrogateMakeSeq(InterrogateModuleDef *def = nullptr);
  INLINE InterrogateMakeSeq(const InterrogateMakeSeq &copy);
  INLINE void operator = (const InterrogateMakeSeq &copy);

  INLINE bool has_scoped_name() const;
  INLINE const string &get_scoped_name() const;

  INLINE bool has_comment() const;
  INLINE const string &get_comment() const;

  INLINE FunctionIndex get_length_getter() const;
  INLINE FunctionIndex get_element_getter() const;

  void output(ostream &out) const;
  void input(istream &in);

  void remap_indices(const IndexRemapper &remap);

private:
  string _scoped_name;
  string _comment;
  FunctionIndex _length_getter;
  FunctionIndex _element_getter;

  friend class InterrogateBuilder;
};

INLINE ostream &operator << (ostream &out, const InterrogateMakeSeq &make_seq);
INLINE istream &operator >> (istream &in, InterrogateMakeSeq &make_seq);

#include "interrogateMakeSeq.I"

#endif
