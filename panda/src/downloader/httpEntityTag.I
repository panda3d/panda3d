/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpEntityTag.I
 * @author drose
 * @date 2003-01-28
 */

/**
 *
 */
INLINE HTTPEntityTag::
HTTPEntityTag() {
  _weak = false;
}

/**
 * This constructor accepts an explicit weak flag and a literal (not quoted)
 * tag string.
 */
INLINE HTTPEntityTag::
HTTPEntityTag(bool weak, const std::string &tag) :
  _weak(weak),
  _tag(tag)
{
}

/**
 *
 */
INLINE HTTPEntityTag::
HTTPEntityTag(const HTTPEntityTag &copy) :
  _weak(copy._weak),
  _tag(copy._tag)
{
}

/**
 *
 */
INLINE void HTTPEntityTag::
operator = (const HTTPEntityTag &copy) {
  _weak = copy._weak;
  _tag = copy._tag;
}

/**
 * Returns true if the entity tag is marked as "weak". A consistent weak
 * entity tag does not guarantee that its resource has not changed in any way,
 * but it does promise that the resource has not changed in any semantically
 * meaningful way.
 */
INLINE bool HTTPEntityTag::
is_weak() const {
  return _weak;
}

/**
 * Returns the tag as a literal string.
 */
INLINE const std::string &HTTPEntityTag::
get_tag() const {
  return _tag;
}

/**
 * Returns true if the two tags have "strong" equivalence: they are the same
 * tag, and both are "strong".
 */
INLINE bool HTTPEntityTag::
strong_equiv(const HTTPEntityTag &other) const {
  return _tag == other._tag && !_weak && !other._weak;
}

/**
 * Returns true if the two tags have "weak" equivalence: they are the same
 * tag, and one or both may be "weak".
 */
INLINE bool HTTPEntityTag::
weak_equiv(const HTTPEntityTag &other) const {
  return _tag == other._tag;
}

/**
 * The == operator tests object equivalence; see also strong_equiv() and
 * weak_equiv() for the two kinds of HTTP equivalence.
 */
INLINE bool HTTPEntityTag::
operator == (const HTTPEntityTag &other) const {
  return _weak == other._weak && _tag == other._tag;
}

/**
 *
 */
INLINE bool HTTPEntityTag::
operator != (const HTTPEntityTag &other) const {
  return !operator == (other);
}

/**
 *
 */
INLINE bool HTTPEntityTag::
operator < (const HTTPEntityTag &other) const {
  if (_weak != other._weak) {
    return (int)_weak < (int)other._weak;
  }
  return _tag < other._tag;
}

/**
 * Returns a number less than zero if this HTTPEntityTag sorts before the
 * other one, greater than zero if it sorts after, or zero if they are
 * equivalent.
 */
INLINE int HTTPEntityTag::
compare_to(const HTTPEntityTag &other) const {
  if (_weak != other._weak) {
    return (int)_weak - (int)other._weak;
  }
  return strcmp(_tag.c_str(), other._tag.c_str());
}

/**
 *
 */
INLINE void HTTPEntityTag::
output(std::ostream &out) const {
  out << get_string();
}


INLINE std::ostream &
operator << (std::ostream &out, const HTTPEntityTag &entityTag) {
  entityTag.output(out);
  return out;
}
