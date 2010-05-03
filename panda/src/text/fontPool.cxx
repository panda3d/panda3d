// Filename: fontPool.cxx
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

#include "fontPool.h"
#include "staticTextFont.h"
#include "dynamicTextFont.h"
#include "config_util.h"
#include "config_express.h"
#include "virtualFileSystem.h"
#include "nodePath.h"
#include "loader.h"
#include "lightMutexHolder.h"

FontPool *FontPool::_global_ptr = (FontPool *)NULL;

////////////////////////////////////////////////////////////////////
//     Function: FontPool::write
//       Access: Published, Static
//  Description: Lists the contents of the font pool to the
//               indicated output stream.
////////////////////////////////////////////////////////////////////
void FontPool::
write(ostream &out) {
  get_ptr()->ns_list_contents(out);
}

////////////////////////////////////////////////////////////////////
//     Function: FontPool::ns_has_font
//       Access: Private
//  Description: The nonstatic implementation of has_font().
////////////////////////////////////////////////////////////////////
bool FontPool::
ns_has_font(const string &str) {
  LightMutexHolder holder(_lock);

  string index_str;
  Filename filename;
  int face_index;
  lookup_filename(str, index_str, filename, face_index);

  Fonts::const_iterator ti;
  ti = _fonts.find(index_str);
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
  string index_str;
  Filename filename;
  int face_index;
  lookup_filename(str, index_str, filename, face_index);

  {
    LightMutexHolder holder(_lock);
    
    Fonts::const_iterator ti;
    ti = _fonts.find(index_str);
    if (ti != _fonts.end()) {
      // This font was previously loaded.
      return (*ti).second;
    }
  }

  text_cat.info()
    << "Loading font " << filename << "\n";

  // Now, figure out how to load the font.  If its filename extension
  // is "egg" or "bam", or if it's unspecified, assume it's a model
  // file, representing a static font.
  PT(TextFont) font;

  string extension = filename.get_extension();
  if (extension.empty() || extension == "egg" || extension == "bam") {
    Loader *model_loader = Loader::get_global_ptr();
    PT(PandaNode) node = model_loader->load_sync(filename);
    if (node != (PandaNode *)NULL) {
      // It is a model.  Elevate all the priorities by 1, and make a
      // font out of it.

      // On second thought, why should we elevate the priorities?  The
      // DynamicTextFont doesn't do this, and doing so for the
      // StaticTextFont only causes problems (it changes the default
      // ColorAttrib from pri -1 to pri 0).
      /*
      NodePath np(node);
      np.adjust_all_priorities(1);
      */

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


  {
    LightMutexHolder holder(_lock);

    // Look again.  It may have been loaded by another thread.
    Fonts::const_iterator ti;
    ti = _fonts.find(index_str);
    if (ti != _fonts.end()) {
      // This font was previously loaded.
      return (*ti).second;
    }

    _fonts[index_str] = font;
  }

  return font;
}

////////////////////////////////////////////////////////////////////
//     Function: FontPool::ns_add_font
//       Access: Private
//  Description: The nonstatic implementation of add_font().
////////////////////////////////////////////////////////////////////
void FontPool::
ns_add_font(const string &str, TextFont *font) {
  LightMutexHolder holder(_lock);

  string index_str;
  Filename filename;
  int face_index;
  lookup_filename(str, index_str, filename, face_index);

  // We blow away whatever font was there previously, if any.
  _fonts[index_str] = font;
}

////////////////////////////////////////////////////////////////////
//     Function: FontPool::ns_release_font
//       Access: Private
//  Description: The nonstatic implementation of release_font().
////////////////////////////////////////////////////////////////////
void FontPool::
ns_release_font(const string &str) {
  LightMutexHolder holder(_lock);

  string index_str;
  Filename filename;
  int face_index;
  lookup_filename(str, index_str, filename, face_index);

  Fonts::iterator ti;
  ti = _fonts.find(index_str);
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
  LightMutexHolder holder(_lock);

  _fonts.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: FontPool::ns_garbage_collect
//       Access: Private
//  Description: The nonstatic implementation of garbage_collect().
////////////////////////////////////////////////////////////////////
int FontPool::
ns_garbage_collect() {
  LightMutexHolder holder(_lock);

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
ns_list_contents(ostream &out) const {
  LightMutexHolder holder(_lock);

  out << _fonts.size() << " fonts:\n";
  Fonts::const_iterator ti;
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
//               Sets the filename and face index accordingly.  Also
//               sets index_str to be the concatenation of the
//               found filename with the face index, thus restoring
//               the original input (but normalized to contain the
//               full path.)
////////////////////////////////////////////////////////////////////
void FontPool::
lookup_filename(const string &str, string &index_str,
                Filename &filename, int &face_index) {
  int colon = (int)str.length() - 1;
  // Scan backwards over digits for a colon.
  while (colon >= 0 && isdigit(str[colon])) {
    --colon;
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
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(filename, get_model_path());

  ostringstream strm;
  strm << filename << ":" << face_index;
  index_str = strm.str();
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
