// Filename: expand.h
// Created by:  cary (26Aug98)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef __EXPAND_H__
#define __EXPAND_H__

#include "dtoolbase.h"

#include "pfstream.h"
#include "config_setup.h"

#include <pwd.h>
#include <memory>
#include <sys/types.h>

namespace Expand {

static const ConfigString VChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.";

class Base_Expander {
   private:
      ConfigString _result;

      ConfigString Strip(ConfigString S);
      INLINE bool isEnv(ConfigString S);
      INLINE ConfigString Env(ConfigString S);
      istream& CopyStreamToString(istream& is, ConfigString& S);
      INLINE bool isUser(ConfigString S);
      INLINE ConfigString GetMyDir();
      INLINE ConfigString GetUserDir(ConfigString S);
      ConfigString Expand(ConfigString S);
      Base_Expander() {}
   public:
      Base_Expander(ConfigString S) : _result(Base_Expander::Expand(S)) {}
      Base_Expander(const Base_Expander& c) : _result(c._result) {}
      ~Base_Expander() {}
      INLINE ConfigString operator()();
      INLINE ConfigString operator()(ConfigString);
      INLINE operator ConfigString();
};

typedef Base_Expander Expander;

INLINE ConfigString Expand(ConfigString S);

#include "expand.I"

}  // Close namespace Expand

#endif /* __EXPAND_H__ */
