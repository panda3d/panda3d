// Filename: wrapperBuilderC.h
// Created by:  drose (06Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef WRAPPERBUILDERC_H
#define WRAPPERBUILDERC_H

#include <dtoolbase.h>

#include "wrapperBuilder.h"

////////////////////////////////////////////////////////////////////
//       Class : WrapperBuilderC
// Description : A specialization on WrapperBuilder that builds
//               C-style wrapper functions.
////////////////////////////////////////////////////////////////////
class WrapperBuilderC : public WrapperBuilder {
public:
  WrapperBuilderC();

  virtual void
  write_wrapper(ostream &out, const string &wrapper_name) const;

  virtual string
  get_wrapper_name(const string &library_hash_name) const;

  virtual bool supports_atomic_strings() const;
  virtual CallingConvention get_calling_convention() const;
};

#endif
