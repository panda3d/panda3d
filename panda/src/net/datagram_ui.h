/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagram_ui.h
 * @author drose
 * @date 2000-02-09
 */

#ifndef DATAGRAM_UI_H
#define DATAGRAM_UI_H

/*
 * The functions defined here are used for testing purposes only by some of
 * the various test_* programs in this directory.  They are not compiled into
 * the package library, libnet.so.  These functions are handy for getting and
 * reporting a datagram from and to the user.  They extend a datagram by
 * encoding information about the types of values stored in it.
 */

#include "pandabase.h"

#include "netDatagram.h"

std::istream &operator >> (std::istream &in, NetDatagram &datagram);
std::ostream &operator << (std::ostream &out, const NetDatagram &datagram);

#endif
