// Filename: pipeSpec.h
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

#ifndef __PIPESPEC_H__
#define __PIPESPEC_H__

#include <pandabase.h>

#include <string>

class EXPCL_PANDA PipeSpecifier {
PUBLISHED:
  PipeSpecifier(void);
  PipeSpecifier(const PipeSpecifier&);
  ~PipeSpecifier(void);

  INLINE void set_name(const std::string&);
  INLINE std::string get_name(void) const;
  INLINE void set_machine_name(const std::string&);
  INLINE std::string get_machine_name(void) const;
  INLINE void set_file_name(const std::string&);
  INLINE std::string get_file_name(void) const;
  INLINE void set_pipe_number(const int);
  INLINE int get_pipe_number(void) const;

  INLINE bool is_file(void) const;
  INLINE bool is_remote(void) const;

  std::string get_X_specifier(void) const;

private:
  std::string _name;
  std::string _machine;
  std::string _filename;
  int         _pipe_number;
  bool        _is_file;
  bool        _is_remote;

  INLINE void unset_machine(void);
  INLINE void unset_file(void);
};

#include "pipeSpec.I"

#endif /* __PIPESPEC_H__ */
