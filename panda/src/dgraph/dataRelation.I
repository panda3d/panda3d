// Filename: dataRelation.I
// Created by:  drose (08Jun00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: DataRelation::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE DataRelation::
DataRelation(Node *from, Node *to, int sort) :
  NodeRelation(from, to, sort, get_class_type()) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: DataRelation::Constructor
//       Access: Protected
//  Description: Creates a new, unattached arc.  This constructor is
//               only intended for passing through the factory to
//               create an arc based on a particular type using
//               create_typed_arc(), below.  You shouldn't, in
//               general, attempt to create an unattached arc.
////////////////////////////////////////////////////////////////////
INLINE DataRelation::
DataRelation() :
  NodeRelation(get_class_type())
{
}

////////////////////////////////////////////////////////////////////
//     Function: DataRelation::register_with_factory
//       Access: Public, Static
//  Description: Registers the type DataRelation with the factory.
//               This is only intended to be called at initialization
//               time; don't call it directly.
////////////////////////////////////////////////////////////////////
INLINE void DataRelation::
register_with_factory() {
  get_factory().register_factory(get_class_type(), make_arc);
}
