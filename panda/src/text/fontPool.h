// Filename: fontPool.h
// Created by:  drose (31Jan03)
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

#ifndef FONTPOOL_H
#define FONTPOOL_H

#include "pandabase.h"

#include "texture.h"
#include "textFont.h"

#include "filename.h"
#include "pmap.h"

////////////////////////////////////////////////////////////////////
//       Class : FontPool
// Description : This is the preferred interface for loading fonts for
//               the TextNode system.  It is similar to ModelPool and
//               TexturePool in that it unifies references to the same
//               filename.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FontPool {
PUBLISHED:
  // These functions take string parameters instead of Filenames
  // because that's somewhat more convenient to the scripting
  // language.
  INLINE static bool has_font(const string &filename);
  INLINE static bool verify_font(const string &filename);
  INLINE static TextFont *load_font(const string &filename);
  INLINE static void add_font(const string &filename, TextFont *font);
  INLINE static void release_font(const string &filename);
  INLINE static void release_all_fonts();

  INLINE static int garbage_collect();

  INLINE static void list_contents(ostream &out);

private:
  INLINE FontPool();

  bool ns_has_font(const string &str);
  TextFont *ns_load_font(const string &str);
  void ns_add_font(const string &filename, TextFont *font);
  void ns_release_font(const string &filename);
  void ns_release_all_fonts();
  int ns_garbage_collect();
  void ns_list_contents(ostream &out);

  static void lookup_filename(const string &str,
                              Filename &filename, int &face_index);

  static FontPool *get_ptr();

  static FontPool *_global_ptr;
  typedef pmap<string,  PT(TextFont) > Fonts;
  Fonts _fonts;
};

#include "fontPool.I"

#endif
