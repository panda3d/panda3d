// Filename: interrogateComponent.h
// Created by:  drose (08Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef INTERROGATECOMPONENT_H
#define INTERROGATECOMPONENT_H

#include <dtoolbase.h>

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
};

#include "interrogateComponent.I"

#endif


