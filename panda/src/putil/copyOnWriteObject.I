/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file copyOnWriteObject.I
 * @author drose
 * @date 2007-04-09
 */

template<class Base>
TypeHandle CopyOnWriteObj<Base>::_type_handle;

template<class Base, class Param1>
TypeHandle CopyOnWriteObj1<Base, Param1>::_type_handle;

/**
 *
 */
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
  _locking_thread = nullptr;
#endif  // COW_THREADED
}

/**
 *
 */
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
  _locking_thread = nullptr;
#endif  // COW_THREADED
}

/**
 *
 */
INLINE void CopyOnWriteObject::
operator = (const CopyOnWriteObject &copy) {
  CachedTypedWritableReferenceCount::operator = (copy);
}

#ifdef COW_THREADED
/**
 * @see CachedTypedWritableReferenceCount::cache_ref()
 */
INLINE void CopyOnWriteObject::
cache_ref() const {
  MutexHolder holder(_lock_mutex);
  CachedTypedWritableReferenceCount::cache_ref();
}

/**
 * @see CachedTypedWritableReferenceCount::cache_unref()
 */
INLINE bool CopyOnWriteObject::
cache_unref() const {
  MutexHolder holder(_lock_mutex);
  return CachedTypedWritableReferenceCount::cache_unref();
}
#endif  // COW_THREADED

/**
 *
 */
template<class Base>
INLINE CopyOnWriteObj<Base>::
CopyOnWriteObj() {
}

/**
 *
 */
template<class Base>
INLINE CopyOnWriteObj<Base>::
CopyOnWriteObj(const Base &copy) :
  Base(copy)
{
}

/**
 *
 */
template<class Base>
INLINE CopyOnWriteObj<Base>::
CopyOnWriteObj(const CopyOnWriteObj<Base> &copy) :
  CopyOnWriteObject(copy),
  Base(copy)
{
}

/**
 *
 */
template<class Base>
PT(CopyOnWriteObject) CopyOnWriteObj<Base>::
make_cow_copy() {
  return new CopyOnWriteObj<Base>(*this);
}

/**
 *
 */
template<class Base>
void CopyOnWriteObj<Base>::
init_type() {
  std::string base_name = get_type_name(Base);
  TypeHandle base_type = register_dynamic_type(base_name);

  CopyOnWriteObject::init_type();
  _type_handle =
    register_dynamic_type("CopyOnWriteObj<" + base_name + ">",
                          base_type,
                          CopyOnWriteObject::get_class_type());
}

/**
 *
 */
template<class Base, class Param1>
INLINE CopyOnWriteObj1<Base, Param1>::
CopyOnWriteObj1(Param1 p1) : Base(p1) {
}

/**
 *
 */
template<class Base, class Param1>
INLINE CopyOnWriteObj1<Base, Param1>::
CopyOnWriteObj1(const Base &copy) :
  Base(copy)
{
}

/**
 *
 */
template<class Base, class Param1>
INLINE CopyOnWriteObj1<Base, Param1>::
CopyOnWriteObj1(const CopyOnWriteObj1<Base, Param1> &copy) :
  CopyOnWriteObject(copy),
  Base(copy)
{
}

/**
 *
 */
template<class Base, class Param1>
PT(CopyOnWriteObject) CopyOnWriteObj1<Base, Param1>::
make_cow_copy() {
  return new CopyOnWriteObj1<Base, Param1>(*this);
}

/**
 *
 */
template<class Base, class Param1>
void CopyOnWriteObj1<Base, Param1>::
init_type() {
  std::string base_name = get_type_name(Base);
  TypeHandle base_type = register_dynamic_type(base_name);

  CopyOnWriteObject::init_type();
  _type_handle =
    register_dynamic_type("CopyOnWriteObj1<" + base_name + ">",
                          base_type,
                          CopyOnWriteObject::get_class_type());
}
