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
EggComment(std::string node_name, std::string comment)
  : EggNode(std::move(node_name)), _comment(std::move(comment)) {
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
operator = (std::string comment) {
  _comment = std::move(comment);
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
set_comment(std::string comment) {
  _comment = std::move(comment);
}


/**
 *
 */
INLINE std::string EggComment::
get_comment() const {
  return _comment;
}
