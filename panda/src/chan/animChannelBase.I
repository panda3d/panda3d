// Filename: animChannelBase.I
// Created by:  drose (19Feb99)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelBase::Protected constructor
//       Access: Protected
//  Description: Don't use this constructor.  It exists only so that
//               AnimChannelFixed may define itself outside of the
//               hierarchy.  Normally, an AnimChannel must be created
//               as part of a hierarchy.
////////////////////////////////////////////////////////////////////
INLINE AnimChannelBase::
AnimChannelBase(const string &name)
  : AnimGroup(name)
{
  _last_frame = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelBase::Constructor
//       Access: Public
//  Description: This is the normal constructor, which automatically
//               places the AnimChannel in the previously-created
//               hierarchy.
////////////////////////////////////////////////////////////////////
INLINE AnimChannelBase::
AnimChannelBase(AnimGroup *parent, const string &name)
  : AnimGroup(parent, name)
{
  _last_frame = -1;
}


