/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parameterRemap.I
 * @author drose
 * @date 2000-08-01
 */

/**
 *
 */
INLINE ParameterRemap::
ParameterRemap(CPPType *orig_type) :
  _orig_type(orig_type),
  _new_type(orig_type)
{
  _is_valid = true;
  _temporary_type = nullptr;
  _default_value = nullptr;
}

/**
 *
 */
INLINE bool ParameterRemap::
is_valid() const {
  return _is_valid;
}

/**
 * Returns the type of the original, C++ parameter or return value.
 */
INLINE CPPType *ParameterRemap::
get_orig_type() const {
  return _orig_type;
}

/**
 * Returns the type of the wrapper's parameter or return value.  This is the
 * type that will be reported in the interrogate database, and the type that
 * the scripting language is expected to deal with.
 */
INLINE CPPType *ParameterRemap::
get_new_type() const {
  return _new_type;
}

/**
 * Returns the type of any temporary variables used to hold the return value
 * before returning it.  This is normally the same as get_new_type(), but in
 * some circumstances it may need to be different.
 */
INLINE CPPType *ParameterRemap::
get_temporary_type() const {
  if (_temporary_type == nullptr) {
    return _new_type;
  } else {
    return _temporary_type;
  }
}

/**
 * Returns true if this particular parameter has a default value defined.
 */
INLINE bool ParameterRemap::
has_default_value() const {
  return (_default_value != nullptr);
}

/**
 * Returns the expression corresponding to this parameter's default value.
 */
INLINE CPPExpression *ParameterRemap::
get_default_value() const {
  return _default_value;
}

/**
 * Records a default value to be associated with this parameter.
 */
INLINE void ParameterRemap::
set_default_value(CPPExpression *expr) {
  _default_value = expr;
}
