// Filename: datagramGenerator.h
// Created by:  jason (07Jun00)
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

#ifndef DATAGRAMGENERATOR_H
#define DATAGRAMGENERATOR_H

#include "pandabase.h"

#include "datagram.h"

////////////////////////////////////////////////////////////////////
//       Class : DatagramGenerator
// Description : This class defines the abstract interace to any
//               source of datagrams, whether it be from a file or
//               from the net
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS DatagramGenerator {
public:
  INLINE DatagramGenerator();
  virtual ~DatagramGenerator();

  virtual bool get_datagram(Datagram &data) = 0;
  virtual bool is_eof() = 0;
  virtual bool is_error() = 0;
};

#include "datagramGenerator.I"

#endif
