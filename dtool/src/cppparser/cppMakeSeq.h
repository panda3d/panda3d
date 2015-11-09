// Filename: cppMakeSeq.h
// Created by:  drose (06Nov08)
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

#ifndef CPPMAKESEQ_H
#define CPPMAKESEQ_H

#include "dtoolbase.h"

#include "cppDeclaration.h"
#include "cppIdentifier.h"

///////////////////////////////////////////////////////////////////
//       Class : CPPMakeSeq
// Description : This is a MAKE_SEQ() declaration appearing within a
//               class body.  It means to generate a sequence method
//               within Python, replacing (for instance)
//               get_num_nodes()/get_node(n) with a synthetic
//               get_nodes() method.
////////////////////////////////////////////////////////////////////
class CPPMakeSeq : public CPPDeclaration {
public:
  CPPMakeSeq(CPPIdentifier *ident,
             CPPFunctionGroup *length_getter,
             CPPFunctionGroup *element_getter,
             CPPScope *current_scope, const CPPFile &file);

  virtual string get_simple_name() const;
  virtual string get_local_name(CPPScope *scope = NULL) const;
  virtual string get_fully_scoped_name() const;

  virtual void output(ostream &out, int indent_level, CPPScope *scope,
                      bool complete) const;

  virtual SubType get_subtype() const;
  virtual CPPMakeSeq *as_make_seq();

  CPPIdentifier *_ident;
  CPPFunctionGroup *_length_getter;
  CPPFunctionGroup *_element_getter;
};

#endif
