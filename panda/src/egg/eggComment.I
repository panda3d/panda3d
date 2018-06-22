/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggComment.I
 * @author drose
 * @date 1999-01-20
 */

/**
 *
 */
INLINE EggComment::
EggComment(const std::string &node_name, const std::string &comment)
  : EggNode(node_name), _comment(comment) {
}

/**
 *
 */
INLINE EggComment::
EggComment(const EggComment &copy) : EggNode(copy), _comment(copy._comment) {
}


/**
 *
 */
INLINE EggComment &EggComment::
operator = (const std::string &comment) {
  _comment = comment;
  return *this;
}


/**
 *
 */
INLINE EggComment &EggComment::
operator = (const EggComment &copy) {
  _comment = copy._comment;
  return *this;
}


/**
 *
 */
INLINE EggComment::
operator const std::string & () const {
  return _comment;
}


/**
 *
 */
INLINE void EggComment::
set_comment(const std::string &comment) {
  _comment = comment;
}


/**
 *
 */
INLINE std::string EggComment::
get_comment() const {
  return _comment;
}
