/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file interrogateMakeSeq.I
 * @author drose
 * @date 2009-09-15
 */

/**
 *
 */
INLINE InterrogateMakeSeq::
InterrogateMakeSeq(InterrogateModuleDef *def) :
  InterrogateComponent(def)
{
  _length_getter = 0;
  _element_getter = 0;
}

/**
 *
 */
INLINE InterrogateMakeSeq::
InterrogateMakeSeq(const InterrogateMakeSeq &copy) {
  (*this) = copy;
}

/**
 *
 */
INLINE void InterrogateMakeSeq::
operator = (const InterrogateMakeSeq &copy) {
  InterrogateComponent::operator = (copy);
  _scoped_name = copy._scoped_name;
  _comment = copy._comment;
  _length_getter = copy._length_getter;
  _element_getter = copy._element_getter;
}

/**
 *
 */
INLINE bool InterrogateMakeSeq::
has_scoped_name() const {
  return !_scoped_name.empty();
}

/**
 *
 */
INLINE const std::string &InterrogateMakeSeq::
get_scoped_name() const {
  return _scoped_name;
}

/**
 *
 */
INLINE bool InterrogateMakeSeq::
has_comment() const {
  return !_comment.empty();
}

/**
 *
 */
INLINE const std::string &InterrogateMakeSeq::
get_comment() const {
  return _comment;
}

/**
 *
 */
INLINE FunctionIndex InterrogateMakeSeq::
get_length_getter() const {
  return _length_getter;
}

/**
 *
 */
INLINE FunctionIndex InterrogateMakeSeq::
get_element_getter() const {
  return _element_getter;
}

INLINE std::ostream &
operator << (std::ostream &out, const InterrogateMakeSeq &make_seq) {
  make_seq.output(out);
  return out;
}

INLINE std::istream &
operator >> (std::istream &in, InterrogateMakeSeq &make_seq) {
  make_seq.input(in);
  return in;
}
