// Filename: p3dAuthDialog.h
// Created by:  drose (16Sep09)
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

#ifndef P3DAUTHDIALOG_H
#define P3DAUTHDIALOG_H

#include "p3d_plugin_common.h"
#include "p3dMultifileReader.h"
#include "p3dInstanceManager.h"

////////////////////////////////////////////////////////////////////
//       Class : P3DAuthDialog
// Description : This dialog is presented to the user when he/she
//               clicks on the red authorization button on the splash
//               window.  It tells the user the status of the
//               application's signature, and invites the user to
//               approve the signature or cancel.
//
//               This is the base implementation; it contains no
//               specific code to open a window.
////////////////////////////////////////////////////////////////////
class P3DAuthDialog {
public:
  P3DAuthDialog();
  virtual ~P3DAuthDialog();

  void read_cert(const P3DMultifileReader::CertChain &cert_chain);
  void clear_cert();

  virtual void open();
  virtual void close();

protected:
  void get_text(string &header, string &text);

protected:
  X509 *_cert;
  string _common_name;

  enum VerifyStatus {
    VS_verified,
    VS_self_signed,
    VS_expired,
    VS_no_cert,
    VS_other,
  };
  VerifyStatus _verify_status;
};

#include "p3dAuthDialog.I"

#endif
