// Filename: wrapperBuilderPython.h
// Created by:  drose (07Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef WRAPPERBUILDERPYTHON_H
#define WRAPPERBUILDERPYTHON_H

#include <dtoolbase.h>

#include "wrapperBuilder.h"

////////////////////////////////////////////////////////////////////
// 	 Class : WrapperBuilderPython
// Description : A specialization on WrapperBuilder that builds
//               Python-style wrapper functions.
////////////////////////////////////////////////////////////////////
class WrapperBuilderPython : public WrapperBuilder {
public:
  WrapperBuilderPython();

  virtual void
  write_wrapper(ostream &out, const string &wrapper_name) const;

  virtual string
  get_wrapper_name(const string &library_hash_name) const;

  virtual bool supports_atomic_strings() const;
  virtual CallingConvention get_calling_convention() const;

protected:
  void test_assert(ostream &out, int indent_level) const;
  void pack_return_value(ostream &out, string return_expr) const;
};

#endif
