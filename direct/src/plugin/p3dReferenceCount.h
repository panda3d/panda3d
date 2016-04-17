/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dReferenceCount.h
 * @author drose
 * @date 2009-07-09
 */

#ifndef P3DREFERENCECOUNT_H
#define P3DREFERENCECOUNT_H

#include "p3d_plugin_common.h"

/**
 * A base class for reference-counted objects in this module.  We follow the
 * Panda convention, rather than the Python convention: the reference count of
 * a new object is initially 0.
 */
class P3DReferenceCount {
public:
  inline P3DReferenceCount();
  inline ~P3DReferenceCount();

  inline void ref() const;
  inline bool unref() const;
  inline int get_ref_count() const;

private:
  int _ref_count;
};

template<class RefCountType>
inline void p3d_unref_delete(RefCountType *ptr);

#include "p3dReferenceCount.I"

#endif
