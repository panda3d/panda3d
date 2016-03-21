/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaToEgg_client.h
 * @author cbrunner
 * @date 2009-11-09
 */

#ifndef MAYATOEGGCLIENT_H
#define MAYATOEGGCLIENT_H

#include "somethingToEgg.h"
#include "queuedConnectionManager.h"
#include "queuedConnectionReader.h"
#include "connectionWriter.h"

/**
 *
 */
class MayaToEggClient : public SomethingToEgg {
public:
  MayaToEggClient();

  QueuedConnectionManager *qManager;
  QueuedConnectionReader *qReader;
  ConnectionWriter *cWriter;
  NetAddress server;
};

#endif
