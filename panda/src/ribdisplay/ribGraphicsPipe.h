// Filename: ribGraphicsPipe.h
// Created by:  drose (15Feb99)
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

#ifndef RIBGRAPHICSPIPE_H
#define RIBGRAPHICSPIPE_H

#include "pandabase.h"

#include <string>
#include <noninteractiveGraphicsPipe.h>


////////////////////////////////////////////////////////////////////
//       Class : RIBGraphicsPipe
// Description : A place to grab RIBGraphicsWindows from.  Not
//               terribly exciting in itself.  The name of the pipe is
//               used as the default RIB output filename for each
//               window, but the window may change this.
////////////////////////////////////////////////////////////////////
class RIBGraphicsPipe : public NoninteractiveGraphicsPipe {
public:

  INLINE RIBGraphicsPipe(const PipeSpecifier&);

  virtual TypeHandle get_window_type() const;

public:

  static GraphicsPipe *make_RIBGraphicsPipe(const FactoryParams &params);

  static TypeHandle get_class_type(void);
  static void init_type(void);
  virtual TypeHandle get_type(void) const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

public:

  INLINE std::string get_file_name(void) const;

protected:

  std::string _filename;

private:

  static TypeHandle _type_handle;

protected:

  INLINE RIBGraphicsPipe(void);
  INLINE RIBGraphicsPipe(const RIBGraphicsPipe&);
  INLINE RIBGraphicsPipe& operator=(const RIBGraphicsPipe&);
};

#include "ribGraphicsPipe.I"

#endif

