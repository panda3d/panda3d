// Filename: typedWritableReferenceCount.I
// Created by:  jason (08Jun00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: TypedWritableReferenceCount::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TypedWritableReferenceCount::
TypedWritableReferenceCount() { 
  MemoryUsage::update_type(this, this);
}

////////////////////////////////////////////////////////////////////
//     Function: TypedWritableReferenceCount::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TypedWritableReferenceCount::
TypedWritableReferenceCount(const TypedWritableReferenceCount &copy) : 
  TypedWritable(copy), 
  ReferenceCount(copy) 
{
  MemoryUsage::update_type(this, this);
}

////////////////////////////////////////////////////////////////////
//     Function: TypedWritableReferenceCount::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void TypedWritableReferenceCount::
operator = (const TypedWritableReferenceCount &copy) {
  TypedWritable::operator = (copy);
  ReferenceCount::operator = (copy);
}
