/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaConversionClient.h
 * @author Derzsi Dániel
 * @date 2020-10-01
 */

#ifndef MAYACONVERSIONCLIENT_H
#define MAYACONVERSIONCLIENT_H

#include "queuedConnectionManager.h"
#include "queuedConnectionReader.h"
#include "connectionWriter.h"
#include "netAddress.h"
#include "filename.h"
#include "mayaConversionServer.h"

/**
 * The Maya conversion client sends batch conversion
 * requests to the Maya conversion server.
 *
 * These utilities rely on the Maya conversion server.
 * Use egg2maya or maya2egg to boot up the Maya conversion server.
 * Use egg2maya_client and maya2egg_client as a replacement for
 *
 * Very useful in case you need to batch convert models.
 * The regular utilities can only convert one model at a time.
*/
class MayaConversionClient {
public:
  MayaConversionClient();
  ~MayaConversionClient();

  bool connect(NetAddress server);
  bool queue(Filename working_directory, int argc, char *argv[], MayaConversionServer::ConversionType conversion_type);
  void close();

  int main(int argc, char *argv[], MayaConversionServer::ConversionType conversion_type);

private:
  QueuedConnectionManager _manager;
  QueuedConnectionReader _reader;
  ConnectionWriter _writer;
  PT(Connection) _conn;
};

#endif
