// Filename: documentSpec.I
// Created by:  drose (28Jan03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: DocumentSpec::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DocumentSpec::
DocumentSpec() {
  _request_mode = RM_any;
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DocumentSpec::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DocumentSpec::
DocumentSpec(const string &url) :
  _url(url)
{
  _request_mode = RM_any;
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DocumentSpec::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DocumentSpec::
DocumentSpec(const URLSpec &url) :
  _url(url)
{
  _request_mode = RM_any;
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DocumentSpec::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DocumentSpec::
DocumentSpec(const DocumentSpec &copy) :
  _url(copy._url),
  _tag(copy._tag),
  _date(copy._date),
  _request_mode(copy._request_mode),
  _flags(copy._flags)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DocumentSpec::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void DocumentSpec::
operator = (const DocumentSpec &copy) {
  _url = copy._url;
  _tag = copy._tag;
  _date = copy._date;
  _request_mode = copy._request_mode;
  _flags = copy._flags;
}

////////////////////////////////////////////////////////////////////
//     Function: DocumentSpec::operator ==
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool DocumentSpec::
operator == (const DocumentSpec &other) const {
  return compare_to(other) == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DocumentSpec::operator !=
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool DocumentSpec::
operator != (const DocumentSpec &other) const {
  return compare_to(other) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DocumentSpec::operator <
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool DocumentSpec::
operator < (const DocumentSpec &other) const {
  return compare_to(other) < 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DocumentSpec::set_url
//       Access: Published
//  Description: Changes the URL of the DocumentSpec without modifying
//               its other properties.  Normally this would be a
//               strange thing to do, because the tag and date are
//               usually strongly associated with the URL.  To get a
//               DocumentSpec pointing to a new URL, you would
//               normally create a new DocumentSpec object.
////////////////////////////////////////////////////////////////////
INLINE void DocumentSpec::
set_url(const URLSpec &url) {
  _url = url;
}

////////////////////////////////////////////////////////////////////
//     Function: DocumentSpec::get_url
//       Access: Published
//  Description: Retrieves the URL of the DocumentSpec.
////////////////////////////////////////////////////////////////////
INLINE const URLSpec &DocumentSpec::
get_url() const {
  return _url;
}

////////////////////////////////////////////////////////////////////
//     Function: DocumentSpec::set_tag
//       Access: Published
//  Description: Changes the identity tag associated with the
//               DocumentSpec.
////////////////////////////////////////////////////////////////////
INLINE void DocumentSpec::
set_tag(const HTTPEntityTag &tag) {
  _tag = tag;
  _flags |= F_has_tag;
}

////////////////////////////////////////////////////////////////////
//     Function: DocumentSpec::has_tag
//       Access: Published
//  Description: Returns true if an identity tag is associated with
//               the DocumentSpec.
////////////////////////////////////////////////////////////////////
INLINE bool DocumentSpec::
has_tag() const {
  return (_flags & F_has_tag) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DocumentSpec::get_tag
//       Access: Published
//  Description: Returns the identity tag associated with the
//               DocumentSpec, if there is one.  It is an error to
//               call this if has_tag() returns false.
//
//               The identity tag is set by the HTTP server to
//               uniquely refer to a particular version of a document.
////////////////////////////////////////////////////////////////////
INLINE const HTTPEntityTag &DocumentSpec::
get_tag() const {
  nassertr(has_tag(), _tag);
  return _tag;
}

////////////////////////////////////////////////////////////////////
//     Function: DocumentSpec::clear_tag
//       Access: Published
//  Description: Removes the identity tag associated with the
//               DocumentSpec, if there is one.
////////////////////////////////////////////////////////////////////
INLINE void DocumentSpec::
clear_tag() {
  _flags &= ~F_has_tag;
}

////////////////////////////////////////////////////////////////////
//     Function: DocumentSpec::set_date
//       Access: Published
//  Description: Changes the last-modified date associated with the
//               DocumentSpec.
////////////////////////////////////////////////////////////////////
INLINE void DocumentSpec::
set_date(const HTTPDate &date) {
  _date = date;
  _flags |= F_has_date;
}

////////////////////////////////////////////////////////////////////
//     Function: DocumentSpec::has_date
//       Access: Published
//  Description: Returns true if a last-modified date is associated
//               with the DocumentSpec.
////////////////////////////////////////////////////////////////////
INLINE bool DocumentSpec::
has_date() const {
  return (_flags & F_has_date) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DocumentSpec::get_date
//       Access: Published
//  Description: Returns the last-modified date associated with the
//               DocumentSpec, if there is one.  It is an error to
//               call this if has_date() returns false.
////////////////////////////////////////////////////////////////////
INLINE const HTTPDate &DocumentSpec::
get_date() const {
  nassertr(has_date(), _date);
  return _date;
}

////////////////////////////////////////////////////////////////////
//     Function: DocumentSpec::clear_date
//       Access: Published
//  Description: Removes the last-modified date associated with the
//               DocumentSpec, if there is one.
////////////////////////////////////////////////////////////////////
INLINE void DocumentSpec::
clear_date() {
  _flags &= ~F_has_date;
}

////////////////////////////////////////////////////////////////////
//     Function: DocumentSpec::set_request_mode
//       Access: Published
//  Description: Sets the request mode of this DocumentSpec.  This is
//               only relevant when using the DocumentSpec to generate
//               a request (for instance, in HTTPChannel).  This
//               specifies whether the document request will ask the
//               server for a newer version than the indicated
//               version, or the exact version, neither, or either.
//
//               The possible values are:
//
//                 RM_any: ignore date and tag (if specified), and
//                 retrieve any document that matches the URL.  For a
//                 subrange request, if the document matches the
//                 version indicated exactly, retrieve the subrange
//                 only; otherwise, retrieve the entire document.
//
//                 RM_equal: request only the precise version of the
//                 document that matches the particular date and/or
//                 tag exactly, if specified; fail if this version is
//                 not available.
//
//                 RM_newer: request any document that is newer than
//                 the version indicated by the particular date and/or
//                 tag; fail if only that version (or older versions)
//                 are available.
//
//                 RM_newer_or_equal: request any document that
//                 matches the version indicated by the particular
//                 date and/or tag, or is a newer version; fail if
//                 only older versions are available.
//
//               In any of the above, you may specify either or both
//               of the last-modified date and the identity tag,
//               whichever is known to the client.
//
//               The default mode is RM_any.
////////////////////////////////////////////////////////////////////
INLINE void DocumentSpec::
set_request_mode(DocumentSpec::RequestMode request_mode) {
  _request_mode = request_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: DocumentSpec::get_request_mode
//       Access: Published
//  Description: Returns the request mode of this DocumentSpec.  See
//               set_request_mode().
////////////////////////////////////////////////////////////////////
INLINE DocumentSpec::RequestMode DocumentSpec::
get_request_mode() const {
  return _request_mode;
}


INLINE ostream &
operator << (ostream &out, const DocumentSpec &doc) {
  doc.output(out);
  return out;
}
