// Filename: expand.I
// Created by:  cary (20Mar00)
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

INLINE bool Base_Expander::isEnv(ConfigString S) {
  return (getenv(S.c_str()) != (TYPENAME ConfigString::value_type *)0L);
}

INLINE ConfigString Base_Expander::Env(ConfigString S) {
  return isEnv(S)?getenv(S.c_str()):"";
}

INLINE bool Base_Expander::isUser(ConfigString S) {
   if (S.find_first_of(VChars) == 0) {
      std::auto_ptr<struct passwd> pw(getpwnam(S.c_str()));
      return (pw.get() != (struct passwd *)0L);
   } else
      return false;
}

INLINE ConfigString Base_Expander::GetMyDir() {
   std::auto_ptr<struct passwd> pw(getpwuid(geteuid()));
   return pw->pw_dir;
}

INLINE ConfigString Base_Expander::GetUserDir(ConfigString S) {
   std::auto_ptr<struct passwd> pw(getpwnam(S.c_str()));
   return ((pw.get() != (struct passwd *)0L)?pw->pw_dir:"");
}

INLINE ConfigString Base_Expander::operator()(void) {
  return _result;
}

INLINE ConfigString Base_Expander::operator()(ConfigString S) {
  _result = Base_Expander::Expand(S);
  return _result;
}

INLINE Base_Expander::operator ConfigString() {
  return _result;
}

INLINE ConfigString Expand(ConfigString S) {
   Base_Expander ex(S);
   return ex;
}
