/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file extension.h
 * @author rdb
 * @date 2013-09-11
 */

#ifndef EXTENSION_H
#define EXTENSION_H

#include "dtoolbase.h"

/**
 * This is where all extensions should derive from.  It defines the _self and
 * _this members that can be used from the extension method.
 */
template<class T>
class EXPCL_INTERROGATEDB ExtensionBase {
public:
  T * _this;
};

/**
 * The default class template does not define any methods.  Classes that are
 * extended should create a specialization of this class template.
 */
template<class T>
class Extension : public ExtensionBase<T> {
};

/**
 * Creates a new extension object for the given pointer that can then be used
 * to call extension methods, as follows: invoke_extension((MyClass)
 * *ptr).method()
 */
template<class T>
inline Extension<T>
invoke_extension(T *ptr) {
  Extension<T> ext;
  ext._this = ptr;
  return ext;
}

/**
 * The const version of the above function.
 */
template<class T>
inline const Extension<T>
invoke_extension(const T *ptr) {
  Extension<T> ext;
  ext._this = (T *) ptr;
  return ext;
}

#endif
