// Filename: ppInstance.h
// Created by:  drose (19Jun09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef PPINSTANCE_H
#define PPINSTANCE_H

#include "nppanda3d_common.h"
#include "pluginbase.h"

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : PPInstance
// Description : This represents a single instance of the Panda3D
//               plugin, via the NPAPI interface.  This instance
//               brokers the communication with the P3D Core API, as
//               defined in the plugin directory.
////////////////////////////////////////////////////////////////////
class PPInstance : public nsPluginInstanceBase {
public:
  PPInstance(nsPluginCreateData *create_data);
  ~PPInstance();

  // Methods inherited from base class

  virtual NPBool init(NPWindow *aWindow);
  virtual void shut();
  virtual NPBool isInitialized();

  /*
  virtual NPError SetWindow(NPWindow *pNPWindow);
  virtual NPError NewStream(NPMIMEType type, NPStream *stream, 
                            NPBool seekable, uint16 *stype);
  virtual NPError DestroyStream(NPStream *stream, NPError reason);
  virtual void    StreamAsFile(NPStream *stream, const char *fname);
  virtual int32   WriteReady(NPStream *stream);
  virtual int32   Write(NPStream *stream, int32 offset, 
                        int32 len, void *buffer);
  virtual void    Print(NPPrint *printInfo);
  virtual uint16  HandleEvent(void *event);
  virtual void    URLNotify(const char *url, NPReason reason, 
                            void *notifyData);
  virtual NPError GetValue(NPPVariable variable, void *value);
  virtual NPError SetValue(NPNVariable variable, void *value);
  */

private:
  typedef vector<P3D_token> Tokens;
  Tokens _tokens;
  int _npp_mode;

  P3D_instance *_inst;
};

#include "ppInstance.I"

#endif
