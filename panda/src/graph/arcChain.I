// Filename: arcChain.I
// Created by:  drose (05Jan01)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::ArcComponent::Constructor
//       Access: Public
//  Description: Constructs an ArcComponent that references a node.
//               This kind of ArcComponent may *only* be at the head
//               of the chain; i.e. _next must always be NULL.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH ArcChain::ArcComponent::
ArcComponent(Node *node) :
  _next(NULL)
{
  MemoryUsage::update_type(this, get_class_type());
  _p._node = node;
  nassertv(node != (Node *)NULL);
  _p._node->ref();
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::ArcComponent::Constructor
//       Access: Public
//  Description: Constructs an ArcComponent that references an arc.
//               This kind of ArcComponent must be in the middle or
//               tail of the chain; it may not be at the head of the
//               chain; i.e. _next must never be NULL.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH ArcChain::ArcComponent::
ArcComponent(NodeRelation *arc, ArcComponent *next) :
  _next(next)
{
  MemoryUsage::update_type(this, get_class_type());
  _p._arc = arc;
  nassertv(_next != (ArcComponent *)NULL);
  nassertv(arc != (NodeRelation *)NULL);
  _p._arc->ref();
  _p._arc->ref_parent();
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::ArcComponent::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH ArcChain::ArcComponent::
ArcComponent(const ArcComponent &copy) :
  _next(copy._next)
{
  MemoryUsage::update_type(this, get_class_type());
  if (has_arc()) {
    _p._arc = copy._p._arc;
    nassertv(_p._arc != (NodeRelation *)NULL);
    _p._arc->ref();
    _p._arc->ref_parent();
  } else {
    _p._node = copy._p._node;
    nassertv(_p._node != (Node *)NULL);
    _p._node->ref();
  }
}


////////////////////////////////////////////////////////////////////
//     Function: ArcChain::ArcComponent::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH ArcChain::ArcComponent::
~ArcComponent() {
  if (has_arc()) {
    nassertv(_p._arc != (NodeRelation *)NULL);
    _p._arc->unref_parent();
    unref_delete(_p._arc);
  } else {
    nassertv(_p._node != (Node *)NULL);
    unref_delete(_p._node);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::ArcComponent::has_arc
//       Access: Public
//  Description: Returns true if this component stores an arc, false
//               if it stores a node.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool ArcChain::ArcComponent::
has_arc() const {
  return (_next != (ArcComponent *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::ArcComponent::get_arc
//       Access: Public
//  Description: Returns the arc referenced by this component.  It is
//               an error to call this unless has_arc() has already
//               returned true.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeRelation *ArcChain::ArcComponent::
get_arc() const {
  nassertr(has_arc(), (NodeRelation *)NULL);
  return _p._arc;
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::ArcComponent::get_node
//       Access: Public
//  Description: Returns the node referenced by this component.  If
//               this component references a node (i.e. has_arc() is
//               false), this will return that node; if the component
//               references an arc (has_arc() is true), this will
//               return the child node of the arc.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH Node *ArcChain::ArcComponent::
get_node() const {
  if (has_arc()) {
    return _p._arc->get_child();
  } else {
    return _p._node;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::ArcComponent::is_top_node
//       Access: Public
//  Description: Returns true if this component represents the top
//               node in the chain.  This is the one component at the
//               top of the chain that represents a node; the rest of
//               the chain represents arcs.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool ArcChain::ArcComponent::
is_top_node() const {
  return (_next == (ArcComponent *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::ArcComponent::is_top_arc
//       Access: Public
//  Description: Returns true if this component represents the top arc
//               in the chain.  This is the component just below the
//               top of the chain; the highest component in the chain
//               that still represents an arc.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool ArcChain::ArcComponent::
is_top_arc() const {
  return (_next != (ArcComponent *)NULL && _next->is_top_node());
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::ArcComponent::get_next
//       Access: Public
//  Description: Returns the next component in the chain.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH ArcChain::ArcComponent *ArcChain::ArcComponent::
get_next() const {
  return _next;
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::ArcComponent::set_next
//       Access: Public
//  Description: Changes the next component in the chain.  This breaks
//               the chain at this component and relinks it to another
//               chain.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void ArcChain::ArcComponent::
set_next(ArcComponent *next) {
  nassertv(next != (ArcComponent *)NULL);
  nassertv(_next != (ArcComponent *)NULL);

  _next = next;
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
INLINE_GRAPH ArcChain::ForwardIterator::
ForwardIterator(ArcChain::ArcComponent *comp) : _comp(comp) {
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::ForwardIterator::Dereference Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeRelation *ArcChain::ForwardIterator::
operator * () const {
  nassertr(_comp != (ArcComponent *)NULL, NULL);
  return _comp->get_arc();
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::ForwardIterator::Increment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void ArcChain::ForwardIterator::
operator ++() {
  nassertv(_comp != (ArcComponent *)NULL);
  if (_comp->is_top_arc()) {
    // We only visit arcs, not nodes.  When we reach the end of the
    // list of arcs (i.e. the "top" arc), we stop.
    _comp = (ArcComponent *)NULL;
  } else {
    _comp = _comp->get_next();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::ForwardIterator::Equality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool ArcChain::ForwardIterator::
operator == (const ArcChain::ForwardIterator &other) const {
  return _comp == other._comp;
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::ForwardIterator::Inequality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool ArcChain::ForwardIterator::
operator != (const ArcChain::ForwardIterator &other) const {
  return _comp != other._comp;
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::Default Constructor
//       Access: Public
//  Description: This constructs an empty ArcChain with no nodes.  It
//               cannot be extended, since you cannot add arcs without
//               first specifying the top node.  Use the constructor
//               that receives a node if you ever want to do anything
//               with this chain.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH ArcChain::
ArcChain() {
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::Constructor
//       Access: Public
//  Description: This constructs an empty ArcChain with a single node.
//               This chain may now be extended by repeatedly calling
//               push_back() with each arc below that node in
//               sequence.
//
//               If the Node pointer is NULL, this quietly creates an
//               empty ArcChain.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH ArcChain::
ArcChain(Node *top_node) {
  if (top_node != (Node *)NULL) {
    _head = new ArcComponent(top_node);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH ArcChain::
ArcChain(const ArcChain &copy) :
  _head(copy._head)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void ArcChain::
operator = (const ArcChain &copy) {
  _head = copy._head;
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::has_node
//       Access: Public
//  Description: Returns true if there is at least a top node in the
//               chain, or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool ArcChain::
has_node() const {
  return (_head != (ArcComponent *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::has_arcs
//       Access: Public
//  Description: Returns true if there are any arcs in the chain, or
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool ArcChain::
has_arcs() const {
  return (_head != (ArcComponent *)NULL && !_head->is_top_node());
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::begin
//       Access: Public
//  Description: Returns an iterator that can be used to traverse the
//               list of arcs.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH ArcChain::const_iterator ArcChain::
begin() const {
  if (empty()) {
    return ForwardIterator();
  } else {
    return ForwardIterator(_head);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::end
//       Access: Public
//  Description: Returns an iterator that can be used to traverse the
//               list of arcs.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH ArcChain::const_iterator ArcChain::
end() const {
  return ForwardIterator();
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::empty
//       Access: Public
//  Description: Returns true if the list of arcs returned by begin()
//               .. end() is empty (i.e. begin() == end()), false
//               otherwise.  This is the same as !has_arcs().
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool ArcChain::
empty() const {
  return (_head == (ArcComponent *)NULL || _head->is_top_node());
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::push_back
//       Access: Public
//  Description: Appends the indicated arc onto the end of the chain.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void ArcChain::
push_back(NodeRelation *arc) {
  // It is invalid to push an arc onto a chain that does not already
  // have at least a top node.
  nassertv(_head != (ArcComponent *)NULL);
  _head = new ArcComponent(arc, _head);
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::pop_back
//       Access: Public
//  Description: Removes the last arc from the end of the chain.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void ArcChain::
pop_back() {
  nassertv(_head != (ArcComponent *)NULL);
  _head = _head->get_next();

  // It is invalid to pop off the top node of the chain.
  nassertv(_head != (ArcComponent *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::back
//       Access: Public
//  Description: Returns the last arc in the chain.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH NodeRelation *ArcChain::
back() const {
  nassertr(_head != (ArcComponent *)NULL, (NodeRelation *)NULL);
  return _head->get_arc();
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::operator ==
//       Access: Public
//  Description: Returns true if the two chains are equivalent; that
//               is, if they contain the same list of arcs in the same
//               order.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool ArcChain::
operator == (const ArcChain &other) const {
  return (compare_to(other) == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::operator !=
//       Access: Public
//  Description: Returns true if the two chains are not equivalent.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool ArcChain::
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
INLINE_GRAPH bool ArcChain::
operator < (const ArcChain &other) const {
  return (compare_to(other) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::compare_to
//       Access: Public
//  Description: Returns a number less than zero if this ArcChain
//               sorts before the other one, greater than zero if it
//               sorts after, or zero if they are equivalent.
//
//               Two ArcChains are considered equivalent if they
//               consist of exactly the same list of arcs in the same
//               order.  Otherwise, they are different; different
//               ArcChains will be ranked in a consistent but
//               undefined ordering; the ordering is useful only for
//               placing the ArcChains in a sorted container like an
//               STL set.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH int ArcChain::
compare_to(const ArcChain &other) const {
  return r_compare_to(_head, other._head);
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::output
//       Access: Public
//  Description: Writes a sensible description of the ArcChain to the
//               indicated output stream.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void ArcChain::
output(ostream &out) const {
  if (_head == (ArcComponent *)NULL) {
    out << "(empty)";
  } else {
    r_output(out, _head);
  }
}

INLINE_GRAPH ostream &operator << (ostream &out, const ArcChain &arc_chain) {
  arc_chain.output(out);
  return out;
}

