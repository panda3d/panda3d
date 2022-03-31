/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file documentSpec.I
 * @author drose
 * @date 2003-01-28
 */

/**
 *
 */
INLINE DocumentSpec::
DocumentSpec() {
  _request_mode = RM_any;
  _cache_control = CC_allow_cache;
  _flags = 0;
}

/**
 *
 */
INLINE DocumentSpec::
DocumentSpec(const std::string &url) :
  _url(url)
{
  _request_mode = RM_any;
  _cache_control = CC_allow_cache;
  _flags = 0;
}

/**
 *
 */
INLINE DocumentSpec::
DocumentSpec(const URLSpec &url) :
  _url(url)
{
  _request_mode = RM_any;
  _cache_control = CC_allow_cache;
  _flags = 0;
}

/**
 *
 */
INLINE DocumentSpec::
DocumentSpec(const DocumentSpec &copy) :
  _url(copy._url),
  _tag(copy._tag),
  _date(copy._date),
  _request_mode(copy._request_mode),
  _cache_control(copy._cache_control),
  _flags(copy._flags)
{
}

/**
 *
 */
INLINE void DocumentSpec::
operator = (const DocumentSpec &copy) {
  _url = copy._url;
  _tag = copy._tag;
  _date = copy._date;
  _request_mode = copy._request_mode;
  _cache_control = copy._cache_control;
  _flags = copy._flags;
}

/**
 *
 */
INLINE bool DocumentSpec::
operator == (const DocumentSpec &other) const {
  return compare_to(other) == 0;
}

/**
 *
 */
INLINE bool DocumentSpec::
operator != (const DocumentSpec &other) const {
  return compare_to(other) != 0;
}

/**
 *
 */
INLINE bool DocumentSpec::
operator < (const DocumentSpec &other) const {
  return compare_to(other) < 0;
}

/**
 * Changes the URL of the DocumentSpec without modifying its other properties.
 * Normally this would be a strange thing to do, because the tag and date are
 * usually strongly associated with the URL.  To get a DocumentSpec pointing
 * to a new URL, you would normally create a new DocumentSpec object.
 */
INLINE void DocumentSpec::
set_url(const URLSpec &url) {
  _url = url;
}

/**
 * Retrieves the URL of the DocumentSpec.
 */
INLINE const URLSpec &DocumentSpec::
get_url() const {
  return _url;
}

/**
 * Changes the identity tag associated with the DocumentSpec.
 */
INLINE void DocumentSpec::
set_tag(const HTTPEntityTag &tag) {
  _tag = tag;
  _flags |= F_has_tag;
}

/**
 * Returns true if an identity tag is associated with the DocumentSpec.
 */
INLINE bool DocumentSpec::
has_tag() const {
  return (_flags & F_has_tag) != 0;
}

/**
 * Returns the identity tag associated with the DocumentSpec, if there is one.
 * It is an error to call this if has_tag() returns false.
 *
 * The identity tag is set by the HTTP server to uniquely refer to a
 * particular version of a document.
 */
INLINE const HTTPEntityTag &DocumentSpec::
get_tag() const {
  nassertr(has_tag(), _tag);
  return _tag;
}

/**
 * Removes the identity tag associated with the DocumentSpec, if there is one.
 */
INLINE void DocumentSpec::
clear_tag() {
  _flags &= ~F_has_tag;
}

/**
 * Changes the last-modified date associated with the DocumentSpec.
 */
INLINE void DocumentSpec::
set_date(const HTTPDate &date) {
  _date = date;
  _flags |= F_has_date;
}

/**
 * Returns true if a last-modified date is associated with the DocumentSpec.
 */
INLINE bool DocumentSpec::
has_date() const {
  return (_flags & F_has_date) != 0;
}

/**
 * Returns the last-modified date associated with the DocumentSpec, if there
 * is one.  It is an error to call this if has_date() returns false.
 */
INLINE const HTTPDate &DocumentSpec::
get_date() const {
  nassertr(has_date(), _date);
  return _date;
}

/**
 * Removes the last-modified date associated with the DocumentSpec, if there
 * is one.
 */
INLINE void DocumentSpec::
clear_date() {
  _flags &= ~F_has_date;
}

/**
 * Sets the request mode of this DocumentSpec.  This is only relevant when
 * using the DocumentSpec to generate a request (for instance, in
 * HTTPChannel).  This specifies whether the document request will ask the
 * server for a newer version than the indicated version, or the exact
 * version, neither, or either.
 *
 * The possible values are:
 *
 * RM_any: ignore date and tag (if specified), and retrieve any document that
 * matches the URL.  For a subrange request, if the document matches the
 * version indicated exactly, retrieve the subrange only; otherwise, retrieve
 * the entire document.
 *
 * RM_equal: request only the precise version of the document that matches the
 * particular date and/or tag exactly, if specified; fail if this version is
 * not available.
 *
 * RM_newer: request any document that is newer than the version indicated by
 * the particular date and/or tag; fail if only that version (or older
 * versions) are available.
 *
 * RM_newer_or_equal: request any document that matches the version indicated
 * by the particular date and/or tag, or is a newer version; fail if only
 * older versions are available.
 *
 * In any of the above, you may specify either or both of the last-modified
 * date and the identity tag, whichever is known to the client.
 *
 * The default mode is RM_any.
 */
INLINE void DocumentSpec::
set_request_mode(DocumentSpec::RequestMode request_mode) {
  _request_mode = request_mode;
}

/**
 * Returns the request mode of this DocumentSpec.  See set_request_mode().
 */
INLINE DocumentSpec::RequestMode DocumentSpec::
get_request_mode() const {
  return _request_mode;
}

/**
 * Specifies what kind of cached value is acceptable for this document.
 * Warning: some HTTP proxies may not respect this setting and may return a
 * cached result anyway.
 *
 * CC_allow_cache: the normal HTTP behavior; the server may return a cached
 * value if it believes it is valid.
 *
 * CC_revalidate: a proxy is forced to contact the origin server and verify
 * that is cached value is in fact still valid before it returns it.
 *
 * CC_no_cache: a proxy must not return its cached value at all, but is forced
 * to go all the way back to the origin server for the official document.
 *
 * The default mode is CC_allow_cache.
 */
INLINE void DocumentSpec::
set_cache_control(DocumentSpec::CacheControl cache_control) {
  _cache_control = cache_control;
}

/**
 * Returns the request mode of this DocumentSpec.  See set_cache_control().
 */
INLINE DocumentSpec::CacheControl DocumentSpec::
get_cache_control() const {
  return _cache_control;
}

INLINE std::istream &
operator >> (std::istream &in, DocumentSpec &doc) {
  if (!doc.input(in)) {
    in.clear(std::ios::failbit | in.rdstate());
  }
  return in;
}

INLINE std::ostream &
operator << (std::ostream &out, const DocumentSpec &doc) {
  doc.output(out);
  return out;
}
