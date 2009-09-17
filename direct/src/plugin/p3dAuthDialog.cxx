// Filename: p3dAuthDialog.cxx
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

#include "p3dAuthDialog.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DAuthDialog::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DAuthDialog::
P3DAuthDialog() {
  _cert = NULL;
  _verify_status = VS_no_cert;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DAuthDialog::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DAuthDialog::
~P3DAuthDialog() {
  close();
  clear_cert();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DAuthDialog::read_cert
//       Access: Public
//  Description: Reads the indicated certificate chain and stores the
//               first certificate in _x509 and its common name in
//               _common_name; also checks whether the certificate is
//               valid by the chain and initializes _verify_status
//               accordingly.
////////////////////////////////////////////////////////////////////
void P3DAuthDialog::
read_cert(const P3DMultifileReader::CertChain &cert_chain) {
  clear_cert();
  if (cert_chain.empty()) {
    _verify_status = VS_no_cert;
    return;
  }

  _cert = X509_dup(cert_chain[0]._cert);

  // A complex OpenSSL interface to extract out the common name in
  // utf-8.
  X509_NAME *xname = X509_get_subject_name(cert_chain[0]._cert);
  if (xname != NULL) {
    int pos = X509_NAME_get_index_by_NID(xname, NID_commonName, -1);
    if (pos != -1) {
      // We just get the first common name.  I guess it's possible to
      // have more than one; not sure what that means in this context.
      X509_NAME_ENTRY *xentry = X509_NAME_get_entry(xname, pos);
      if (xentry != NULL) {
        ASN1_STRING *data = X509_NAME_ENTRY_get_data(xentry);
        if (data != NULL) {
          // We use "print" to dump the output to a memory BIO.  Is
          // there an easier way to decode the ASN1_STRING?  Curse
          // these incomplete docs.
          BIO *mbio = BIO_new(BIO_s_mem());
          ASN1_STRING_print_ex(mbio, data, ASN1_STRFLGS_RFC2253 & ~ASN1_STRFLGS_ESC_MSB);

          char *pp;
          long pp_size = BIO_get_mem_data(mbio, &pp);
          string name(pp, pp_size);
          BIO_free(mbio);
          _common_name = name;
        }
      }
    }
  }

  // Now validate the signature.

  // Create a STACK of everything following the first cert.
  STACK *stack = NULL;
  if (cert_chain.size() > 1) {
    stack = sk_new(NULL);
    for (size_t n = 1; n < cert_chain.size(); ++n) {
      sk_push(stack, (char *)cert_chain[n]._cert);
    }
  }

  // Create a new X509_STORE.
  X509_STORE *store = X509_STORE_new();
  X509_STORE_set_default_paths(store);

  // Create the X509_STORE_CTX for verifying the cert and chain.
  X509_STORE_CTX *ctx = X509_STORE_CTX_new();
  X509_STORE_CTX_init(ctx, store, _cert, stack);
  X509_STORE_CTX_set_cert(ctx, _cert);

  if (X509_verify_cert(ctx)) {
    _verify_status = VS_verified;
  } else {
    int verify_result = X509_STORE_CTX_get_error(ctx);
    switch (verify_result) {
    case X509_V_ERR_CERT_NOT_YET_VALID:
    case X509_V_ERR_CERT_HAS_EXPIRED:
    case X509_V_ERR_CRL_NOT_YET_VALID:
    case X509_V_ERR_CRL_HAS_EXPIRED:
      _verify_status = VS_expired;
      break;
      
    case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
    case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
      _verify_status = VS_self_signed;
      break;

    default:
      _verify_status = VS_other;
    }
  }

  sk_free(stack);
  X509_STORE_CTX_cleanup(ctx);
  X509_STORE_CTX_free(ctx);

  X509_STORE_free(store);

  nout << "Got certificate from " << _common_name
       << ", verify_status = " << _verify_status << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: P3DAuthDialog::clear_cert
//       Access: Public
//  Description: Clears the data allocated by read_cert().
////////////////////////////////////////////////////////////////////
void P3DAuthDialog::
clear_cert() {
  if (_cert != NULL){ 
    X509_free(_cert);
    _cert = NULL;
  }
  
  _common_name.clear();
  _verify_status = VS_no_cert;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DAuthDialog::open
//       Access: Public, Virtual
//  Description: Displays the dialog and waits for user to click a
//               button.
////////////////////////////////////////////////////////////////////
void P3DAuthDialog::
open() {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DAuthDialog::close
//       Access: Public, Virtual
//  Description: Closes the dialog prematurely.
////////////////////////////////////////////////////////////////////
void P3DAuthDialog::
close() {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DAuthDialog::get_text
//       Access: Protected
//  Description: Fills in the text appropriate to display in the
//               dialog box, based on the certificate read so far.
////////////////////////////////////////////////////////////////////
void P3DAuthDialog::
get_text(string &header, string &text) {
  switch (_verify_status) {
  case VS_verified:
    text = "This Panda3D application has been signed by ";
    text += _common_name;
    text += ".\n\nIf you trust ";
    text += _common_name;
    text += ", then click the Run button below\n";
    text += "to run this application on your computer.  This will also\n";
    text += "automatically approve this and any other applications signed by\n";
    text += _common_name;
    text += " in the future.\n\n";
    text += "If you are unsure about this application,\n";
    text += "you should click Cancel instead.";
    return;

  case VS_self_signed:
    header = "Unverified signature!\n";
    text = "This Panda3D application has been signed by what's known as a\n";
    text += "self-signed certificate.  This means the name on the certificate can't\n";
    text += "be verified, and you have no way of knowing for sure who wrote it.\n\n";
    text += "We recommend you click Cancel to avoid running this application.";
    return;

  case VS_no_cert:
    header = "No signature!\n";
    text = "This Panda3D application has not been signed.";
    return;

  default:
    text = "Undefined text.";
  }
}
