// Filename: geomBinBackToFront.I
// Created by:  drose (13Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "cullTraverser.h"

////////////////////////////////////////////////////////////////////
//     Function: GeomBinBackToFront::NodeEntry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GeomBinBackToFront::NodeEntry::
NodeEntry(float distance, const PT(CullState) &state, 
          const ArcChain &arc_chain, bool is_direct) :
  _distance(distance),
  _state(state),
  _arc_chain(arc_chain),
  _is_direct(is_direct)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinBackToFront::NodeEntry::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GeomBinBackToFront::NodeEntry::
NodeEntry(const GeomBinBackToFront::NodeEntry &copy) :
  _distance(copy._distance),
  _state(copy._state),
  _arc_chain(copy._arc_chain),
  _is_direct(copy._is_direct)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinBackToFront::NodeEntry::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void GeomBinBackToFront::NodeEntry::
operator = (const NodeEntry &copy) {
  _distance = copy._distance;
  _state = copy._state;
  _arc_chain = copy._arc_chain;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinBackToFront::NodeEntry::Ordering Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool GeomBinBackToFront::NodeEntry::
operator < (const NodeEntry &other) const {
  return _distance > other._distance;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinBackToFront::NodeEntry::draw
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void GeomBinBackToFront::NodeEntry::
draw(CullTraverser *trav) const {
  nassertv(!_arc_chain.empty());
  if (_is_direct) {
    trav->draw_direct(_arc_chain, AllAttributesWrapper(_state->get_attributes()));
  } else {
    trav->draw_geom(_arc_chain, AllAttributesWrapper(_state->get_attributes()));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinBackToFront::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GeomBinBackToFront::
GeomBinBackToFront(const string &name) :
  GeomBin(name) 
{
}
