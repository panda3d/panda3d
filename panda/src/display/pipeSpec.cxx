// Filename: pipeSpec.cxx
// Created by:  frang (07Mar99)
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


#include "config_display.h"

#include "pipeSpec.h"

PipeSpecifier::PipeSpecifier(void)
  : _machine(pipe_spec_machine),
    _filename(pipe_spec_filename),
    _pipe_number(pipe_spec_pipe_number),
    _is_file(pipe_spec_is_file),
    _is_remote(pipe_spec_is_remote) {}

PipeSpecifier::PipeSpecifier(const PipeSpecifier& c)
  : _name(c._name), _machine(c._machine), _filename(c._filename),
    _pipe_number(c._pipe_number), _is_file(c._is_file),
    _is_remote(c._is_remote) {}

PipeSpecifier::~PipeSpecifier(void) {}

std::string PipeSpecifier::get_X_specifier(void) const {
  std::string ret;

  if (!_is_file) {
    if (getenv("DISPLAY")) {
       ret = getenv("DISPLAY");
    } else {
      ostringstream ss;
      ss << _machine << ":" << ((_pipe_number<0)?0:_pipe_number) << ".0";
      ret = ss.str();
    }
  }
  return ret;
}
