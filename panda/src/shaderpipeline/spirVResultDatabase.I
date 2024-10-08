/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spirVResultDatabase.I
 * @author rdb
 * @date 2024-10-08
 */

/**
 * Returns true if this is a type (including a pointer type).
 */
INLINE bool SpirVResultDatabase::Definition::
is_type() const {
  return _dtype == DT_type || _dtype == DT_pointer_type;
}

/**
 * Returns true if this is specifically a pointer type.
 */
INLINE bool SpirVResultDatabase::Definition::
is_pointer_type() const {
  return _dtype == DT_pointer_type;
}

/**
 * Returns true if this is a variable.
 */
INLINE bool SpirVResultDatabase::Definition::
is_variable() const {
  return _dtype == DT_variable;
}

/**
 * Returns true if this is a constant.
 */
INLINE bool SpirVResultDatabase::Definition::
is_constant() const {
  return _dtype == DT_constant;
}

/**
 * Returns true if this is a specialization constant.
 */
INLINE bool SpirVResultDatabase::Definition::
is_spec_constant() const {
  return _dtype == DT_spec_constant;
}

/**
 * Returns true if this is a function.
 */
INLINE bool SpirVResultDatabase::Definition::
is_function() const {
  return _dtype == DT_function;
}

/**
 * Returns true if this is the result of an OpExtInstImport instruction.
 */
INLINE bool SpirVResultDatabase::Definition::
is_ext_inst() const {
  return _dtype == DT_ext_inst;
}

/**
 * For a variable or function parameter, returns true if its value has been
 * loaded or passed into a function call.  For a type or type pointer, returns
 * true if it is the type of at least one variable that is marked "used".  For
 * a function, returns true if it is called at least once.
 */
INLINE bool SpirVResultDatabase::Definition::
is_used() const {
  return (_flags & DF_used) != 0;
}

/**
 * For a variable, returns true if it has been sampled using a dref comparison.
 */
INLINE bool SpirVResultDatabase::Definition::
is_dref_sampled() const {
  return (_flags & DF_dref_sampled) != 0;
}

/**
 * For a variable, returns true if it has been dynamically indexed.
 */
INLINE bool SpirVResultDatabase::Definition::
is_dynamically_indexed() const {
  return (_flags & DF_dynamically_indexed) != 0;
}

/**
 * Returns true if this has the BuiltIn decoration.  See also has_builtin().
 */
INLINE bool SpirVResultDatabase::Definition::
is_builtin() const {
  return _builtin != spv::BuiltInMax;
}

/**
 * Returns true if this has a Location decoration.
 */
INLINE bool SpirVResultDatabase::Definition::
has_location() const {
  return _location >= 0;
}
