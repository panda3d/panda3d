// Filename: fontPool.cxx
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

#include "fontPool.h"
#include "config_util.h"
#include "config_express.h"
#include "virtualFileSystem.h"
#include "nodePath.h"
#include "loader.h"

FontPool *FontPool::_global_ptr = (FontPool *)NULL;

static Loader model_loader;

////////////////////////////////////////////////////////////////////
//     Function: FontPool::ns_has_font
//       Access: Private
//  Description: The nonstatic implementation of has_font().
////////////////////////////////////////////////////////////////////
bool FontPool::
ns_has_font(const string &str) {
  Filename filename;
  int face_index;
  lookup_filename(str, filename, face_index);

  Fonts::const_iterator ti;
  ti = _fonts.find(filename);
  if (ti != _fonts.end()) {
    // This font was previously loaded.
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: FontPool::ns_load_font
//       Access: Private
//  Description: The nonstatic implementation of load_font().
////////////////////////////////////////////////////////////////////
TextFont *FontPool::
ns_load_font(const string &str) {
  Filename filename;
  int face_index;
  lookup_filename(str, filename, face_index);

  Fonts::const_iterator ti;
  ti = _fonts.find(filename);
  if (ti != _fonts.end()) {
    // This font was previously loaded.
    return (*ti).second;
  }

  text_cat.info()
    << "Loading font " << filename << "\n";

  // Now, figure out how to load the font.  If its filename extension
  // is "egg" or "bam", or if it's unspecified, assume it's a model
  // file, representing a static font.
  PT(TextFont) font;

  string extension = filename.get_extension();
  if (extension.empty() || extension == "egg" || extension == "bam") {
    PT(PandaNode) node = model_loader.load_sync(filename);
    if (node != (PandaNode *)NULL) {
      // It is a model.  Elevate all the priorities by 1, and make a
      // font out of it.
      NodePath np(node);
      np.adjust_all_priorities(1);
      font = new StaticTextFont(node);
    }
  }

#ifdef HAVE_FREETYPE
  if (font == (TextFont *)NULL || !font->is_valid()) {
    // If we couldn't load the font as a model, try using FreeType to
    // load it as a font file.
    font = new DynamicTextFont(filename, face_index);
  }
#endif

  if (font == (TextFont *)NULL || !font->is_valid()) {
    // This font was not found or could not be read.
    return NULL;
  }

  _fonts[filename] = font;
  return font;
}

////////////////////////////////////////////////////////////////////
//     Function: FontPool::ns_add_font
//       Access: Private
//  Description: The nonstatic implementation of add_font().
////////////////////////////////////////////////////////////////////
void FontPool::
ns_add_font(const string &filename, TextFont *font) {
  // We blow away whatever font was there previously, if any.
  _fonts[filename] = font;
}

////////////////////////////////////////////////////////////////////
//     Function: FontPool::ns_release_font
//       Access: Private
//  Description: The nonstatic implementation of release_font().
////////////////////////////////////////////////////////////////////
void FontPool::
ns_release_font(const string &filename) {
  Fonts::iterator ti;
  ti = _fonts.find(filename);
  if (ti != _fonts.end()) {
    _fonts.erase(ti);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FontPool::ns_release_all_fonts
//       Access: Private
//  Description: The nonstatic implementation of release_all_fonts().
////////////////////////////////////////////////////////////////////
void FontPool::
ns_release_all_fonts() {
  _fonts.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: FontPool::ns_garbage_collect
//       Access: Private
//  Description: The nonstatic implementation of garbage_collect().
////////////////////////////////////////////////////////////////////
int FontPool::
ns_garbage_collect() {
  int num_released = 0;
  Fonts new_set;

  Fonts::iterator ti;
  for (ti = _fonts.begin(); ti != _fonts.end(); ++ti) {
    TextFont *font = (*ti).second;
    if (font->get_ref_count() == 1) {
      if (text_cat.is_debug()) {
        text_cat.debug()
          << "Releasing " << (*ti).first << "\n";
      }
      num_released++;
    } else {
      new_set.insert(new_set.end(), *ti);
    }
  }

  _fonts.swap(new_set);
  return num_released;
}

////////////////////////////////////////////////////////////////////
//     Function: FontPool::ns_list_contents
//       Access: Private
//  Description: The nonstatic implementation of list_contents().
////////////////////////////////////////////////////////////////////
void FontPool::
ns_list_contents(ostream &out) {
  out << _fonts.size() << " fonts:\n";
  Fonts::iterator ti;
  for (ti = _fonts.begin(); ti != _fonts.end(); ++ti) {
    TextFont *font = (*ti).second;
    out << "  " << (*ti).first
        << " (count = " << font->get_ref_count() << ")\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FontPool::lookup_filename
//       Access: Private, Static
//  Description: Accepts a font "filename", which might consist of a
//               filename followed by an optional colon and a face
//               index, and splits it out into its two components.
//               Then it looks up the filename on the model path.
//               Sets the filename and face index accordingly.
////////////////////////////////////////////////////////////////////
void FontPool::
lookup_filename(const string &str,
                Filename &filename, int &face_index) {
  int colon = (int)str.length() - 1;
  // Scan backwards over digits for a colon.
  while (colon >= 0 && isdigit(str[colon])) {
    colon--;
  }
  if (colon >= 0 && str[colon] == ':') {
    string digits = str.substr(colon + 1);
    filename = str.substr(0, colon);
    face_index = atoi(digits.c_str());

  } else {
    filename = str;
    face_index = 0;
  }

  // Now look up the filename on the model path.
  if (use_vfs) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    vfs->resolve_filename(filename, get_model_path());

  } else {
    filename.resolve_filename(get_model_path());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FontPool::get_ptr
//       Access: Private, Static
//  Description: Initializes and/or returns the global pointer to the
//               one FontPool object in the system.
////////////////////////////////////////////////////////////////////
FontPool *FontPool::
get_ptr() {
  if (_global_ptr == (FontPool *)NULL) {
    _global_ptr = new FontPool;
  }
  return _global_ptr;
}
