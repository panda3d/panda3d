/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggToMayaClient.cxx
 * @author Derzsi Dániel
 * @date 2020-10-01
 */

#include "mayaConversionClient.h"

/**
 * Entrypoint for egg2maya_client.
 */
int main(int argc, char *argv[]) {
    MayaConversionClient client;
    return client.main(argc, argv, MayaConversionServer::ConversionType::CT_egg_to_maya);
}
