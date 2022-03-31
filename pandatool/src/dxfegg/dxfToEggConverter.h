/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxfToEggConverter.h
 * @author drose
 * @date 2004-05-04
 */

#ifndef DXFTOEGGCONVERTER_H
#define DXFTOEGGCONVERTER_H

#include "pandatoolbase.h"

#include "somethingToEggConverter.h"
#include "dxfFile.h"

/**
 * This class supervises the construction of an EggData structure from a DXF
 * file.
 */
class DXFToEggConverter : public SomethingToEggConverter, public DXFFile {
public:
  DXFToEggConverter();
  DXFToEggConverter(const DXFToEggConverter &copy);
  ~DXFToEggConverter();

  virtual SomethingToEggConverter *make_copy();

  virtual std::string get_name() const;
  virtual std::string get_extension() const;
  virtual bool supports_compressed() const;

  virtual bool convert_file(const Filename &filename);

protected:
  virtual DXFLayer *new_layer(const std::string &name);
  virtual void done_entity();
  virtual void error();

  bool _error;
};

#endif
