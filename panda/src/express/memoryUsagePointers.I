// Filename: memoryUsagePointers.I
// Created by:  drose (25May00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsagePointers::Entry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MemoryUsagePointers::Entry::
Entry(ReferenceCount *ptr, TypeHandle type, double age) :
  _ptr(ptr),
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
MemoryUsagePointers::Entry::
Entry(const Entry &copy) :
  _ptr(copy._ptr),
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
void MemoryUsagePointers::Entry::
operator = (const Entry &copy) {
  if (_ptr != copy._ptr) {
    _ptr->unref();
    _ptr = copy._ptr;
    _ptr->ref();
  }
  _type = copy._type;
  _age = copy._age;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsagePointers::Entry::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MemoryUsagePointers::Entry::
~Entry() {
  _ptr->unref();
}

