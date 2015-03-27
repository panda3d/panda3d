// Filename: p3dCert.h
// Created by:  rdb (08Mar11)
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

#ifndef P3DCERT_H
#define P3DCERT_H

#include <FL/Fl.H>
#include <FL/Fl_Window.H>

#define OPENSSL_NO_KRB5
#include "openssl/x509.h"
#include "openssl/x509_vfy.h"
#include "openssl/pem.h"

#include <string>
#include <iostream>
#include <stdio.h>
using namespace std;

class ViewCertDialog;

#ifndef STACK_OF
  // At some point, presumably in 1.0.0, openssl went to the
  // STACK_OF() macro system to typedef the contents of a stack.
  // Unfortunately, that new API is different.  We define some macros
  // here here for backward compatiblity.
  #define STACK_OF(type) STACK
  #define sk_X509_push(stack, item) sk_push((stack), (char *)(item))
  #define sk_X509_free(stack) sk_free(stack)
  #define sk_X509_new(cmp) sk_new(cmp)
#endif

////////////////////////////////////////////////////////////////////
//       Class : AuthDialog
// Description : This is the primary dialog of this application.
//
//               This dialog is presented to the user when he/she
//               clicks on the red authorization button on the splash
//               window.  It tells the user the status of the
//               application's signature, and invites the user to
//               approve the signature or cancel.
////////////////////////////////////////////////////////////////////
class AuthDialog : public Fl_Window {
public:
#ifdef _WIN32
  AuthDialog(const wstring &cert_filename, const wstring &cert_dir);
#else
  AuthDialog(const string &cert_filename, const string &cert_dir);
#endif
  virtual ~AuthDialog();

  static void run_clicked(Fl_Widget *w, void *data);
  static void view_cert_clicked(Fl_Widget *w, void *data);
  static void cancel_clicked(Fl_Widget *w, void *data);

  void approve_cert();

private:
#ifdef _WIN32
  void read_cert_file(const wstring &cert_filename);
#else
  void read_cert_file(const string &cert_filename);
#endif
  void get_friendly_name();
  void verify_cert();
  int load_certificates_from_der_ram(X509_STORE *store,
                                     const char *data, size_t data_size);

  void layout();
  void get_text(char *header, size_t hlen, char *text, size_t tlen);

public:
  ViewCertDialog *_view_cert_dialog;

private:
#ifdef _WIN32
  wstring _cert_dir;
#else
  string _cert_dir;
#endif
  X509 *_cert;
  STACK_OF(X509) *_stack;

  char _header[32];
  char _text[512];
  char _text_clean[1024];

  string _friendly_name;
  int _verify_result;
};

////////////////////////////////////////////////////////////////////
//       Class : ViewCertDialog
// Description : This is the detailed view of the particular
//               certificate.
////////////////////////////////////////////////////////////////////
class ViewCertDialog : public Fl_Window {
public:
  ViewCertDialog(AuthDialog *auth_dialog, X509 *cert);
  virtual ~ViewCertDialog();

  static void run_clicked(Fl_Widget *w, void *data);
  static void cancel_clicked(Fl_Widget *w, void *data);

private:
  void layout();

private:
  AuthDialog *_auth_dialog;
  X509 *_cert;
};

#endif
