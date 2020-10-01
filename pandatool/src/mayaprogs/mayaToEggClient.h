/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaToEggClient.h
 * @author Derzsi Dániel
 * @date 2020-10-01
 */

#ifndef MAYATOEGGCLIENT_H
#define MAYATOEGGCLIENT_H

#include "somethingToEgg.h"
#include "mayaConversionClient.h"

/**
 * The MayaToEggClient class is responsible for implementing
 * the maya2egg_client utility.
 *
 * The maya2egg_client utility is designed to be used in tandem
 * with maya2egg's -server flag.
 *
 * Using the maya2egg server, users are able to batch convert
 * Maya ASCII and binary files to Panda egg files.
 */
class MayaToEggClient : public SomethingToEgg, public MayaConversionClient {
public:
    MayaToEggClient();
};

#endif
