/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileTemplate.I
 * @author drose
 * @date 2004-10-03
 */

/**
 * Returns true if this particular template is one of the "standard" templates
 * defined by standardTemplates.x in this directory (and compiled into the
 * binary), or false if it is a user-custom template.
 */
INLINE bool XFileTemplate::
is_standard() const {
  return _is_standard;
}

/**
 * Sets whether the template is considered "open" or not.  If it is open (this
 * flag is true), the set of options is ignored and the instances of this
 * template may include any types of children.  If it is closed (false), only
 * the named types may be added.
 */
INLINE void XFileTemplate::
set_open(bool open) {
  _open = open;
}

/**
 * Returns whether the template is considered "open" or not.  If it is open
 * (this flag is true), the set of options is ignored and the instances of
 * this template may include any types of children.  If it is closed (false),
 * only the named types may be added.
 */
INLINE bool XFileTemplate::
get_open() const {
  return _open;
}

/**
 * Adds a new type to the list of allowable types of child nodes for an
 * instance of this template.
 */
INLINE void XFileTemplate::
add_option(XFileTemplate *option) {
  _options.push_back(option);
}

/**
 * Returns the number of templates on the options list.
 */
INLINE int XFileTemplate::
get_num_options() const {
  return _options.size();
}

/**
 * Returns the nth template on the options list.
 */
INLINE XFileTemplate *XFileTemplate::
get_option(int n) const {
  nassertr(n >= 0 && n < (int)_options.size(), nullptr);
  return _options[n];
}
