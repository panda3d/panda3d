// Filename: interrogateComponent.h
// Created by:  drose (08Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef INTERROGATECOMPONENT_H
#define INTERROGATECOMPONENT_H

#include "dtoolbase.h"

#include "interrogate_interface.h"
#include "interrogate_request.h"

#include <vector>

class IndexRemapper;

////////////////////////////////////////////////////////////////////
//       Class : InterrogateComponent
// Description : The base class for things that are part of the
//               interrogate database.  This includes types,
//               functions, and function wrappers.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG InterrogateComponent {
public:
  INLINE InterrogateComponent(InterrogateModuleDef *def = NULL);
  INLINE InterrogateComponent(const InterrogateComponent &copy);
  INLINE void operator = (const InterrogateComponent &copy);

  INLINE bool has_library_name() const;
  INLINE const char *get_library_name() const;

  INLINE bool has_module_name() const;
  INLINE const char *get_module_name() const;

  INLINE bool has_name() const;
  INLINE const string &get_name() const;

  void output(ostream &out) const;
  void input(istream &in);

private:
  InterrogateModuleDef *_def;
  string _name;

  friend class InterrogateBuilder;
  friend class FunctionRemap;
};

#include "interrogateComponent.I"

#endif


