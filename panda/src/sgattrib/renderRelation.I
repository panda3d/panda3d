// Filename: renderRelation.I
// Created by:  mike (24Sep99)
// 

////////////////////////////////////////////////////////////////////
//     Function: RenderRelation::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE RenderRelation::
RenderRelation(Node *from, Node *to, int sort) :
  NodeRelation(from, to, sort, get_class_type()) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: RenderRelation::Constructor
//       Access: Protected
//  Description: Creates a new, unattached arc.  This constructor is
//               only intended for passing through the factory to
//               create an arc based on a particular type using
//               create_typed_arc(), below.  You shouldn't, in
//               general, attempt to create an unattached arc.
////////////////////////////////////////////////////////////////////
INLINE RenderRelation::
RenderRelation() :
  NodeRelation(get_class_type())
{
}

////////////////////////////////////////////////////////////////////
//     Function: RenderRelation::register_with_factory
//       Access: Public, Static
//  Description: Registers the type RenderRelation with the factory.
//               This is only intended to be called at initialization
//               time; don't call it directly.
////////////////////////////////////////////////////////////////////
INLINE void RenderRelation::
register_with_factory() {
  get_factory().register_factory(get_class_type(), make_arc);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderRelation::get_class_type
//       Access: Public, Static
//  Description: Returns the TypeHandle associated with this type.
////////////////////////////////////////////////////////////////////
INLINE TypeHandle RenderRelation::
get_class_type() {
  return _type_handle;
}
