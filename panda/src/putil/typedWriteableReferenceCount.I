// Filename: typedWriteableReferenceCount.I
// Created by:  jason (08Jun00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: TypedWriteableReferenceCount::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TypedWriteableReferenceCount::
TypedWriteableReferenceCount() { 
  MemoryUsage::update_type(this, TypedWriteableReferenceCount::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: TypedWriteableReferenceCount::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TypedWriteableReferenceCount::
TypedWriteableReferenceCount(const TypedWriteableReferenceCount &copy) : 
  TypedWriteable(copy), 
  ReferenceCount(copy) 
{
  MemoryUsage::update_type(this, TypedWriteableReferenceCount::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: TypedWriteableReferenceCount::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TypedWriteableReferenceCount::
operator = (const TypedWriteableReferenceCount &copy) {
  TypedWriteable::operator = (copy);
  ReferenceCount::operator = (copy);
}
