/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcast.T
 * @author drose
 * @date 2001-08-06
 */

/**
 * Returns the TypeHandle associated with the type of the parameter, if it can
 * be determined.  This is a support function for _dcast, below.
 */
template<class WantType>
INLINE TypeHandle
_dcast_get_typehandle(WantType *) {
  TAU_PROFILE("_dcast_get_typehandle()", " ", TAU_USER);
  TypeHandle handle = WantType::get_class_type();

#ifdef _DEBUG
  if (handle == TypeHandle::none()) {
    // This type handle is unregistered.  Oops!
    WantType::init_type();
    handle = WantType::get_class_type();
    express_cat->warning()
      << "Type " << handle << " was unregistered!\n";
  }
#endif

  return handle;
}

/**
 * The implementation of the DCAST macro, this checks the actual type of the
 * pointer before performing a downcast operation.  In NDEBUG mode, it simply
 * downcasts.
 *
 * This flavor of _dcast works on non-const pointers.
 */
template<class WantType>
INLINE WantType *
_dcast(WantType *, TypedObject *ptr) {
#ifdef DO_DCAST
  TAU_PROFILE("_dcast()", " ", TAU_USER);
  TypeHandle want_handle = _dcast_get_typehandle((WantType *)0);
  if (!_dcast_verify(want_handle, sizeof(WantType), ptr)) {
    return (WantType *)NULL;
  }
#endif
  return (WantType *)ptr;
}

/**
 * The implementation of the DCAST macro, this checks the actual type of the
 * pointer before performing a downcast operation.  In NDEBUG mode, it simply
 * downcasts.
 *
 * This flavor of _dcast works on const pointers.
 */
template<class WantType>
INLINE const WantType *
_dcast(WantType *, const TypedObject *ptr) {
#ifdef DO_DCAST
  TAU_PROFILE("_dcast()", " ", TAU_USER);
  TypeHandle want_handle = _dcast_get_typehandle((WantType *)0);
  if (!_dcast_verify(want_handle, sizeof(WantType), ptr)) {
    return (const WantType *)NULL;
  }
#endif
  return (const WantType *)ptr;
}

/**
 * Similar to the above, with a pointer reference as the first parameter.
 * Just for fiddly compiler reasons; the reference isn't used.
*/
template<class WantType>
INLINE WantType *
_dcast_ref(WantType *&, TypedObject *ptr) {
  return _dcast((WantType *)NULL, ptr);
}

template<class WantType>
INLINE const WantType *
_dcast_ref(WantType *&, const TypedObject *ptr) {
  return _dcast((WantType *)NULL, ptr);
}
