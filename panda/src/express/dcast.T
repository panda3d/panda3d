// Filename: dcast.T
// Created by:  drose (06Aug01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: _dcast_get_typehandle
//  Description: Returns the TypeHandle associated with the type of
//               the parameter, if it can be determined.  This is a
//               support function for _dcast, below.
////////////////////////////////////////////////////////////////////
template<class WantType>
INLINE TypeHandle
_dcast_get_typehandle(WantType *) {
  TypeHandle handle = WantType::get_class_type();
  if (handle == TypeHandle::none()) {
    // This type handle is unregistered.  Oops!
    WantType::init_type();
    handle = WantType::get_class_type();
    express_cat->warning()
      << "Type " << handle << " was unregistered!\n";
  }
  return handle;
}


////////////////////////////////////////////////////////////////////
//     Function: _dcast
//  Description: The implementation of the DCAST macro, this checks
//               the actual type of the pointer before performing a
//               downcast operation.  In NDEBUG mode, it simply
//               downcasts.
//
//               This flavor of _dcast works on non-const pointers.
////////////////////////////////////////////////////////////////////
template<class WantType>
INLINE WantType *
_dcast(WantType *, TypedObject *ptr) {
#ifndef NDEBUG
  TypeHandle want_handle = _dcast_get_typehandle((WantType *)0);
 #if defined(_DEBUG) && defined(_WIN32)
  if ((ptr == (TypedObject *)NULL) || IsBadWritePtr(ptr,sizeof(TypedObject))) {
 #else
  if (ptr == (TypedObject *)NULL) {
 #endif
    express_cat->warning()
      << "Attempt to cast NULL pointer to " << want_handle << "\n";
    return (WantType *)NULL;
  }
  if (!ptr->is_of_type(want_handle)) {
    express_cat->error()
      << "Attempt to cast pointer from " << ptr->get_type()
      << " to " << want_handle << "\n";
    if (ptr->get_type() == TypedObject::get_class_type()) {
      express_cat->error(false)
        << "Perhaps pointer was inadvertently deleted?\n";
    }
    return (WantType *)NULL;
  }
#endif
  return (WantType *)ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: _dcast
//  Description: The implementation of the DCAST macro, this checks
//               the actual type of the pointer before performing a
//               downcast operation.  In NDEBUG mode, it simply
//               downcasts.
//
//               This flavor of _dcast works on const pointers.
////////////////////////////////////////////////////////////////////
template<class WantType>
INLINE const WantType *
_dcast(WantType *, const TypedObject *ptr) {
#ifndef NDEBUG
  TypeHandle want_handle = _dcast_get_typehandle((WantType *)0);
  if (ptr == (const TypedObject *)NULL) {
    express_cat->warning()
      << "Attempt to cast NULL pointer to " << want_handle << "\n";
    return (const WantType *)NULL;
  }
  if (!ptr->is_of_type(want_handle)) {
    express_cat->error()
      << "Attempt to cast pointer from " << ptr->get_type()
      << " to " << want_handle << "\n";
    if (ptr->get_type() == TypedObject::get_class_type()) {
      express_cat->error(false)
        << "Perhaps pointer was inadvertently deleted?\n";
    }
    return (const WantType *)NULL;
  }
#endif
  return (const WantType *)ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: _dcast_ref
//  Description: Similar to the above, with a pointer reference as the
//               first parameter.  Just for fiddly compiler reasons;
//               the reference isn't used.
////////////////////////////////////////////////////////////////////
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
