// Filename: datagram_ui.h
// Created by:  drose (09Feb00)
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

#include <pandabase.h>

#include "netDatagram.h"

istream &operator >> (istream &in, NetDatagram &datagram);
ostream &operator << (ostream &out, const NetDatagram &datagram);

#endif
