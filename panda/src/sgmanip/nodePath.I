// Filename: nodePath.I
// Created by:  drose (06Mar00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: NodePath::ForwardIterator::Constructor
//       Access: Public
//  Description: This is an STL-style iterator that can be used to
//               traverse the full list of arcs in the NodePath.  Its
//               primary purpose is as a parameter to wrt() so we can
//               compute a correct relative wrt to the particular
//               instance referenced by the NodePath.
////////////////////////////////////////////////////////////////////
INLINE NodePath::ForwardIterator::
ForwardIterator(NodePath::ArcComponent *comp) : _comp(comp) {
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::ForwardIterator::Dereference Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodeRelation *NodePath::ForwardIterator::
operator * () const {
  nassertr(_comp != (ArcComponent *)NULL, NULL);
  return _comp->_arc;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::ForwardIterator::Increment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void NodePath::ForwardIterator::
operator ++() {
  nassertv(_comp != (ArcComponent *)NULL);
  _comp = _comp->_next;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::ForwardIterator::Equality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::ForwardIterator::
operator == (const NodePath::ForwardIterator &other) const {
  return _comp == other._comp;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::ForwardIterator::Inequality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::ForwardIterator::
operator != (const NodePath::ForwardIterator &other) const {
  return _comp != other._comp;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::Default Constructor
//       Access: Public
//  Description: Creates an empty NodePath.  This does not refer to
//               any nodes, and is equivalent to a NULL Node pointer;
//               it's an error to attempt to do any node operations on
//               this object.
////////////////////////////////////////////////////////////////////
INLINE NodePath::
NodePath(TypeHandle graph_type) : NodePathBase(graph_type) {
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::Constructor
//       Access: Public
//  Description: Creates a NodePath that contains just one node: the
//               top node.  It's not yet much of a path, but it does
//               reference at least the one node and can be used as an
//               ordinary node pointer.
////////////////////////////////////////////////////////////////////
INLINE NodePath::
NodePath(Node *top_node, TypeHandle graph_type) : NodePathBase(graph_type) {
  _top_node = top_node;
  nassertv(verify_connectivity());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::Constructor
//       Access: Public
//  Description: This constructor creates a new NodePath given an
//               ArcChain, which is a more primitive version of a
//               NodePath.  Some graph-traversal operations generate
//               ArcChains, so it's useful to be able to use one of
//               these to create a NodePath.
////////////////////////////////////////////////////////////////////
INLINE NodePath::
NodePath(const ArcChain &chain, TypeHandle graph_type) :
  NodePathBase(chain, graph_type)
{
  nassertv(verify_connectivity());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodePath::
NodePath(const NodePathBase &copy) : NodePathBase(copy) {
}
  
////////////////////////////////////////////////////////////////////
//     Function: NodePath::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
operator = (const NodePath &copy) {
  NodePathBase::operator = (copy);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodePath::
~NodePath() {
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::Equality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
operator == (const NodePath &other) const {
  return compare_to(other) == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::Inequality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
operator != (const NodePath &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_graph_type
//       Access: Public
//  Description: Changes the type of graph that the NodePath will
//               search for.  By default, this is RenderRelation.
//               This may only be called when the NodePath contains no
//               arcs (e.g. is_empty() || is_singleton()).
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_graph_type(TypeHandle graph_type) {
  nassertv(_head == (ArcComponent *)NULL);
  _graph_type = graph_type;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_graph_type
//       Access: Public
//  Description: Returns the type of graph that the NodePath is
//               currently set to.
////////////////////////////////////////////////////////////////////
INLINE TypeHandle NodePath::
get_graph_type() const {
  return _graph_type;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_max_search_depth
//       Access: Public, Static
//  Description: Certain operations, such as extend_down_to() or
//               find_all_matches(), require a traversal of the scene
//               graph to search for the target node or nodes.  This
//               traversal does not attempt to detect cycles, so an
//               arbitrary cap is set on the depth of the traversal as
//               a poor man's cycle detection, in the event that a
//               cycle has inadvertently been introduced into the
//               scene graph.
//
//               There may be other reasons you'd want to truncate a
//               search before the bottom of the scene graph has been
//               reached.  In any event, this function sets the limit
//               on the number of levels that a traversal will
//               continue, and hence the maximum length of a path that
//               may be returned by a traversal.
//
//               This is a static method, and so changing this
//               parameter affects all of the NodePaths in the
//               universe.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_max_search_depth(int max_search_depth) {
  _max_search_depth = max_search_depth;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_max_search_depth
//       Access: Public, Static
//  Description: Returns the current setting of the search depth
//               limit.  See set_max_search_depth.
////////////////////////////////////////////////////////////////////
INLINE int NodePath::
get_max_search_depth() {
  return _max_search_depth;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::is_empty
//       Access: Public
//  Description: Returns true if the NodePath contains no nodes and no
//               arcs.
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
is_empty() const {
  return (_head == (ArcComponent *)NULL && _top_node == (Node *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::is_singleton
//       Access: Public
//  Description: Returns true if the NodePath contains exactly one
//               node, and no arcs.
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
is_singleton() const {
  return (_head == (ArcComponent *)NULL && _top_node != (Node *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::has_arcs
//       Access: Public
//  Description: Returns true if the NodePath contains at least one
//               arc, and therefore at least two nodes.  This is the
//               same thing as asking get_num_arcs() > 0, but is
//               easier to compute.
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
has_arcs() const {
  return (_head != (ArcComponent *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_num_nodes
//       Access: Public
//  Description: Returns the number of nodes in the path, including
//               the top node.
////////////////////////////////////////////////////////////////////
INLINE int NodePath::
get_num_nodes() const {
  if (is_empty()) {
    return 0;
  }
  return get_num_arcs() + 1;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::node
//       Access: Public
//  Description: Returns the bottom node of the path, or NULL if the
//               path is empty.
////////////////////////////////////////////////////////////////////
INLINE Node *NodePath::
node() const {
  if (_head == (ArcComponent *)NULL) {
    // A singleton or empty list.
    return _top_node;
  }
  return _head->_arc->get_child();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::arc
//       Access: Public
//  Description: Returns the bottom arc of the path, or NULL if the
//               path is empty or is a singleton.
////////////////////////////////////////////////////////////////////
INLINE NodeRelation *NodePath::
arc() const {
  if (_head == (ArcComponent *)NULL) {
    // A singleton or empty list.
    return (NodeRelation *)NULL;
  }
  return _head->_arc;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_num_children
//       Access: Public
//  Description: Returns the number of children of the bottom node of
//               the NodePath.  This will be the same as the number of
//               paths in the collection returned by get_children().
////////////////////////////////////////////////////////////////////
INLINE int NodePath::
get_num_children() const {
  nassertr(verify_connectivity(), 0);
  nassertr(!is_empty(), 0);

  return node()->get_num_children(_graph_type);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_child
//       Access: Public
//  Description: Returns the nth of child of the bottom node of
//               the NodePath.  This will be the same as nth path in
//               the collection returned by get_children().
////////////////////////////////////////////////////////////////////
INLINE NodePath NodePath::
get_child(int n) const {
  nassertr(verify_connectivity(), NodePath());
  nassertr(!is_empty(), NodePath());
  nassertr(n >= 0 && n < get_num_children(), NodePath());

  NodePath result(*this);
  result.extend_by(node()->get_child(_graph_type, n));
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::has_parent
//       Access: Public
//  Description: Returns true if the node at the bottom of the
//               NodePath has a parent; i.e. the NodePath contains at
//               least two nodes.
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
has_parent() const {
  return has_arcs();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_parent
//       Access: Public
//  Description: Returns the NodePath to the parent node of the bottom
//               arc: that is, this NodePath, shortened by one node.
////////////////////////////////////////////////////////////////////
INLINE NodePath NodePath::
get_parent() const {
  nassertr(has_parent(), NodePath());
  NodePath parent(*this);
  parent.shorten();
  return parent;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::find_path_down_to
//       Access: Public
//  Description: Returns a NodePath that represents the extension of
//               this NodePath down to the indicated node along the
//               shortest possible path, if any, or an empty NodePath
//               if there is no connection to the indicated node.
////////////////////////////////////////////////////////////////////
INLINE NodePath NodePath::
find_path_down_to(Node *dnode) const {
  NodePath result(*this);
  if (result.extend_down_to(dnode)) {
    return result;
  }
  return NodePath();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::find
//       Access: Public
//  Description: Searches for a node below this NodePath's bottom node
//               that matches the indicated string.  Returns the
//               shortest match found, if any, or an empty NodePath if
//               no match can be found.
////////////////////////////////////////////////////////////////////
INLINE NodePath NodePath::
find(const string &path) const {
  NodePath result(*this);
  if (result.extend_by(path)) {
    return result;
  }
  return NodePath();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::attach_new_node
//       Access: Public
//  Description: Creates a new NamedNode and attaches it below the
//               current NodePath, returning a new NodePath that
//               references it.
////////////////////////////////////////////////////////////////////
INLINE NodePath NodePath::
attach_new_node(const string &name, int sort) const {
  nassertr(verify_connectivity(), NodePath());
  nassertr(!is_empty(), NodePath(_graph_type));

  return attach_new_node(new NamedNode(name), sort);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
output(ostream &out) const {
  out << as_string(0);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::ls
//       Access: Public
//  Description: Lists all the nodes at and below the current path
//               hierarchically.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
ls() const {
  ls(nout);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::ls
//       Access: Public
//  Description: Lists all the nodes at and below the current path
//               hierarchically.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
ls(ostream &out, int indent_level) const {
  nassertv(verify_connectivity());
  nassertv(!is_empty());

  r_list_descendants(out, indent_level);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::ls_transitions
//       Access: Public
//  Description: Lists all the nodes at and below the current path
//               hierarchically, along with all the transitions on the
//               arcs between them.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
ls_transitions() const {
  ls_transitions(nout);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::ls_transitions
//       Access: Public
//  Description: Lists all the nodes at and below the current path
//               hierarchically, along with all the transitions on the
//               arcs between them.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
ls_transitions(ostream &out, int indent_level) const {
  nassertv(verify_connectivity());
  nassertv(!is_empty());

  r_list_transitions(out, indent_level);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos
//       Access: Public
//  Description: Sets the translation component of the transform,
//               leaving rotation and scale untouched.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_pos(float x, float y, float z) {
  set_pos(LPoint3f(x, y, z));
}

INLINE float NodePath::
get_x() const {
  return get_pos()[0];
}

INLINE float NodePath::
get_y() const {
  return get_pos()[1];
}

INLINE float NodePath::
get_z() const {
  return get_pos()[2];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_hpr
//       Access: Public
//  Description: Sets the rotation component of the transform,
//               leaving translation and scale untouched.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_hpr(float h, float p, float r) {
  set_hpr(LVecBase3f(h, p, r));
}

INLINE float NodePath::
get_h() const {
  return get_hpr()[0];
}

INLINE float NodePath::
get_p() const {
  return get_hpr()[1];
}

INLINE float NodePath::
get_r() const {
  return get_hpr()[2];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_scale
//       Access: Public
//  Description: Sets the scale component of the transform,
//               leaving translation and rotation untouched.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_scale(float scale) {
  set_scale(LVecBase3f(scale, scale, scale));
}

INLINE void NodePath::
set_scale(float sx, float sy, float sz) {
  set_scale(LVecBase3f(sx, sy, sz));
}

INLINE float NodePath::
get_sx() const {
  return get_scale()[0];
}

INLINE float NodePath::
get_sy() const {
  return get_scale()[1];
}

INLINE float NodePath::
get_sz() const {
  return get_scale()[2];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_color_scale
//       Access: Public
//  Description: Sets the color scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_color_scale(float sr, float sg, float sb, float sa) {
  set_color_scale(LVecBase4f(sr, sg, sb, sa));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_sr
//       Access: Public
//  Description: Sets the red scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_sr(float sr) {
  LVecBase4f new_scale = get_color_scale();
  new_scale[0] = sr;

  set_color_scale(new_scale);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_sg
//       Access: Public
//  Description: Sets the alpha scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_sg(float sg) {
  LVecBase4f new_scale = get_color_scale();
  new_scale[1] = sg;

  set_color_scale(new_scale);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_sb
//       Access: Public
//  Description: Sets the blue scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_sb(float sb) {
  LVecBase4f new_scale = get_color_scale();
  new_scale[2] = sb;

  set_color_scale(new_scale);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_sa
//       Access: Public
//  Description: Sets the alpha scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_sa(float sa) {
  LVecBase4f new_scale = get_color_scale();
  new_scale[3] = sa;

  set_color_scale(new_scale);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_sr
//       Access: Public
//  Description: Gets the red scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE float NodePath::
get_sr() const {
  return get_color_scale()[0];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_sg
//       Access: Public
//  Description: Gets the green scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE float NodePath::
get_sg() const {
  return get_color_scale()[1];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_sb
//       Access: Public
//  Description: Gets the blue scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE float NodePath::
get_sb() const {
  return get_color_scale()[2];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_sa
//       Access: Public
//  Description: Gets the alpha scale component of the transform
////////////////////////////////////////////////////////////////////
INLINE float NodePath::
get_sa() const {
  return get_color_scale()[3];
}


////////////////////////////////////////////////////////////////////
//     Function: NodePath::clear_color_scale
//       Access: Public
//  Description: Completely removes any color scale from the bottom arc.
//               This is preferable to simply setting the color scale to
//               identity, as it also removes the overhead associated
//               with having a color scale at all.
// 
//               This method is not strictly accurate as it clears
//               both a color matrix and alpha transform, and either of
//               those could have only offset components
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
clear_color_scale() {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  _head->_arc->clear_transition(ColorMatrixTransition::get_class_type());
  _head->_arc->clear_transition(AlphaTransformTransition::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::has_color_scale
//       Access: Public
//  Description: Returns true if a color scale has been applied
//               to the bottom arc, false otherwise.
// 
//               This method is not strictly accurate as it checks
//               for a color matrix or alpha transform, and either of
//               those could have only offset components
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
has_color_scale() const {
  nassertr(has_arcs(), false);
  nassertr(_head != (ArcComponent *)NULL, false);
  return _head->_arc->has_transition(ColorMatrixTransition::get_class_type()) ||
         _head->_arc->has_transition(AlphaTransformTransition::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos_hpr
//       Access: Public
//  Description: Sets the translation and rotation component of the
//               transform, leaving scale untouched.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_pos_hpr(float x, float y, float z, float h, float p, float r) {
  set_pos_hpr(LVecBase3f(x, y, z), LVecBase3f(h, p, r));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos_hpr_scale
//       Access: Public
//  Description: Completely replaces the transform with new
//               translation, rotation, and scale components.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_pos_hpr_scale(float x, float y, float z, float h, float p, float r,
		  float sx, float sy, float sz) {
  set_pos_hpr_scale(LVecBase3f(x, y, z), LVecBase3f(h, p, r),
		    LVecBase3f(sx, sy, sz));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::clear_mat
//       Access: Public
//  Description: Completely removes any transform from the bottom arc.
//               This is preferable to simply setting the matrix to
//               identity, as it also removes the overhead associated
//               with having a matrix at all.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
clear_mat() {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  _head->_arc->clear_transition(TransformTransition::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::has_mat
//       Access: Public
//  Description: Returns true if a transform matrix has been applied
//               to the bottom arc, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
has_mat() const {
  nassertr(has_arcs(), false);
  nassertr(_head != (ArcComponent *)NULL, false);
  return _head->_arc->has_transition(TransformTransition::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_mat
//       Access: Public
//  Description: Returns the complete transform matrix that has been
//               applied to the bottom arc, or the identity matrix if
//               no matrix has been applied.
////////////////////////////////////////////////////////////////////
INLINE LMatrix4f NodePath::
get_mat() const {
  nassertr(has_arcs(), LMatrix4f::ident_mat());
  nassertr(_head != (ArcComponent *)NULL, LMatrix4f::ident_mat());

  const TransformTransition *tt;
  if (!get_transition_into(tt, _head->_arc)) {
    // No relative transform.
    return LMatrix4f::ident_mat();
  }

  return tt->get_matrix();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::look_at
//       Access: Public
//  Description: Sets the transform on this NodePath so that it
//               rotates to face the indicated point in space.  This
//               will overwrite any previously existing scale on the
//               node, although it will preserve any translation.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
look_at(float x, float y, float z) {
  look_at(LPoint3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::heads_up
//       Access: Public
//  Description: Behaves like look_at(), but with a strong preference
//               to keeping the up vector oriented in the indicated
//               "up" direction.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
heads_up(float x, float y, float z) {
  heads_up(LPoint3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::print_pos
//       Access: Public
//  Description: Displays the translation component.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
print_pos() const {
  nout << get_pos() << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::print_hpr
//       Access: Public
//  Description: Displays the rotation component.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
print_hpr() const {
  nout << get_hpr() << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::print_scale
//       Access: Public
//  Description: Displays the scale component.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
print_scale() const {
  nout << get_scale() << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::print_pos_hpr
//       Access: Public
//  Description: Displays the translation and rotation component.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
print_pos_hpr() const {
  nout << get_pos() << "\n" 
       << get_hpr() << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::print_pos_hpr_scale
//       Access: Public
//  Description: Displays the translation, rotation, and scale
//               components.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
print_pos_hpr_scale() const {
  nout << get_pos() << "\n" 
       << get_hpr() << "\n"
       << get_scale() << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::print_mat
//       Access: Public
//  Description: Displays the complete transform matrix.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
print_mat() const {
  nout << get_mat() << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::print_color_scale
//       Access: Public
//  Description: Displays the scale component.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
print_color_scale() const {
  nout << get_color_scale() << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos
//       Access: Public
//  Description: Sets the translation component of the transform,
//               relative to the other node.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_pos(const NodePath &other, float x, float y, float z) {
  set_pos(other, LPoint3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_pos
//       Access: Public
//  Description: Returns the relative position of the bottom node as
//               seen from the other node.
////////////////////////////////////////////////////////////////////
INLINE LPoint3f NodePath::
get_pos(const NodePath &other) const {
  LMatrix4f mat = get_mat(other);
  return mat.get_row3(3);
}

INLINE float NodePath::
get_x(const NodePath &other) const {
  return get_pos(other)[0];
}

INLINE float NodePath::
get_y(const NodePath &other) const {
  return get_pos(other)[1];
}

INLINE float NodePath::
get_z(const NodePath &other) const {
  return get_pos(other)[2];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_hpr
//       Access: Public
//  Description: Sets the rotation component of the transform,
//               relative to the other node.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_hpr(const NodePath &other, float h, float p, float r) {
  set_hpr(other, LPoint3f(h, p, r));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_hpr
//       Access: Public
//  Description: Returns the relative orientation of the bottom node
//               as seen from the other node.
////////////////////////////////////////////////////////////////////
INLINE LVecBase3f NodePath::
get_hpr(const NodePath &other) const {
  LMatrix4f mat = get_mat(other);
  LVector3f scale, hpr, pos;
  decompose_matrix(mat, scale, hpr, pos);
  return hpr;
}

INLINE float NodePath::
get_h(const NodePath &other) const {
  return get_hpr(other)[0];
}

INLINE float NodePath::
get_p(const NodePath &other) const {
  return get_hpr(other)[1];
}

INLINE float NodePath::
get_r(const NodePath &other) const {
  return get_hpr(other)[2];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_scale
//       Access: Public
//  Description: Sets the scale component of the transform,
//               relative to the other node.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_scale(const NodePath &other, float sx, float sy, float sz) {
  set_scale(other, LPoint3f(sx, sy, sz));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_scale
//       Access: Public
//  Description: Returns the relative scale of the bottom node
//               as seen from the other node.
////////////////////////////////////////////////////////////////////
INLINE float NodePath::
get_sx(const NodePath &other) const {
  return get_scale(other)[0];
}

INLINE float NodePath::
get_sy(const NodePath &other) const {
  return get_scale(other)[1];
}

INLINE float NodePath::
get_sz(const NodePath &other) const {
  return get_scale(other)[2];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos_hpr
//       Access: Public
//  Description: Sets the translation and rotation component of the
//               transform, relative to the other node.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_pos_hpr(const NodePath &other, 
	    float x, float y, float z, 
	    float h, float p, float r) {
  set_pos_hpr(other, LVecBase3f(x, y, z), LVecBase3f(h, p, r));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_pos_hpr_scale
//       Access: Public
//  Description: Completely replaces the transform with new
//               translation, rotation, and scale components, relative
//               to the other node.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_pos_hpr_scale(const NodePath &other,
		  float x, float y, float z,
		  float h, float p, float r,
		  float sx, float sy, float sz) {
  set_pos_hpr_scale(other, LVecBase3f(x, y, z), LVecBase3f(h, p, r),
		    LVecBase3f(sx, sy, sz));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::look_at
//       Access: Public
//  Description: Sets the transform on this NodePath so that it
//               rotates to face the indicated point in space, which
//               is relative to the other NodePath.  This
//               will overwrite any previously existing scale on the
//               node, although it will preserve any translation.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
look_at(const NodePath &other, float x, float y, float z) {
  look_at(other, LPoint3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::heads_up
//       Access: Public
//  Description: Behaves like look_at(), but with a strong preference
//               to keeping the up vector oriented in the indicated
//               "up" direction.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
heads_up(const NodePath &other, float x, float y, float z) {
  heads_up(other, LPoint3f(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::print_pos
//       Access: Public
//  Description: Displays the translation component.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
print_pos(const NodePath &other) const {
  nout << get_pos(other) << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::print_hpr
//       Access: Public
//  Description: Displays the rotation component.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
print_hpr(const NodePath &other) const {
  nout << get_hpr(other) << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::print_scale
//       Access: Public
//  Description: Displays the scale component.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
print_scale(const NodePath &other) const {
  nout << get_scale(other) << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::print_pos_hpr
//       Access: Public
//  Description: Displays the translation and rotation component.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
print_pos_hpr(const NodePath &other) const {
  nout << get_pos(other) << "\n" 
       << get_hpr(other) << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::print_pos_hpr_scale
//       Access: Public
//  Description: Displays the translation, rotation, and scale
//               components.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
print_pos_hpr_scale(const NodePath &other) const {
  nout << get_pos(other) << "\n" 
       << get_hpr(other) << "\n"
       << get_scale(other) << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::print_mat
//       Access: Public
//  Description: Displays the complete transform matrix.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
print_mat(const NodePath &other) const {
  nout << get_mat(other) << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::get_distance
//       Access: Public
//  Description: Returns the straight-line distance between this
//               bottom node's coordinate frame's origin, and that of
//               the other node's origin.
////////////////////////////////////////////////////////////////////
INLINE float NodePath::
get_distance(const NodePath &other) const {
  LPoint3f pos = get_pos(other);
  return length(LVector3f(pos));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_color
//       Access: Public
//  Description: Sets the color transition for a render relation 
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_color(float r, float g, float b, float a,
	  int priority) {
  set_color(Colorf(r, g, b, a), priority);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::clear_color
//       Access: Public
//  Description: Completely removes any color adjustment from the arc.
//               This allows the natural color of the geometry, or
//               whatever color transitions might be otherwise
//               affecting the geometry, to show instead.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
clear_color() {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  _head->_arc->clear_transition(ColorTransition::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::has_color
//       Access: Public
//  Description: Returns true if a color has been applied to the given
//               arc, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
has_color() const {
  nassertr(has_arcs(), false);
  nassertr(_head != (ArcComponent *)NULL, false);
  return _head->_arc->has_transition(ColorTransition::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::clear_bin
//       Access: Public
//  Description: Completely removes any bin adjustment that may have
//               been set via set_bin() from this particular arc.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
clear_bin() {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  _head->_arc->clear_transition(TextureTransition::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::has_bin
//       Access: Public
//  Description: Returns true if the arc has been assigned to the a
//               particular rendering bin via set_bin(), false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
has_bin() const {
  nassertr(has_arcs(), false);
  nassertr(_head != (ArcComponent *)NULL, false);
  return _head->_arc->has_transition(GeomBinTransition::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::clear_texture
//       Access: Public
//  Description: Completely removes any texture adjustment that may
//               have been set via set_texture() or set_texture_off()
//               from this particular arc.  This allows whatever
//               textures might be otherwise affecting the geometry to
//               show instead.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
clear_texture() {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  _head->_arc->clear_transition(TextureTransition::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::clear_fog
//       Access: Public
//  Description: Completely removes any fog adjustment that may
//               have been set via set_fog() or set_fog_off()
//               from this particular arc.  This allows whatever
//               fogs might be otherwise affecting the geometry to
//               show instead.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
clear_fog() {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  _head->_arc->clear_transition(FogTransition::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::clear_render_mode
//       Access: Public
//  Description: Completely removes any render mode adjustment that
//               may have been set on this arc via
//               set_render_mode_wireframe() or
//               set_render_mode_filled().
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
clear_render_mode() {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  _head->_arc->clear_transition(RenderModeTransition::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::has_render_mode
//       Access: Public
//  Description: Returns true if a render mode has been explicitly set
//               on this particular arc via
//               set_render_mode_wireframe() or
//               set_render_mode_filled(), false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
has_render_mode() const {
  nassertr(has_arcs(), false);
  nassertr(_head != (ArcComponent *)NULL, false);
  return _head->_arc->has_transition(RenderModeTransition::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::clear_two_sided
//       Access: Public
//  Description: Completely removes any two-sided adjustment that
//               may have been set on this arc via set_two_sided().
//               The geometry at this level and below will
//               subsequently be rendered either two-sided or
//               one-sided, according to whatever other arcs may have
//               had set_two_sided() on it, or according to the
//               initial state otherwise.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
clear_two_sided() {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  _head->_arc->clear_transition(CullFaceTransition::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::has_two_sided
//       Access: Public
//  Description: Returns true if a two-sided adjustment has been
//               explicitly set on this particular arc via
//               set_two_sided().  If this returns true, then
//               get_two_sided() may be called to determine which has
//               been set.
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
has_two_sided() const {
  nassertr(has_arcs(), false);
  nassertr(_head != (ArcComponent *)NULL, false);
  return _head->_arc->has_transition(CullFaceTransition::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_billboard_axis
//       Access: Public
//  Description: Puts a billboard transition on the arc such that it
//               will rotate in two dimensions around the up axis.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_billboard_axis() {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

  _head->_arc->set_transition(new BillboardTransition(BillboardTransition::axis()));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_billboard_point_eye
//       Access: Public
//  Description: Puts a billboard transition on the arc such that it
//               will rotate in three dimensions about the origin,
//               keeping its up vector oriented to the top of the
//               camera.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_billboard_point_eye() {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

  _head->_arc->set_transition(new BillboardTransition(BillboardTransition::point_eye()));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::set_billboard_point_world
//       Access: Public
//  Description: Puts a billboard transition on the arc such that it
//               will rotate in three dimensions about the origin,
//               keeping its up vector oriented to the sky.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
set_billboard_point_world() {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

  _head->_arc->set_transition(new BillboardTransition(BillboardTransition::point_world()));
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::clear_billboard
//       Access: Public
//  Description: Removes any billboard transition from the arc.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
clear_billboard() {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

  _head->_arc->clear_transition(BillboardTransition::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::has_billboard
//       Access: Public
//  Description: Returns true if there is any billboard transition on
//               the arc.
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
has_billboard() const {
  nassertr(has_arcs(), false);
  nassertr(_head != (ArcComponent *)NULL, false);

  return _head->_arc->has_transition(BillboardTransition::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::clear_transparency
//       Access: Public
//  Description: Completely removes any transparency adjustment that
//               may have been set on this arc via set_transparency().
//               The geometry at this level and below will
//               subsequently be rendered either transparent or not,
//               to whatever other arcs may have had
//               set_transparency() on them, or according to the
//               initial state otherwise.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
clear_transparency() {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);
  _head->_arc->clear_transition(TransparencyTransition::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::has_transparency
//       Access: Public
//  Description: Returns true if a transparent-rendering adjustment
//               has been explicitly set on this particular arc via
//               set_transparency().  If this returns true, then
//               get_transparency() may be called to determine whether
//               transparency has been explicitly enabled or
//               explicitly disabled for this arc.
////////////////////////////////////////////////////////////////////
INLINE bool NodePath::
has_transparency() const {
  nassertr(has_arcs(), false);
  nassertr(_head != (ArcComponent *)NULL, false);
  return _head->_arc->has_transition(TransparencyTransition::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::adjust_all_priorities
//       Access: Public
//  Description: Adds the indicated adjustment amount (which may be
//               negative) to the priority for all transitions on the
//               bottom arc, and for all arcs in the subgraph below.
//               This can be used to force these nodes not to be
//               overridden by a high-level state change above.  If
//               the priority would drop below zero, it is set to
//               zero.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
adjust_all_priorities(int adjustment) {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

  r_adjust_all_priorities(_head->_arc, adjustment);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::show
//       Access: Public
//  Description: Removes any PruneTransition on this bottom arc so
//               that the geometry at this level and below will once
//               again be visible--assuming no ancestor arc has a
//               PruneTransition.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
show() {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

  _head->_arc->clear_transition(PruneTransition::get_class_type());
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::hide
//       Access: Public
//  Description: Puts a PruneTransition on this bottom arc so that the
//               geometry at this level and below will be invisible.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
hide() {
  nassertv(has_arcs());
  nassertv(_head != (ArcComponent *)NULL);

  _head->_arc->set_transition(new PruneTransition);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::show_collision_solids
//       Access: Public
//  Description: Reveals all the collision solids at or below this
//               node.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
show_collision_solids() {
  find_all_matches("**/+CollisionNode").show();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePath::hide_collision_solids
//       Access: Public
//  Description: Hides all the collision solids at or below this
//               node.
////////////////////////////////////////////////////////////////////
INLINE void NodePath::
hide_collision_solids() {
  find_all_matches("**/+CollisionNode").hide();
}
