// Filename: typedReferenceCount.I
// Created by:  drose (25May00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: TypedReferenceCount::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TypedReferenceCount::
TypedReferenceCount() { 
  MemoryUsage::update_type(this, TypedReferenceCount::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: TypedReferenceCount::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TypedReferenceCount::
TypedReferenceCount(const TypedReferenceCount &copy) : 
  TypedObject(copy), 
  ReferenceCount(copy) 
{
  MemoryUsage::update_type(this, TypedReferenceCount::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: TypedReferenceCount::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TypedReferenceCount::
operator = (const TypedReferenceCount &copy) {
  TypedObject::operator = (copy);
  ReferenceCount::operator = (copy);
}
