// Filename: datagram_ui.h
// Created by:  drose (09Feb00)
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

#ifndef DATAGRAM_UI_H
#define DATAGRAM_UI_H

////////////////////////////////////////////////////////////////////
//
// The functions defined here are used for testing purposes only by
// some of the various test_* programs in this directory.  They are
// not compiled into the package library, libnet.so.
//
// These functions are handy for getting and reporting a datagram from
// and to the user.  They extend a datagram by encoding information
// about the types of values stored in it.
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"

#include "netDatagram.h"

istream &operator >> (istream &in, NetDatagram &datagram);
ostream &operator << (ostream &out, const NetDatagram &datagram);

#endif
