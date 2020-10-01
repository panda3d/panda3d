/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggToMayaClient.h
 * @author Derzsi Dániel
 * @date 2020-10-01
 */

#ifndef EGGTOMAYACLIENT_H
#define EGGTOMAYACLIENT_H

#include "eggToSomething.h"
#include "mayaConversionClient.h"


/**
 * The EggToMayaClient class is responsible for implementing
 * the egg2maya_client utility.
 *
 * The egg2maya_client utility is designed to be used in tandem
 * with egg2maya's -server flag.
 *
 * Using the egg2maya server, users are able to batch convert
 * Panda egg files to Maya ASCII and binary files.
 */
class EggToMayaClient : public EggToSomething, public MayaConversionClient {
public:
    EggToMayaClient();
};

#endif
