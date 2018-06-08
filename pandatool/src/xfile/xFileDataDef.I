/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileDataDef.I
 * @author drose
 * @date 2004-10-03
 */

/**
 *
 */
INLINE XFileDataDef::
XFileDataDef(XFile *x_file, const std::string &name,
             XFileDataDef::Type type, XFileTemplate *xtemplate) :
  XFileNode(x_file, name),
  _type(type),
  _template(xtemplate)
{
}

/**
 * Returns the primitive type of this element, or T_template if this
 * represents a nested template object.
 */
INLINE XFileDataDef::Type XFileDataDef::
get_data_type() const {
  return _type;
}

/**
 * If get_data_type() returned T_template, this returns the particular
 * template pointer that this object represents.
 */
INLINE XFileTemplate *XFileDataDef::
get_template() const {
  return _template;
}

/**
 * Returns the number of dimensions of array elements on this data object, or
 * 0 if the data object is not an array.
 */
INLINE int XFileDataDef::
get_num_array_defs() const {
  return _array_def.size();
}

/**
 * Returns the description of the nth dimension of array elements on this data
 * object.
 */
INLINE const XFileArrayDef &XFileDataDef::
get_array_def(int i) const {
  nassertr(i >= 0 && i < (int)_array_def.size(), _array_def[0]);
  return _array_def[i];
}
