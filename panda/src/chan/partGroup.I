// Filename: partGroup.I
// Created by:  drose (22Feb99)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: PartGroup::Default Constructor
//       Access: Protected
//  Description: This constructor is only intended for interal use and
//               for derived classes.  You should normally use the
//               non-default constructor, below.
////////////////////////////////////////////////////////////////////
INLINE PartGroup::
PartGroup(const string &name) : 
  Namable(name) 
{ 
}

////////////////////////////////////////////////////////////////////
//     Function: PartGroup::Copy Constructor
//       Access: Protected
//  Description: This constructor is only intended for interal use and
//               for derived classes.  You should normally use the
//               make_copy() interface to make copies..
////////////////////////////////////////////////////////////////////
INLINE PartGroup::
PartGroup(const PartGroup &copy) :
  Namable(copy)
{
  // We don't copy children in the copy constructor.  However,
  // copy_subgraph() will do this.
}
