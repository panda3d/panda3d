// Filename: cppUsing.h
// Created by:  drose (16Nov99)
//
////////////////////////////////////////////////////////////////////

#ifndef CPPUSING_H
#define CPPUSING_H

#include <dtoolbase.h>

#include "cppDeclaration.h"

class CPPIdentifier;
class CPPScope;

///////////////////////////////////////////////////////////////////
// 	 Class : CPPUsing
// Description :
////////////////////////////////////////////////////////////////////
class CPPUsing : public CPPDeclaration {
public:
  CPPUsing(CPPIdentifier *ident, bool full_namespace, const CPPFile &file);

  virtual void output(ostream &out, int indent_level, CPPScope *scope,
		      bool complete) const;
  virtual SubType get_subtype() const;

  virtual CPPUsing *as_using();

  CPPIdentifier *_ident;
  bool _full_namespace;
};

#endif
