/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcmsgtypes.h
 * @author drose
 * @date 2004-05-18
 */

#ifndef DCMSGTYPES_H
#define DCMSGTYPES_H

// This file defines the server message types used within this module.  It
// duplicates some symbols defined in MsgTypes.py and AIMsgTypes.py.

#define CLIENT_OBJECT_UPDATE_FIELD                        24
#define CLIENT_CREATE_OBJECT_REQUIRED                     34
#define CLIENT_CREATE_OBJECT_REQUIRED_OTHER               35

#define STATESERVER_OBJECT_GENERATE_WITH_REQUIRED         2001
#define STATESERVER_OBJECT_GENERATE_WITH_REQUIRED_OTHER   2003
#define STATESERVER_OBJECT_UPDATE_FIELD                   2004
#define STATESERVER_OBJECT_CREATE_WITH_REQUIRED_CONTEXT   2050
#define STATESERVER_OBJECT_CREATE_WITH_REQUIR_OTHER_CONTEXT  2051
#define STATESERVER_BOUNCE_MESSAGE                        2086

#define CLIENT_OBJECT_GENERATE_CMU                        9002

#endif
