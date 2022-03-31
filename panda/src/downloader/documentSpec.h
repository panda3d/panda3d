/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file documentSpec.h
 * @author drose
 * @date 2003-01-28
 */

#ifndef DOCUMENTSPEC_H
#define DOCUMENTSPEC_H

#include "pandabase.h"
#include "urlSpec.h"
#include "httpEntityTag.h"
#include "httpDate.h"

/**
 * A descriptor that refers to a particular version of a document.  This
 * includes the URL of the document and its identity tag and last-modified
 * dates.
 *
 * The DocumentSpec may also be used to request a newer document than a
 * particular one if available, for instance to refresh a cached document.
 */
class EXPCL_PANDA_DOWNLOADER DocumentSpec {
PUBLISHED:
  INLINE DocumentSpec();
  INLINE DocumentSpec(const std::string &url);
  INLINE DocumentSpec(const URLSpec &url);
  INLINE DocumentSpec(const DocumentSpec &copy);
  INLINE void operator = (const DocumentSpec &copy);

  INLINE bool operator == (const DocumentSpec &other) const;
  INLINE bool operator != (const DocumentSpec &other) const;
  INLINE bool operator < (const DocumentSpec &other) const;
  int compare_to(const DocumentSpec &other) const;

  INLINE void set_url(const URLSpec &url);
  INLINE const URLSpec &get_url() const;

  INLINE void set_tag(const HTTPEntityTag &tag);
  INLINE bool has_tag() const;
  INLINE const HTTPEntityTag &get_tag() const;
  INLINE void clear_tag();

  INLINE void set_date(const HTTPDate &date);
  INLINE bool has_date() const;
  INLINE const HTTPDate &get_date() const;
  INLINE void clear_date();

  enum RequestMode {
    RM_any,
    RM_equal,
    RM_newer,
    RM_equal_or_newer,
  };

  INLINE void set_request_mode(RequestMode request_mode);
  INLINE RequestMode get_request_mode() const;

  enum CacheControl {
    CC_allow_cache,
    CC_revalidate,
    CC_no_cache,
  };

  INLINE void set_cache_control(CacheControl cache_control);
  INLINE CacheControl get_cache_control() const;

  bool input(std::istream &in);
  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;

PUBLISHED:
  MAKE_PROPERTY(url, get_url, set_url);
  MAKE_PROPERTY2(tag, has_tag, get_tag, set_tag, clear_tag);
  MAKE_PROPERTY2(date, has_date, get_date, set_date, clear_date);

  MAKE_PROPERTY(request_mode, get_request_mode, set_request_mode);
  MAKE_PROPERTY(cache_control, get_cache_control, set_cache_control);

private:
  URLSpec _url;
  HTTPEntityTag _tag;
  HTTPDate _date;
  RequestMode _request_mode;
  CacheControl _cache_control;

  enum Flags {
    F_has_tag    = 0x0001,
    F_has_date   = 0x0002,
  };
  int _flags;
};

INLINE std::istream &operator >> (std::istream &in, DocumentSpec &doc);
INLINE std::ostream &operator << (std::ostream &out, const DocumentSpec &doc);

#include "documentSpec.I"

#endif
