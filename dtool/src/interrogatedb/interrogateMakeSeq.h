// Filename: interrogateMakeSeq.h
// Created by:  drose (15Sep09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef INTERROGATEMAKESEQ_H
#define INTERROGATEMAKESEQ_H

#include "dtoolbase.h"

#include "interrogateComponent.h"

class IndexRemapper;

////////////////////////////////////////////////////////////////////
//       Class : InterrogateMakeSeq
// Description : Represents a synthetic method created via the
//               MAKE_SEQ() macro.
////////////////////////////////////////////////////////////////////
class EXPCL_INTERROGATEDB InterrogateMakeSeq : public InterrogateComponent {
public:
  INLINE InterrogateMakeSeq(InterrogateModuleDef *def = NULL);
  INLINE InterrogateMakeSeq(const InterrogateMakeSeq &copy);
  INLINE void operator = (const InterrogateMakeSeq &copy);

  INLINE TypeIndex get_class() const;
  INLINE const string &get_seq_name() const;
  INLINE const string &get_num_name() const;
  INLINE const string &get_element_name() const;

  void output(ostream &out) const;
  void input(istream &in);

  void remap_indices(const IndexRemapper &remap);

private:
  TypeIndex _class;
  string _seq_name;
  string _num_name;
  string _element_name;

  friend class InterrogateBuilder;
};

INLINE ostream &operator << (ostream &out, const InterrogateMakeSeq &make_seq);
INLINE istream &operator >> (istream &in, InterrogateMakeSeq &make_seq);

#include "interrogateMakeSeq.I"

#endif
