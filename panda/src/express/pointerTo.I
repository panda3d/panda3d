// Filename: pointerTo.I
// Created by:  drose (10Feb99)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PointerToBase<T>::
PointerToBase(To *ptr) {
  _ptr = (To *)NULL;
  reassign(ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Copy Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PointerToBase<T>::
PointerToBase(const PointerToBase<T> &copy) {
  _ptr = (To *)NULL;
  reassign(copy);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Destructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PointerToBase<T>::
~PointerToBase() {
  reassign((To *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::reassign
//       Access: Protected
//  Description: This is the main work of the PointerTo family.  When
//               the pointer is reassigned, decrement the old
//               reference count and increment the new one.
////////////////////////////////////////////////////////////////////
template<class T>
void PointerToBase<T>::
reassign(To *ptr) {
  if (ptr != _ptr) {
    if (_ptr != (To *)NULL) {
      unref_delete(_ptr);
    }
    
    _ptr = ptr;
    if (_ptr != (To *)NULL) {
      _ptr->ref();
#ifndef NDEBUG
      if (MemoryUsage::get_track_memory_usage()) {
	// Make sure the MemoryUsage record knows what the TypeHandle
	// is, if we know it ourselves.
	TypeHandle type = get_type_handle(To);
	if (type == TypeHandle::none()) {
	  do_init_type(To);
	  type = get_type_handle(To);
	}
	if (type != TypeHandle::none()) {
	  MemoryUsage::update_type(_ptr, type);
	}
      }
#endif
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::reassign
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE void PointerToBase<T>::
reassign(const PointerToBase<To> &copy) {
  reassign(copy._ptr);
}

#ifndef CPPPARSER
#ifndef WIN32_VC
////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Equivalence operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator == (const To *other) const {
  return _ptr == other;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Nonequivalence operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator != (const To *other) const {
  return _ptr != other;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Greater-than operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator > (const To *other) const {
  return _ptr > other;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Less-than-or-equal operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator <= (const To *other) const {
  return _ptr <= other;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Greater-than-or-equal operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator >= (const To *other) const {
  return _ptr >= other;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Equivalence operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator == (const PointerToBase<To> &other) const {
  return _ptr == other._ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Nonequivalence operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator != (const PointerToBase<To> &other) const {
  return _ptr != other._ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Greater-than operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator > (const PointerToBase<To> &other) const {
  return _ptr > other._ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Less-than-or-equal operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator <= (const PointerToBase<To> &other) const {
  return _ptr <= other._ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Greater-than-or-equal operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator >= (const PointerToBase<To> &other) const {
  return _ptr >= other._ptr;
}
#endif  // WIN32_VC

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Less-than operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator < (const To *other) const {
  return _ptr < other;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::Less-than operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
operator < (const PointerToBase<To> &other) const {
  return _ptr < other._ptr;
}

#endif  // CPPPARSER



////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::is_null
//       Access: Public
//  Description: Returns true if the PointerTo is a NULL pointer,
//               false otherwise.  (Direct comparison to a NULL
//               pointer also works.)
////////////////////////////////////////////////////////////////////
template<class T>
INLINE bool PointerToBase<T>::
is_null() const {
  return (_ptr == NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::clear
//       Access: Public
//  Description: A convenient way to set the PointerTo object to NULL.
//               (Assignment to a NULL pointer also works, of course.)
////////////////////////////////////////////////////////////////////
template<class T>
INLINE void PointerToBase<T>::
clear() {
  reassign((To *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PointerToBase::output
//       Access: Public
//  Description: A handy function to output PointerTo's as a hex
//               pointer followed by a reference count.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE void PointerToBase<T>::
output(ostream &out) const {
  out << (void *)_ptr;
  if (_ptr != (To *)NULL) {
    out << ":" << _ptr->get_count();
  }
}


////////////////////////////////////////////////////////////////////
//     Function: PointerTo::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PointerTo<T>::
PointerTo(To *ptr) : PointerToBase<T>(ptr) { 
}

////////////////////////////////////////////////////////////////////
//     Function: PointerTo::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PointerTo<T>::
PointerTo(const PointerTo<T> &copy) : 
  PointerToBase<T>((const PointerToBase<T> &)copy) 
{ 
}

////////////////////////////////////////////////////////////////////
//     Function: PointerTo::Dereference operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PointerTo<T>::To &PointerTo<T>::
operator *() const {
  return *_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerTo::Member access operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PointerTo<T>::To *PointerTo<T>::
operator -> () const {
  return _ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerTo::Typecast operator
//       Access: Public
//  Description: We also have the typecast operator to automatically
//               convert PointerTo's to the required kind of actual
//               pointer.  This introduces ambiguities which the
//               compiler will resolve one way or the other, but we
//               don't care which way it goes because either will be
//               correct.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PointerTo<T>::
operator PointerToBase<T>::To *() const {
  return _ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerTo::p
//       Access: Public
//  Description: Returns an ordinary pointer instead of a PointerTo.
//               Useful to work around compiler problems, particularly
//               for implicit upcasts.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PointerTo<T>::To *PointerTo<T>::
p() const {
  return _ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerTo::Assignment operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PointerTo<T> &PointerTo<T>::
operator = (To *ptr) {
  reassign(ptr);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: PointerTo::Assignment operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE PointerTo<T> &PointerTo<T>::
operator = (const PointerTo<T> &copy) {
  reassign((const PointerToBase<T> &)copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE ConstPointerTo<T>::
ConstPointerTo(const To *ptr) : 
  PointerToBase<T>((ConstPointerTo<T>::To *)ptr) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE ConstPointerTo<T>::
ConstPointerTo(const PointerTo<T> &copy) : 
  PointerToBase<T>((const PointerToBase<T> &)copy) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE ConstPointerTo<T>::
ConstPointerTo(const ConstPointerTo<T> &copy) : 
  PointerToBase<T>((const PointerToBase<T> &)copy) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::Dereference operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE const ConstPointerTo<T>::To &ConstPointerTo<T>::
operator *() const {
  return *_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::Member access operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE const ConstPointerTo<T>::To *ConstPointerTo<T>::
operator -> () const {
  return _ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::Typecast operator
//       Access: Public
//  Description: We also have the typecast operator to automatically
//               convert ConstPointerTo's to the required kind of actual
//               pointer.  This introduces ambiguities which the
//               compiler will resolve one way or the other, but we
//               don't care which way it goes because either will be
//               correct.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE ConstPointerTo<T>::
operator const PointerToBase<T>::To *() const {
  return _ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::p
//       Access: Public
//  Description: Returns an ordinary pointer instead of a ConstPointerTo.
//               Useful to work around compiler problems, particularly
//               for implicit upcasts.
////////////////////////////////////////////////////////////////////
template<class T>
INLINE const ConstPointerTo<T>::To *ConstPointerTo<T>::
p() const {
  return _ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::Assignment operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE ConstPointerTo<T> &ConstPointerTo<T>::
operator = (const To *ptr) {
  reassign((To *)ptr);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::Assignment operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE ConstPointerTo<T> &ConstPointerTo<T>::
operator = (const ConstPointerTo<T> &copy) {
  reassign((const PointerToBase<T> &)copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: ConstPointerTo::Assignment operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class T>
INLINE ConstPointerTo<T> &ConstPointerTo<T>::
operator = (const PointerTo<T> &copy) {
  reassign((const PointerToBase<T> &)copy);
  return *this;
}
