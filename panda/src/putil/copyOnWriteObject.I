// Filename: copyOnWriteObject.I
// Created by:  drose (09Apr07)
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


template<class Base>
TypeHandle CopyOnWriteObj<Base>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWriteObject::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CopyOnWriteObject::
CopyOnWriteObject() 
#ifdef HAVE_THREADS
  : _lock_cvar(_lock_mutex) 
#endif
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
#ifdef HAVE_THREADS
  _lock_status = LS_unlocked;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWriteObject::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CopyOnWriteObject::
CopyOnWriteObject(const CopyOnWriteObject &copy) :
  CachedTypedWritableReferenceCount(copy)
#ifdef HAVE_THREADS
  , _lock_cvar(_lock_mutex)
#endif
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
#ifdef HAVE_THREADS
  _lock_status = LS_unlocked;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWriteObject::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CopyOnWriteObject::
operator = (const CopyOnWriteObject &copy) {
  CachedTypedWritableReferenceCount::operator = (copy);
}

#ifdef HAVE_THREADS
////////////////////////////////////////////////////////////////////
//     Function: CopyOnWriteObject::cache_ref
//       Access: Published
//  Description: See CachedTypedWritableReferenceCount::cache_ref().
////////////////////////////////////////////////////////////////////
INLINE void CopyOnWriteObject::
cache_ref() const {
  MutexHolder holder(_lock_mutex);
  CachedTypedWritableReferenceCount::cache_ref();
}
#endif  // HAVE_THREADS

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWriteObj::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Base>
INLINE CopyOnWriteObj<Base>::
CopyOnWriteObj() {
}

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWriteObj::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Base>
INLINE CopyOnWriteObj<Base>::
CopyOnWriteObj(const Base &copy) : 
  Base(copy) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWriteObj::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Base>
INLINE CopyOnWriteObj<Base>::
CopyOnWriteObj(const CopyOnWriteObj<Base> &copy) : 
  CopyOnWriteObject(copy),
  Base(copy) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWriteObj::make_cow_copy
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class Base>
PT(CopyOnWriteObject) CopyOnWriteObj<Base>::
make_cow_copy() {
  return new CopyOnWriteObj<Base>(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWriteObj::init_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Base>
void CopyOnWriteObj<Base>::
init_type() {
#if defined(HAVE_RTTI) && !defined(__EDG__)
  // If we have RTTI, we can determine the name of the base type.
  string base_name = typeid(Base).name();
#else
  string base_name = "unknown";
#endif

  TypeHandle base_type = register_dynamic_type(base_name);

  CachedTypedWritableReferenceCount::init_type();
  _type_handle =
    register_dynamic_type("CopyOnWriteObj<" + base_name + ">",
                          base_type, 
                          CachedTypedWritableReferenceCount::get_class_type());
}
