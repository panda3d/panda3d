/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file internalName.I
 * @author masad
 * @date 2004-07-15
 */

/**
 * The public interface for constructing an InternalName pointer.  This will
 * return a new InternalName representing the indicated name, if this is the
 * first time the particular name has been requested; if the name is already
 * in use, it will return the existing pointer.
 *
 * If the string contains the '.' character, the string will be divided at the
 * dots and the so-defined hierarchy of names will be registered.  This is
 * handled transparently.
 */
INLINE PT(InternalName) InternalName::
make(const std::string &name) {
  return get_root()->append(name);
}

/**
 * This version of make() is defined such as to accept mostly string literals,
 * which may be string pooled by the compiler.  This allows us to make an
 * additional optimization.
 *
 * This constructor is also used for fixed-size arrays, since there is no neat
 * way to catch string literals only.  Presumably people aren't using
 * InternalName in that way.
 */
template<int N>
INLINE PT(InternalName) InternalName::
make(const char (&literal)[N]) {
  LightMutexHolder holder(_literal_table_lock);
  LiteralTable::const_iterator it = _literal_table.find(literal);

  if (it == _literal_table.end()) {
    PT(InternalName) name = get_root()->append(literal);
    _literal_table.insert(LiteralTable::value_type(literal, name));
    return name;
  } else {
    return it->second;
  }
}

/**
 * Return the parent of this InternalName.  All names have a parent, except
 * the root name.
 */
INLINE InternalName *InternalName::
get_parent() const {
  return _parent;
}

/**
 * Return the name represented by just this particular InternalName object,
 * ignoring its parents names.  This is everything after the rightmost dot.
 */
INLINE const std::string &InternalName::
get_basename() const {
  return _basename;
}

/**
 * Returns the standard root InternalName.  This is the root of all other
 * InternalNames.  It has no name itself, and it is the only InternalName with
 * no parent.
 */
INLINE PT(InternalName) InternalName::
get_root() {
  if (_root == nullptr) {
    _root = new InternalName(nullptr, "");
  }
  return _root;
}

/**
 * Returns the standard InternalName "error".
 */
INLINE PT(InternalName) InternalName::
get_error() {
  if (_error == nullptr) {
    _error = InternalName::make("error");
  }
  return _error;
}

/**
 * Returns the standard InternalName "vertex".  This is the column header for
 * the 3-d or 4-d vertex position information for each vertex.
 */
INLINE PT(InternalName) InternalName::
get_vertex() {
  if (_vertex == nullptr) {
    _vertex = InternalName::make("vertex");
  }
  return _vertex;
}

/**
 * Returns the standard InternalName "normal".  This is the column header for
 * the 3-d lighting normal for each vertex.
 */
INLINE PT(InternalName) InternalName::
get_normal() {
  if (_normal == nullptr) {
    _normal = InternalName::make("normal");
  }
  return _normal;
}

/**
 * Returns the standard InternalName "tangent".  This is the column header for
 * the tangent vector associated with each vertex, which is a unit vector
 * usually perpendicular to the normal and in the direction of the U texture
 * coordinate change.  It is used for deriving bump maps.
 */
INLINE PT(InternalName) InternalName::
get_tangent() {
  if (_tangent == nullptr) {
    _tangent = InternalName::make("tangent");
  }
  return _tangent;
}

/**
 * Returns the InternalName "tangent.name", where name is the supplied string.
 * This is the column header for the tangent associated with the named texture
 * coordinate set.
 */
INLINE PT(InternalName) InternalName::
get_tangent_name(const std::string &name) {
  return get_tangent()->append(name);
}

/**
 * Returns the standard InternalName "binormal".  This is the column header
 * for the tangent vector associated with each vertex, which is a unit vector
 * usually perpendicular to both the normal and the tangent, and in the
 * direction of the V texture coordinate change.  It is used for deriving bump
 * maps.
 */
INLINE PT(InternalName) InternalName::
get_binormal() {
  if (_binormal == nullptr) {
    _binormal = InternalName::make("binormal");
  }
  return _binormal;
}

/**
 * Returns the InternalName "binormal.name", where name is the supplied
 * string.  This is the column header for the binormal associated with the
 * named texture coordinate set.
 */
INLINE PT(InternalName) InternalName::
get_binormal_name(const std::string &name) {
  return get_binormal()->append(name);
}

/**
 * Returns the standard InternalName "texcoord".  This is the column header
 * for the default texture coordinate set for each vertex.  It is also used
 * for identifying the default texture coordinate set in a TextureStage.
 */
INLINE PT(InternalName) InternalName::
get_texcoord() {
  if (_texcoord == nullptr) {
    _texcoord = InternalName::make("texcoord");
  }
  return _texcoord;
}

/**
 * Returns the InternalName "texcoord.name", where name is the supplied
 * string.  This is the column header for the named texture coordinate set for
 * each vertex.  It is also used for identifying the named texture coordinate
 * set in a TextureStage.
 */
INLINE PT(InternalName) InternalName::
get_texcoord_name(const std::string &name) {
  return get_texcoord()->append(name);
}

/**
 * Returns the standard InternalName "color".  This is the column header for
 * the 4-component color value for each vertex.
 */
INLINE PT(InternalName) InternalName::
get_color() {
  if (_color == nullptr) {
    _color = InternalName::make("color");
  }
  return _color;
}

/**
 * Returns the standard InternalName "rotate".  This is the column header for
 * the floating-point rotate value, which represents a number of degrees
 * counter-clockwise to rotate each point or point sprite.
 */
INLINE PT(InternalName) InternalName::
get_rotate() {
  if (_rotate == nullptr) {
    _rotate = InternalName::make("rotate");
  }
  return _rotate;
}

/**
 * Returns the standard InternalName "size".  This is the column header for
 * the floating-point size value, which overrides the thickness parameter of
 * the RenderModeAttrib on a per-vertex (e.g.  per-point) basis.
 */
INLINE PT(InternalName) InternalName::
get_size() {
  if (_size == nullptr) {
    _size = InternalName::make("size");
  }
  return _size;
}

/**
 * Returns the standard InternalName "aspect_ratio". This is the column header
 * for the floating-point aspect ratio value, which is used to define non-
 * square points.  This number is the ratio x / y, where y is the point size
 * (above).
 */
INLINE PT(InternalName) InternalName::
get_aspect_ratio() {
  if (_aspect_ratio == nullptr) {
    _aspect_ratio = InternalName::make("aspect_ratio");
  }
  return _aspect_ratio;
}

/**
 * Returns the standard InternalName "transform_blend". This is the column
 * header for the integer transform_blend index, which is used to define
 * vertex animation on the CPU by indexing to a particular vertex weighting
 * from the TransformBlendTable.
 */
INLINE PT(InternalName) InternalName::
get_transform_blend() {
  if (_transform_blend == nullptr) {
    _transform_blend = InternalName::make("transform_blend");
  }
  return _transform_blend;
}

/**
 * Returns the standard InternalName "transform_weight". This is the column
 * header for the n-component transform_weight value, which is used in
 * conjuntion with "transform_index" to define vertex animation on the
 * graphics card.  The transform_weight value specifies the weight of the nth
 * transform.  By convention, there are 1 fewer weight values than transforms,
 * since the weights are assumed to sum to 1 (and the last value is therefore
 * implicit).
 */
INLINE PT(InternalName) InternalName::
get_transform_weight() {
  if (_transform_weight == nullptr) {
    _transform_weight = InternalName::make("transform_weight");
  }
  return _transform_weight;
}

/**
 * Returns the standard InternalName "transform_index". This is the column
 * header for the n-component transform_index value, which is used in
 * conjuntion with "transform_weight" to define vertex animation on the
 * graphics card.  The transform_index value specifies the nth transform, by
 * lookup in the TransformTable.  The transform_index column may be omitted,
 * in which case the nth transform is the nth entry in the table.
 */
INLINE PT(InternalName) InternalName::
get_transform_index() {
  if (_transform_index == nullptr) {
    _transform_index = InternalName::make("transform_index");
  }
  return _transform_index;
}

/**
 * Returns an InternalName derived from the given base column name and the
 * given slider name, which is the column header for the offset vector that
 * should be applied to the base column name when the named morph slider is
 * engaged.
 *
 * Each morph slider requires a set of n morph columns, one for each base
 * column it applies to.
 */
INLINE PT(InternalName) InternalName::
get_morph(InternalName *column, const std::string &slider) {
  // This actually returns "column.morph.slider", although that's just an
  // implementation detail--as long as it returns a consistent, unique name
  // for each combination of column and slider, everything is good.
  return column->append("morph")->append(slider);
}

/**
 * Returns the standard InternalName "index".  This is the column header for
 * the integer vertex index.  It is not used in the vertex data itself, but is
 * used in the GeomPrimitive structure to index into the vertex data.
 */
INLINE PT(InternalName) InternalName::
get_index() {
  if (_index == nullptr) {
    _index = InternalName::make("index");
  }
  return _index;
}

/**
 * Returns the standard InternalName "world".  This is used as a keyword in
 * the shader subsystem.
 */
INLINE PT(InternalName) InternalName::
get_world() {
  if (_world == nullptr) {
    _world = InternalName::make("world");
  }
  return _world;
}

/**
 * Returns the standard InternalName "camera".  This is used as a keyword in
 * the shader subsystem.
 */
INLINE PT(InternalName) InternalName::
get_camera() {
  if (_camera == nullptr) {
    _camera = InternalName::make("camera");
  }
  return _camera;
}

/**
 * Returns the standard InternalName "model".  This is used as a keyword in
 * the shader subsystem.
 */
INLINE PT(InternalName) InternalName::
get_model() {
  if (_model == nullptr) {
    _model = InternalName::make("model");
  }
  return _model;
}

/**
 * Returns the standard InternalName "view".  This is used as a keyword in the
 * shader subsystem.
 */
INLINE PT(InternalName) InternalName::
get_view() {
  if (_view == nullptr) {
    _view = InternalName::make("view");
  }
  return _view;
}

/**
 *
 */
INLINE std::ostream &
operator << (std::ostream &out, const InternalName &tcn) {
  tcn.output(out);
  return out;
}

#ifndef CPPPARSER
/**
 *
 */
INLINE CPT_InternalName::
CPT_InternalName(const InternalName *ptr) :
  ConstPointerTo<InternalName>(ptr)
{
}

/**
 *
 */
INLINE CPT_InternalName::
CPT_InternalName(const PointerTo<InternalName> &copy) :
  ConstPointerTo<InternalName>(copy)
{
}

/**
 *
 */
INLINE CPT_InternalName::
CPT_InternalName(const ConstPointerTo<InternalName> &copy) :
  ConstPointerTo<InternalName>(copy)
{
}

/**
 *
 */
INLINE CPT_InternalName::
CPT_InternalName(const std::string &name) :
  ConstPointerTo<InternalName>(InternalName::make(name))
{
}

/**
 *
 */
template<int N>
INLINE CPT_InternalName::
CPT_InternalName(const char (&literal)[N]) :
  ConstPointerTo<InternalName>(InternalName::make(literal))
{
}

/**
 *
 */
INLINE CPT_InternalName::
CPT_InternalName(PointerTo<InternalName> &&from) noexcept :
  ConstPointerTo<InternalName>(std::move(from))
{
}

/**
 *
 */
INLINE CPT_InternalName::
CPT_InternalName(ConstPointerTo<InternalName> &&from) noexcept :
  ConstPointerTo<InternalName>(std::move(from))
{
}

/**
 *
 */
INLINE CPT_InternalName &CPT_InternalName::
operator = (PointerTo<InternalName> &&from) noexcept {
  this->reassign(std::move(from));
  return *this;
}

/**
 *
 */
INLINE CPT_InternalName &CPT_InternalName::
operator = (ConstPointerTo<InternalName> &&from) noexcept {
  this->reassign(std::move(from));
  return *this;
}

/**
 *
 */
INLINE CPT_InternalName &CPT_InternalName::
operator = (const To *ptr) {
  this->reassign((To *)ptr);
  return *this;
}

/**
 *
 */
INLINE CPT_InternalName &CPT_InternalName::
operator = (const PointerTo<InternalName> &copy) {
  this->reassign((const ConstPointerTo<InternalName> &)copy);
  return *this;
}

/**
 *
 */
INLINE CPT_InternalName &CPT_InternalName::
operator = (const ConstPointerTo<InternalName> &copy) {
  this->reassign((const ConstPointerTo<InternalName> &)copy);
  return *this;
}
#endif  // CPPPARSER
