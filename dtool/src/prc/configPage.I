// Filename: configPage.I
// Created by:  drose (15Oct04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: ConfigPage::operator <
//       Access: Public
//  Description: Sorts two pages in order based on the order in
//               which their respective pages were loaded, and the
//               order in which they appear within the same page.
////////////////////////////////////////////////////////////////////
INLINE bool ConfigPage::
operator < (const ConfigPage &other) const {
  if (is_implicit() != other.is_implicit()) {
    // Explicitly-loaded pages are more important than
    // implicitly-loaded pages, so put the implicit pages at the end
    // of the list.
    return (int)is_implicit() < (int)other.is_implicit();
  }

  // Within the implicit/explicit categorization, sort by the page
  // sequence.  The higher page sequence is more important (since it
  // was loaded later), so it gets sorted to the front of the list.
  return get_page_seq() > other.get_page_seq();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPage::get_name
//       Access: Published
//  Description: Returns the name of the page.  If the page was loaded
//               from a .prc file, this is usually the filename.
////////////////////////////////////////////////////////////////////
INLINE const string &ConfigPage::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPage::is_special
//       Access: Published
//  Description: Returns true if this is the special "default" or
//               "local" page, or false if it is an ordinary page,
//               e.g. an implicit page loaded from a prc file at
//               startup, or an explicit page created by
//               ConfigPageManager::make_explicit_page().
////////////////////////////////////////////////////////////////////
INLINE bool ConfigPage::
is_special() const {
  return this == get_default_page() || this == get_local_page();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPage::is_implicit
//       Access: Published
//  Description: Returns true if the page was loaded by implicitly
//               searching the config path on startup, or false if it
//               was explicitly loaded by dynamic code after initial
//               startup.
////////////////////////////////////////////////////////////////////
INLINE bool ConfigPage::
is_implicit() const {
  return _implicit_load;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPage::get_page_seq
//       Access: Published
//  Description: Returns the sequence number of the page.  
//
//               Sequence numbers for a particular class (implicit
//               vs. explicit) of pages are assigned as each page is
//               loaded; each page is given a higher sequence number
//               than all the pages loaded before it.
//
//               The implicit_load pages, which are discovered in the
//               file system automatically, have a different set of
//               sequence numbers than the explicit pages.
////////////////////////////////////////////////////////////////////
INLINE int ConfigPage::
get_page_seq() const {
  return _page_seq;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPage::get_trust_level
//       Access: Published
//  Description: Returns the trust level associated with this page.
//               An untrusted page is trust level 0; if the page was
//               loaded from a signed .prc file, its trust level is
//               the index number of the certificate that signed it.
//               Generally, a higher trust level value represents
//               a greater level of trust.
////////////////////////////////////////////////////////////////////
INLINE int ConfigPage::
get_trust_level() const {
  return _trust_level;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigPage::get_signature
//       Access: Published
//  Description: Returns the raw binary signature that was found in
//               the prc file, if any.  This method is probably not
//               terribly useful for most applications.
////////////////////////////////////////////////////////////////////
INLINE const string &ConfigPage::
get_signature() const {
  return _signature;
}


////////////////////////////////////////////////////////////////////
//     Function: ConfigPage::make_dirty
//       Access: Private
//  Description: Called internally when the page is changed through
//               some API operation, this is intended as a hook to
//               mark the page untrusted.
////////////////////////////////////////////////////////////////////
INLINE void ConfigPage::
make_dirty() {
  _trust_level = 0;
}

INLINE ostream &
operator << (ostream &out, const ConfigPage &page) {
  page.output(out);
  return out;
}
