// Filename: dcmsgtypes.h
// Created by:  drose (18May04)
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

#ifndef DCMSGTYPES_H
#define DCMSGTYPES_H

// This file defines the server message types used within this module.
// It duplicates some symbols defined in MsgTypes.py and
// AIMsgTypes.py.

#define CLIENT_OBJECT_UPDATE_FIELD                        24
#define CLIENT_CREATE_OBJECT_REQUIRED                     34
#define CLIENT_CREATE_OBJECT_REQUIRED_OTHER               35

#define STATESERVER_OBJECT_GENERATE_WITH_REQUIRED         2001
#define STATESERVER_OBJECT_GENERATE_WITH_REQUIRED_OTHER   2003
#define STATESERVER_OBJECT_UPDATE_FIELD                   2004


#endif

