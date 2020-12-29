/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaToEggClient.cxx
 * @author Derzsi Dániel
 * @date 2020-10-01
 */

#include "mayaConversionClient.h"

/**
 * Entrypoint for maya2egg_client.
 */
int main(int argc, char *argv[]) {
    MayaConversionClient client;
    return client.main(argc, argv, MayaConversionServer::ConversionType::CT_maya_to_egg);
}
