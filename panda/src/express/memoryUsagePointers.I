// Filename: memoryUsagePointers.I
// Created by:  drose (25May00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsagePointers::Entry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE MemoryUsagePointers::Entry::
Entry(ReferenceCount *ptr, TypedObject *typed_ptr,
      TypeHandle type, double age) :
  _ptr(ptr),
  _typed_ptr(typed_ptr),
  _type(type),
  _age(age)
{
  _ptr->ref();
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsagePointers::Entry::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE MemoryUsagePointers::Entry::
Entry(const Entry &copy) :
  _ptr(copy._ptr),
  _typed_ptr(copy._typed_ptr),
  _type(copy._type),
  _age(copy._age)
{
  _ptr->ref();
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsagePointers::Entry::Copy Assigment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void MemoryUsagePointers::Entry::
operator = (const Entry &copy) {
  if (_ptr != copy._ptr) {
    _ptr->unref();
    _ptr = copy._ptr;
    // We can't call unref_delete(), because we don't know what kind
    // of pointer it is precisely.  Potential leak.
    _ptr->ref();
  }
  _typed_ptr = copy._typed_ptr;
  _type = copy._type;
  _age = copy._age;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsagePointers::Entry::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE MemoryUsagePointers::Entry::
~Entry() {
  // We can't call unref_delete(), because we don't know what kind
  // of pointer it is precisely.  Potential leak.
  _ptr->unref();
}

