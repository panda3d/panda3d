// Filename: arcChain.I
// Created by:  drose (05Jan01)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::ArcComponent::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ArcChain::ArcComponent::
ArcComponent(NodeRelation *arc, ArcComponent *next) :
  _arc(arc),
  _next(next)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::ForwardIterator::Constructor
//       Access: Public
//  Description: This is an STL-style iterator that can be used to
//               traverse the full list of arcs traversed so far by
//               the ArcChain.  Its primary purpose is as a
//               parameter to wrt() so we can compute a correct
//               relative wrt to the particular instance we've reached
//               so far.
////////////////////////////////////////////////////////////////////
INLINE ArcChain::ForwardIterator::
ForwardIterator(ArcChain::ArcComponent *comp) : _comp(comp) {
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::ForwardIterator::Dereference Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodeRelation *ArcChain::ForwardIterator::
operator * () const {
  nassertr(_comp != (ArcComponent *)NULL, NULL);
  return _comp->_arc;
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::ForwardIterator::Increment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ArcChain::ForwardIterator::
operator ++() {
  nassertv(_comp != (ArcComponent *)NULL);
  _comp = _comp->_next;
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::ForwardIterator::Equality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ArcChain::ForwardIterator::
operator == (const ArcChain::ForwardIterator &other) const {
  return _comp == other._comp;
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::ForwardIterator::Inequality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ArcChain::ForwardIterator::
operator != (const ArcChain::ForwardIterator &other) const {
  return _comp != other._comp;
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ArcChain::
ArcChain() {
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ArcChain::
ArcChain(const ArcChain &copy) :
  _head(copy._head)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ArcChain::
operator = (const ArcChain &copy) {
  _head = copy._head;
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::begin
//       Access: Public
//  Description: Returns an iterator that can be used to traverse the
//               list of arcs.
////////////////////////////////////////////////////////////////////
INLINE ArcChain::const_iterator ArcChain::
begin() const {
  return ForwardIterator(_head);
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::end
//       Access: Public
//  Description: Returns an iterator that can be used to traverse the
//               list of arcs.
////////////////////////////////////////////////////////////////////
INLINE ArcChain::const_iterator ArcChain::
end() const {
  return ForwardIterator();
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::empty
//       Access: Public
//  Description: Returns true if the list of arcs returned by begin()
//               .. end() is empty (i.e. begin() == end()), false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool ArcChain::
empty() const {
  return (_head == (ArcComponent *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::push_back
//       Access: Public
//  Description: Appends the indicated arc onto the end of the chain.
////////////////////////////////////////////////////////////////////
INLINE void ArcChain::
push_back(NodeRelation *arc) {
  _head = new ArcComponent(arc, _head);
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::pop_back
//       Access: Public
//  Description: Removes the last arc from the end of the chain.
////////////////////////////////////////////////////////////////////
INLINE void ArcChain::
pop_back() {
  nassertv(_head != (ArcComponent *)NULL);
  _head = _head->_next;
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::back
//       Access: Public
//  Description: Returns the last arc in the chain.
////////////////////////////////////////////////////////////////////
INLINE NodeRelation *ArcChain::
back() const {
  nassertr(_head != (ArcComponent *)NULL, (NodeRelation *)NULL);
  return _head->_arc;
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::operator ==
//       Access: Public
//  Description: Returns true if the two chains are equivalent; that
//               is, if they contain the same list of arcs in the same
//               order.
////////////////////////////////////////////////////////////////////
INLINE bool ArcChain::
operator == (const ArcChain &other) const {
  return (compare_to(other) == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::operator !=
//       Access: Public
//  Description: Returns true if the two chains are not equivalent.
////////////////////////////////////////////////////////////////////
INLINE bool ArcChain::
operator != (const ArcChain &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::operator <
//       Access: Public
//  Description: Returns true if this ArcChain sorts before the other
//               one, false otherwise.  The sorting order of two
//               nonequivalent ArcChains is consistent but undefined,
//               and is useful only for storing ArcChains in a sorted
//               container like an STL set.
////////////////////////////////////////////////////////////////////
INLINE bool ArcChain::
operator < (const ArcChain &other) const {
  return (compare_to(other) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::output
//       Access: Public
//  Description: Writes a sensible description of the ArcChain to the
//               indicated output stream.
////////////////////////////////////////////////////////////////////
INLINE void ArcChain::
output(ostream &out) const {
  if (empty()) {
    out << "(empty)";
  } else {
    r_output(out, _head);
  }
}
