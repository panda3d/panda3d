// Filename: cppEnumType.h
// Created by:  drose (25Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef CPPENUMTYPE_H
#define CPPENUMTYPE_H

#include <dtoolbase.h>

#include "cppExtensionType.h"

#include <vector>

class CPPExpression;
class CPPInstance;
class CPPScope;


///////////////////////////////////////////////////////////////////
// 	 Class : CPPEnumType
// Description :
////////////////////////////////////////////////////////////////////
class CPPEnumType : public CPPExtensionType {
public:
  CPPEnumType(CPPIdentifier *ident, CPPScope *current_scope,
	      const CPPFile &file);

  void add_element(const string &name, CPPScope *scope,
		   CPPExpression *value = (CPPExpression *)NULL);

  virtual bool is_incomplete() const;

  virtual CPPDeclaration *substitute_decl(SubstDecl &subst,
					  CPPScope *current_scope,
					  CPPScope *global_scope);

  virtual void output(ostream &out, int indent_level, CPPScope *scope,
		      bool complete) const;
  virtual SubType get_subtype() const;

  virtual CPPEnumType *as_enum_type();
 
  typedef vector<CPPInstance *> Elements;
  Elements _elements;
};


#endif
