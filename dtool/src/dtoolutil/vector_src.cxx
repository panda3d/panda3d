/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vector_src.cxx
 * @author drose
 * @date 2001-05-15
 */

/*
 * This file defines the interface to declare and export from the DLL an STL
 * vector of some type.  To use this file you must #define a number of symbols
 * and then #include it from a .cxx file.  You also must do the same thing
 * with vector_something_src.h from a .h file.  This is necessary because of
 * the complexity involved in exporting a vector class from a DLL.  If we are
 * using the Dinkumware STL implementation, it is even more complex.  However,
 * all this complexity is only needed to support Windows builds; Unix shared
 * libraries are able to export symbols (including templates) without any
 * special syntax.
 */


/*
 * The following variables should be defined prior to including this file:
 * EXPCL - the appropriate EXPCL_* symbol for this DLL. EXPTP - the
 * appropriate EXPTP_* symbol for this DLL. TYPE - the type of thing we are
 * building a vector on.  NAME - The name of the resulting vector typedef,
 * e.g.  vector_int.  They will automatically be undefined at the end of the
 * file.
 */

/*
void
insert_into_vector(NAME &vec, NAME::iterator where,
                   NAME::const_pointer begin, NAME::const_pointer end) {
  vec.insert(where, begin, end);
}
*/

#undef EXPCL
#undef EXPTP
#undef TYPE
#undef NAME
