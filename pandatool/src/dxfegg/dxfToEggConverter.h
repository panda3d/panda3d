// Filename: DXFToEggConverter.h
// Created by:  drose (04May04)
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

#ifndef DXFTOEGGCONVERTER_H
#define DXFTOEGGCONVERTER_H

#include "pandatoolbase.h"

#include "somethingToEggConverter.h"
#include "dxfFile.h"

////////////////////////////////////////////////////////////////////
//       Class : DXFToEggConverter
// Description : This class supervises the construction of an EggData
//               structure from a DXF file.
////////////////////////////////////////////////////////////////////
class DXFToEggConverter : public SomethingToEggConverter, public DXFFile {
public:
  DXFToEggConverter();
  DXFToEggConverter(const DXFToEggConverter &copy);
  ~DXFToEggConverter();

  virtual SomethingToEggConverter *make_copy();

  virtual string get_name() const;
  virtual string get_extension() const;

  virtual bool convert_file(const Filename &filename);

protected:
  virtual DXFLayer *new_layer(const string &name);
  virtual void done_entity();
  virtual void error();

  bool _error;
};

#endif


