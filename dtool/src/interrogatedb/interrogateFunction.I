// Filename: interrogateFunction.I
// Created by:  drose (01Aug00)
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


////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunction::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE InterrogateFunction::
InterrogateFunction(InterrogateModuleDef *def) :
  InterrogateComponent(def)
{
  _flags = 0;
  _class = 0;
  _instances = (Instances *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunction::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE InterrogateFunction::
InterrogateFunction(const InterrogateFunction &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunction::is_global
//       Access: Public
//  Description: Returns true if the function is marked as 'global'.
//               This means only that it should appear in the global
//               function list.
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateFunction::
is_global() const {
  return (_flags & F_global) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunction::is_virtual
//       Access: Public
//  Description: Returns true if the function is virtual, for whatever
//               that's worth.
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateFunction::
is_virtual() const {
  return (_flags & F_virtual) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunction::is_method
//       Access: Public
//  Description: Returns true if the function is a class method.
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateFunction::
is_method() const {
  return (_flags & F_method) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunction::get_class
//       Access: Public
//  Description: Return the class that owns the method, if is_method()
//               returns true.
////////////////////////////////////////////////////////////////////
INLINE TypeIndex InterrogateFunction::
get_class() const {
  return _class;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunction::has_scoped_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateFunction::
has_scoped_name() const {
  return !_scoped_name.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunction::get_scoped_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &InterrogateFunction::
get_scoped_name() const {
  return _scoped_name;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunction::has_comment
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateFunction::
has_comment() const {
  return !_comment.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunction::get_comment
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &InterrogateFunction::
get_comment() const {
  return _comment;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunction::has_prototype
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InterrogateFunction::
has_prototype() const {
  return !_prototype.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunction::get_prototype
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &InterrogateFunction::
get_prototype() const {
  return _prototype;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunction::number_of_c_wrappers
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int InterrogateFunction::
number_of_c_wrappers() const {
  return _c_wrappers.size();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunction::get_c_wrapper
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FunctionWrapperIndex InterrogateFunction::
get_c_wrapper(int n) const {
  if (n >= 0 && n < (int)_c_wrappers.size()) {
    return _c_wrappers[n];
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunction::number_of_python_wrappers
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int InterrogateFunction::
number_of_python_wrappers() const {
  return _python_wrappers.size();
}

////////////////////////////////////////////////////////////////////
//     Function: InterrogateFunction::get_python_wrapper
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FunctionWrapperIndex InterrogateFunction::
get_python_wrapper(int n) const {
  if (n >= 0 && n < (int)_python_wrappers.size()) {
    return _python_wrappers[n];
  }
  return 0;
}


INLINE ostream &
operator << (ostream &out, const InterrogateFunction &function) {
  function.output(out);
  return out;
}

INLINE istream &
operator >> (istream &in, InterrogateFunction &function) {
  function.input(in);
  return in;
}
