/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaConversionServer.h
 * @author Derzsi Dániel
 * @date 2020-10-01
 */

#ifndef MAYACONVERSIONSERVER_H
#define MAYACONVERSIONSERVER_H

#include "queuedConnectionManager.h"
#include "queuedConnectionListener.h"
#include "queuedConnectionReader.h"
#include "connectionWriter.h"

/**
 * The Maya conversion server listens for incoming requests to
 * handle maya-to-egg and egg-to-maya model conversions.
 *
 * This server listens to port 4242 after being started with
 * egg2maya -server or maya2egg -server.
 *
 * Use egg2maya_client and maya2egg_client as a replacement for
 * the egg2maya and maya2egg utilities after starting the server.
 *
 * Very useful in case you need to batch convert models.
 * The regular utilities can only convert one model at a time.
*/
class MayaConversionServer {
public:
  enum ConversionType {
    CT_maya_to_egg = 0,
    CT_egg_to_maya = 1
  };

  MayaConversionServer();

  void listen();
  void poll();

protected:
  typedef pset< PT(Connection) > Clients;
  Clients _clients;

  QueuedConnectionManager _qManager;
  QueuedConnectionListener _qListener;
  QueuedConnectionReader _qReader;
  ConnectionWriter _cWriter;
};

#endif
