// Filename: fontPool.h
// Created by:  drose (31Jan03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef FONTPOOL_H
#define FONTPOOL_H

#include "pandabase.h"

#include "texture.h"
#include "textFont.h"
#include "filename.h"
#include "lightMutex.h"
#include "pmap.h"

////////////////////////////////////////////////////////////////////
//       Class : FontPool
// Description : This is the preferred interface for loading fonts for
//               the TextNode system.  It is similar to ModelPool and
//               TexturePool in that it unifies references to the same
//               filename.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_TEXT FontPool {
PUBLISHED:
  // These functions take string parameters instead of Filenames
  // because the parameters may not be entirely an actual filename:
  // they may be a filename followed by a face index.

  INLINE static bool has_font(const string &filename);
  INLINE static bool verify_font(const string &filename);
  BLOCKING INLINE static TextFont *load_font(const string &filename);
  INLINE static void add_font(const string &filename, TextFont *font);
  INLINE static void release_font(const string &filename);
  INLINE static void release_all_fonts();

  INLINE static int garbage_collect();

  INLINE static void list_contents(ostream &out);
  static void write(ostream &out);

private:
  INLINE FontPool();

  bool ns_has_font(const string &str);
  TextFont *ns_load_font(const string &str);
  void ns_add_font(const string &str, TextFont *font);
  void ns_release_font(const string &str);
  void ns_release_all_fonts();
  int ns_garbage_collect();
  void ns_list_contents(ostream &out) const;

  static void lookup_filename(const string &str, string &index_str,
                              Filename &filename, int &face_index);

  static FontPool *get_ptr();
  static FontPool *_global_ptr;

  LightMutex _lock;
  typedef pmap<string,  PT(TextFont) > Fonts;
  Fonts _fonts;
};

#include "fontPool.I"

#endif
