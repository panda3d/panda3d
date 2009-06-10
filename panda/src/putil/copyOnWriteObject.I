// Filename: copyOnWriteObject.I
// Created by:  drose (09Apr07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


template<class Base>
TypeHandle CopyOnWriteObj<Base>::_type_handle;

template<class Base, class Param1>
TypeHandle CopyOnWriteObj1<Base, Param1>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWriteObject::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CopyOnWriteObject::
CopyOnWriteObject() 
#ifdef COW_THREADED
  : _lock_mutex("CopyOnWriteObject::_lock"),
    _lock_cvar(_lock_mutex) 
#endif
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
#ifdef COW_THREADED
  _lock_status = LS_unlocked;
  _locking_thread = NULL;
#endif  // COW_THREADED
}

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWriteObject::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CopyOnWriteObject::
CopyOnWriteObject(const CopyOnWriteObject &copy) :
  CachedTypedWritableReferenceCount(copy)
#ifdef COW_THREADED
  , _lock_mutex("CopyOnWriteObject::_lock"),
  _lock_cvar(_lock_mutex) 
#endif
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, this);
#endif
#ifdef COW_THREADED
  _lock_status = LS_unlocked;
  _locking_thread = NULL;
#endif  // COW_THREADED
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

#ifdef COW_THREADED
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
#endif  // COW_THREADED

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

  CopyOnWriteObject::init_type();
  _type_handle =
    register_dynamic_type("CopyOnWriteObj<" + base_name + ">",
                          base_type, 
                          CopyOnWriteObject::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWriteObj1::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Base, class Param1>
INLINE CopyOnWriteObj1<Base, Param1>::
CopyOnWriteObj1(Param1 p1) : Base(p1) {
}

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWriteObj1::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Base, class Param1>
INLINE CopyOnWriteObj1<Base, Param1>::
CopyOnWriteObj1(const Base &copy) : 
  Base(copy) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWriteObj1::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Base, class Param1>
INLINE CopyOnWriteObj1<Base, Param1>::
CopyOnWriteObj1(const CopyOnWriteObj1<Base, Param1> &copy) : 
  CopyOnWriteObject(copy),
  Base(copy) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWriteObj1::make_cow_copy
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
template<class Base, class Param1>
PT(CopyOnWriteObject) CopyOnWriteObj1<Base, Param1>::
make_cow_copy() {
  return new CopyOnWriteObj1<Base, Param1>(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CopyOnWriteObj1::init_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Base, class Param1>
void CopyOnWriteObj1<Base, Param1>::
init_type() {
#if defined(HAVE_RTTI) && !defined(__EDG__)
  // If we have RTTI, we can determine the name of the base type.
  string base_name = typeid(Base).name();
#else
  string base_name = "unknown";
#endif

  TypeHandle base_type = register_dynamic_type(base_name);

  CopyOnWriteObject::init_type();
  _type_handle =
    register_dynamic_type("CopyOnWriteObj1<" + base_name + ">",
                          base_type, 
                          CopyOnWriteObject::get_class_type());
}
