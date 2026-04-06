/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configPage.I
 * @author drose
 * @date 2004-10-15
 */

/**
 * Sorts two pages in order based on the order in which their respective pages
 * were loaded, and the order in which they appear within the same page.
 */
INLINE bool ConfigPage::
operator < (const ConfigPage &other) const {
  // The explicit sort value is the most important setting.  It's usually zero
  // unless explicitly changed.
  if (get_sort() != other.get_sort()) {
    return get_sort() < other.get_sort();
  }

  // Within the implicitexplicit categorization, sort by the page sequence.
  // The higher page sequence is more important (since it was loaded later),
  // so it gets sorted to the front of the list.
  return get_page_seq() > other.get_page_seq();
}

/**
 * Returns the name of the page.  If the page was loaded from a .prc file,
 * this is usually the filename.
 */
INLINE const std::string &ConfigPage::
get_name() const {
  return _name;
}

/**
 * Returns true if this is the special "default" or "local" page, or false if
 * it is an ordinary page, e.g.  an implicit page loaded from a prc file at
 * startup, or an explicit page created by
 * ConfigPageManager::make_explicit_page().
 */
INLINE bool ConfigPage::
is_special() const {
  return this == get_default_page() || this == get_local_page();
}

/**
 * Returns true if the page was loaded by implicitly searching the config path
 * on startup, or false if it was explicitly loaded by dynamic code after
 * initial startup.
 */
INLINE bool ConfigPage::
is_implicit() const {
  return _implicit_load;
}

/**
 * Returns the explicit sort order of this particular ConfigPage.  See
 * set_sort().
 */
INLINE int ConfigPage::
get_sort() const {
  return _sort;
}

/**
 * Returns the sequence number of the page.
 *
 * Sequence numbers for a particular class (implicit vs.  explicit) of pages
 * are assigned as each page is loaded; each page is given a higher sequence
 * number than all the pages loaded before it.
 *
 * The implicit_load pages, which are discovered in the file system
 * automatically, have a different set of sequence numbers than the explicit
 * pages.
 */
INLINE int ConfigPage::
get_page_seq() const {
  return _page_seq;
}

/**
 * Returns the trust level associated with this page.  An untrusted page is
 * trust level 0; if the page was loaded from a signed .prc file, its trust
 * level is the index number of the certificate that signed it.  Generally, a
 * higher trust level value represents a greater level of trust.
 */
INLINE int ConfigPage::
get_trust_level() const {
  return _trust_level;
}

/**
 * Explicitly sets the trust level on this particular page.  Note that any
 * subsequent changes to the page, or to any variable declarations on it, will
 * reset the trust level to zero.
 */
INLINE void ConfigPage::
set_trust_level(int trust_level) {
  _trust_level = trust_level;
}

/**
 * Returns the raw binary signature that was found in the prc file, if any.
 * This method is probably not terribly useful for most applications.
 */
INLINE const std::string &ConfigPage::
get_signature() const {
  return _signature;
}


/**
 * Called internally when the page is changed through some API operation, this
 * is intended as a hook to mark the page untrusted.
 */
INLINE void ConfigPage::
make_dirty() {
  _trust_level = 0;
}

INLINE std::ostream &
operator << (std::ostream &out, const ConfigPage &page) {
  page.output(out);
  return out;
}
