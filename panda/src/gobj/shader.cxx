// Filename: shader.cxx
// Created by: jyelon (Sep05)
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

#include "pandabase.h"
#include "shader.h"

TypeHandle Shader::_type_handle;

////////////////////////////////////////////////////////////////////
//  Function: Shader::Constructor
//  Access: Public
//  Description: Construct a Shader.
////////////////////////////////////////////////////////////////////
Shader::
Shader(const string &text, const string &file) {
  _text = text;
  _file = file;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::Destructor
//       Access: Public
//  Description: Delete the compiled code, if it exists.
////////////////////////////////////////////////////////////////////
Shader::
~Shader() {
  release_all();
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::parse_init
//       Access: Public
//  Description: Set a 'parse pointer' to the beginning of the shader.
////////////////////////////////////////////////////////////////////
void Shader::
parse_init() {
  _parse = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::parse_line
//       Access: Public
//  Description: Parse a line of text. If 'lt' is true, trim blanks
//               from the left end of the line. If 'rt' is true, trim
//               blanks from the right end (the newline is always
//               trimmed).
////////////////////////////////////////////////////////////////////
void Shader::
parse_line(string &result, bool lt, bool rt) {
  int len = _text.size();
  int head = _parse;
  int tail = head;
  while ((tail < len) && (_text[tail] != '\n')) tail++;
  if (tail < len) {
    _parse = tail+1;
  } else {
    _parse = tail;
  }
  if (lt) {
    while ((head < tail)&&(isspace(_text[head]))) head++;
    while ((tail > head)&&(isspace(_text[tail-1]))) tail--;
  }
  result = _text.substr(head, tail-head);
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::parse_upto
//       Access: Public
//  Description: Parse lines until you read a line that matches the
//               specified pattern.  Returns all the preceding lines,
//               and if the include flag is set, returns the final
//               line as well.
////////////////////////////////////////////////////////////////////
void Shader::
parse_upto(string &result, string pattern, bool include) {
  GlobPattern endpat(pattern);
  int start = _parse;
  int last = _parse;
  while (_parse < _text.size()) {
    string t;
    parse_line(t, true, true);
    if (endpat.matches(t)) break;
    last = _parse;
  }
  if (include) {
    result = _text.substr(start, _parse - start);
  } else {
    result = _text.substr(start, last - start);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::parse_rest
//       Access: Public
//  Description: Returns the rest of the text from the current
//               parse location.
////////////////////////////////////////////////////////////////////
void Shader::
parse_rest(string &result) {
  result = _text.substr(_parse, _text.size() - _parse);
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::parse_eof
//       Access: Public
//  Description: Returns true if the parse pointer is at the end of
//               the shader.
////////////////////////////////////////////////////////////////////
bool Shader::
parse_eof() {
  return (int)_text.size() == _parse;
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::arg_index
//  Access: Public
//  Description: Allocates an integer index to the given
//               shader parameter name.
////////////////////////////////////////////////////////////////////
int Shader::
arg_index(const string &id) {
  for (int i=0; i<(int)(_args.size()); i++)
    if (_args[i] == id)
      return i;
  _args.push_back(id);
  return _args.size() - 1;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::prepare
//       Access: Published
//  Description: Indicates that the shader should be enqueued to be
//               prepared in the indicated prepared_objects at the
//               beginning of the next frame.  This will ensure the
//               texture is already loaded into texture memory if it
//               is expected to be rendered soon.
//
//               Use this function instead of prepare_now() to preload
//               textures from a user interface standpoint.
////////////////////////////////////////////////////////////////////
void Shader::
prepare(PreparedGraphicsObjects *prepared_objects) {
  prepared_objects->enqueue_shader(this);
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::release
//       Access: Published
//  Description: Frees the texture context only on the indicated object,
//               if it exists there.  Returns true if it was released,
//               false if it had not been prepared.
////////////////////////////////////////////////////////////////////
bool Shader::
release(PreparedGraphicsObjects *prepared_objects) {
  Contexts::iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    ShaderContext *sc = (*ci).second;
    if (sc != (ShaderContext *)NULL) {
      prepared_objects->release_shader(sc);
    } else {
      _contexts.erase(ci);
    }
    return true;
  }

  // Maybe it wasn't prepared yet, but it's about to be.
  return prepared_objects->dequeue_shader(this);
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::prepare_now
//       Access: Public
//  Description: Creates a context for the texture on the particular
//               GSG, if it does not already exist.  Returns the new
//               (or old) ShaderContext.  This assumes that the
//               GraphicsStateGuardian is the currently active
//               rendering context and that it is ready to accept new
//               textures.  If this is not necessarily the case, you
//               should use prepare() instead.
//
//               Normally, this is not called directly except by the
//               GraphicsStateGuardian; a texture does not need to be
//               explicitly prepared by the user before it may be
//               rendered.
////////////////////////////////////////////////////////////////////
ShaderContext *Shader::
prepare_now(PreparedGraphicsObjects *prepared_objects, 
            GraphicsStateGuardianBase *gsg) {
  Contexts::const_iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    return (*ci).second;
  }

  ShaderContext *tc = prepared_objects->prepare_shader_now(this, gsg);
  _contexts[prepared_objects] = tc;

  return tc;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::clear_prepared
//       Access: Private
//  Description: Removes the indicated PreparedGraphicsObjects table
//               from the Shader's table, without actually releasing
//               the texture.  This is intended to be called only from
//               PreparedGraphicsObjects::release_texture(); it should
//               never be called by user code.
////////////////////////////////////////////////////////////////////
void Shader::
clear_prepared(PreparedGraphicsObjects *prepared_objects) {
  Contexts::iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    _contexts.erase(ci);
  } else {
    // If this assertion fails, clear_prepared() was given a
    // prepared_objects which the texture didn't know about.
    nassertv(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::release_all
//       Access: Published
//  Description: Frees the context allocated on all objects for which
//               the texture has been declared.  Returns the number of
//               contexts which have been freed.
////////////////////////////////////////////////////////////////////
int Shader::
release_all() {
  // We have to traverse a copy of the _contexts list, because the
  // PreparedGraphicsObjects object will call clear_prepared() in response
  // to each release_texture(), and we don't want to be modifying the
  // _contexts list while we're traversing it.
  Contexts temp = _contexts;
  int num_freed = (int)_contexts.size();

  Contexts::const_iterator ci;
  for (ci = temp.begin(); ci != temp.end(); ++ci) {
    PreparedGraphicsObjects *prepared_objects = (*ci).first;
    ShaderContext *sc = (*ci).second;
    if (sc != (ShaderContext *)NULL) {
      prepared_objects->release_shader(sc);
    }
  }
  
  // There might still be some outstanding contexts in the map, if
  // there were any NULL pointers there.  Eliminate them.
  _contexts.clear();

  return num_freed;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::register_with_read_factory
//       Access: Public, Static
//  Description: Factory method to generate a Shader object
////////////////////////////////////////////////////////////////////
void Shader::
register_with_read_factory() {
  // IMPLEMENT ME
}
