// Filename: pipeSpec.cxx
// Created by:  frang (07Mar99)
// 
////////////////////////////////////////////////////////////////////

#include "pipeSpec.h"
#include "config_display.h"

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
