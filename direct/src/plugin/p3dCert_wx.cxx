/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dCert_wx.cxx
 * @author drose
 * @date 2009-09-11
 */

#include "p3dCert_wx.h"
#include "wstring_encode.h"
#include "mkdir_complete.h"

#include "wx/cmdline.h"
#include "wx/filename.h"

#include "ca_bundle_data_src.c"

using std::cerr;

static const wxString
self_signed_cert_text =
  _T("This Panda3D application uses a self-signed certificate.  ")
  _T("This means the author's name can't be verified, and you have ")
  _T("no way of knowing for sure who wrote it.\n\n")

  _T("We recommend you click Cancel to avoid running this application.");

static const wxString
unknown_auth_cert_text =
  _T("This Panda3D application has been signed, but we don't recognize ")
  _T("the authority that verifies the signature.  This means the author's ")
  _T("name can't be trusted, and you have no way of knowing ")
  _T("for sure who wrote it.\n\n")

  _T("We recommend you click Cancel to avoid running this application.");

static const wxString
verified_cert_text =
  _T("This Panda3D application has been signed by %s. ")
  _T("If you trust %s, then click the Run button below ")
  _T("to run this application on your computer.  This will also ")
  _T("automatically approve this and any other applications signed by ")
  _T("%s in the future.\n\n")

  _T("If you are unsure about this application, ")
  _T("you should click Cancel instead.");

static const wxString
expired_cert_text =
  _T("This Panda3D application has been signed by %s, ")
  _T("but the certificate has expired.\n\n")

  _T("You should check the current date set on your computer's clock ")
  _T("to make sure it is correct.\n\n")

  _T("If your computer's date is correct, we recommend ")
  _T("you click Cancel to avoid running this application.");

static const wxString
generic_error_cert_text =
  _T("This Panda3D application has been signed, but there is a problem ")
  _T("with the certificate (OpenSSL error code %d).\n\n")

  _T("We recommend you click Cancel to avoid running this application.");

static const wxString
no_cert_text =
  _T("This Panda3D application has not been signed.  This means you have ")
  _T("no way of knowing for sure who wrote it.\n\n")

  _T("Click Cancel to avoid running this application.");

// wxWidgets boilerplate macro to define main() and start up the application.
IMPLEMENT_APP(P3DCertApp)

/**
 * The "main" of a wx application.  This is the first entry point.
 */
bool P3DCertApp::
OnInit() {
  // call the base class initialization method, currently it only parses a few
  // common command-line options but it could be do more in the future
  if (!wxApp::OnInit()) {
    return false;
  }

  OpenSSL_add_all_algorithms();

  AuthDialog *dialog = new AuthDialog(_cert_filename, _cert_dir);
  SetTopWindow(dialog);
  dialog->Show(true);
  dialog->SetFocus();
  dialog->Raise();

  // Return true to enter the main loop and wait for user input.
  return true;
}

/**
 * A callback to initialize the parser with the command line options.
 */
void P3DCertApp::
OnInitCmdLine(wxCmdLineParser &parser) {
  parser.AddParam();
  parser.AddParam();
}

/**
 * A callback after the successful parsing of the command line.
 */
bool P3DCertApp::
OnCmdLineParsed(wxCmdLineParser &parser) {
  _cert_filename = (const char *)parser.GetParam(0).mb_str();
  _cert_dir = (const char *)parser.GetParam(1).mb_str();

  return true;
}


// The event table for AuthDialog.
#define VIEW_CERT_BUTTON  (wxID_HIGHEST + 1)
BEGIN_EVENT_TABLE(AuthDialog, wxDialog)
    EVT_BUTTON(wxID_OK, AuthDialog::run_clicked)
    EVT_BUTTON(VIEW_CERT_BUTTON, AuthDialog::view_cert_clicked)
    EVT_BUTTON(wxID_CANCEL, AuthDialog::cancel_clicked)
END_EVENT_TABLE()

/**
 *
 */
AuthDialog::
AuthDialog(const std::string &cert_filename, const std::string &cert_dir) :
  // I hate stay-on-top dialogs, but if we don't set this flag, it doesn't
  // come to the foreground on OSX, and might be lost behind the browser
  // window.
  wxDialog(nullptr, wxID_ANY, _T("New Panda3D Application"), wxDefaultPosition,
           wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP),
  _cert_dir(cert_dir)
{
  _view_cert_dialog = nullptr;

  _cert = nullptr;
  _stack = nullptr;
  _verify_result = -1;

  read_cert_file(cert_filename);
  get_friendly_name();
  verify_cert();
  layout();
}

/**
 *
 */
AuthDialog::
~AuthDialog() {
  if (_view_cert_dialog != nullptr) {
    _view_cert_dialog->Destroy();
  }

  if (_cert != nullptr) {
    X509_free(_cert);
    _cert = nullptr;
  }
  if (_stack != nullptr) {
    sk_X509_free(_stack);
    _stack = nullptr;
  }
}

/**
 * The user clicks the "Run" button.
 */
void AuthDialog::
run_clicked(wxCommandEvent &event) {
  approve_cert();
}

/**
 * The user clicks the "View Certificate" button.
 */
void AuthDialog::
view_cert_clicked(wxCommandEvent &event) {
  if (_view_cert_dialog != nullptr) {
    _view_cert_dialog->Destroy();
  }
  Hide();
  _view_cert_dialog = new ViewCertDialog(this, _cert);
  _view_cert_dialog->Show();
}

/**
 * The user clicks the "Cancel" button.
 */
void AuthDialog::
cancel_clicked(wxCommandEvent &event) {
  Destroy();
}

/**
 * Writes the certificate into the _cert_dir, so that it will be found by the
 * P3DInstanceManager and known to be approved.
 */
void AuthDialog::
approve_cert() {
  assert(_cert != nullptr);

  // Make sure the directory exists.
  mkdir_complete(_cert_dir, cerr);

  // Look for an unused filename.
  int i = 1;
  size_t buf_length = _cert_dir.length() + 100;
  char *buf = new char[buf_length];
#ifdef _WIN32
  std::wstring buf_w;
#endif // _WIN32

  while (true) {
    sprintf(buf, "%s/p%d.crt", _cert_dir.c_str(), i);
    assert(strlen(buf) < buf_length);

    // Check if it already exists.  If not, take it.
#ifdef _WIN32
    DWORD results = 0;
    if (string_to_wstring(buf_w, buf)) {
      results = GetFileAttributesW(buf_w.c_str());
    }
    if (results == -1) {
      break;
    }
#else
    struct stat statbuf;
    if (stat(buf, &statbuf) != 0) {
      break;
    }
#endif
    ++i;
  }

  // Sure, there's a slight race condition right now: another process might
  // attempt to create the same filename.  So what.
  FILE *fp = nullptr;
#ifdef _WIN32
  fp = _wfopen(buf_w.c_str(), L"w");
#else // _WIN32
  fp = fopen(buf, "w");
#endif  // _WIN32
  if (fp != nullptr) {
    PEM_write_X509(fp, _cert);
    fclose(fp);
  }

  Destroy();
}

/**
 * Reads the list of certificates in the pem filename passed on the command
 * line into _cert and _stack.
 */
void AuthDialog::
read_cert_file(const std::string &cert_filename) {
  FILE *fp = nullptr;
#ifdef _WIN32
  std::wstring cert_filename_w;
  if (string_to_wstring(cert_filename_w, cert_filename)) {
    fp = _wfopen(cert_filename_w.c_str(), L"r");
  }
#else // _WIN32
  fp = fopen(cert_filename.c_str(), "r");
#endif  // _WIN32

  if (fp == nullptr) {
    cerr << "Couldn't read " << cert_filename << "\n";
    return;
  }
  _cert = PEM_read_X509(fp, nullptr, nullptr, (void *)"");
  if (_cert == nullptr) {
    cerr << "Could not read certificate in " << cert_filename << ".\n";
    fclose(fp);
    return;
  }

  // Build up a STACK of the remaining certificates in the file.
  _stack = sk_X509_new(nullptr);
  X509 *c = PEM_read_X509(fp, nullptr, nullptr, (void *)"");
  while (c != nullptr) {
    sk_X509_push(_stack, c);
    c = PEM_read_X509(fp, nullptr, nullptr, (void *)"");
  }

  fclose(fp);
}

/**
 * Extracts the "friendly name" from the certificate: the common name or email
 * name.
 */
void AuthDialog::
get_friendly_name() {
  if (_cert == nullptr) {
    _friendly_name.clear();
    return;
  }


  static const int nid_choices[] = {
    NID_pkcs9_emailAddress,
    NID_commonName,
    -1,
  };

  // Choose the first NID that exists on the cert.
  for (int ni = 0; nid_choices[ni] != -1; ++ni) {
    int nid = nid_choices[ni];

    // A complex OpenSSL interface to extract out the name in utf-8.
    X509_NAME *xname = X509_get_subject_name(_cert);
    if (xname != nullptr) {
      int pos = X509_NAME_get_index_by_NID(xname, nid, -1);
      if (pos != -1) {
        // We just get the first common name.  I guess it's possible to have
        // more than one; not sure what that means in this context.
        X509_NAME_ENTRY *xentry = X509_NAME_get_entry(xname, pos);
        if (xentry != nullptr) {
          ASN1_STRING *data = X509_NAME_ENTRY_get_data(xentry);
          if (data != nullptr) {
            // We use "print" to dump the output to a memory BIO.  Is there an
            // easier way to decode the ASN1_STRING?  Curse these incomplete
            // docs.
            BIO *mbio = BIO_new(BIO_s_mem());
            ASN1_STRING_print_ex(mbio, data, ASN1_STRFLGS_RFC2253 & ~ASN1_STRFLGS_ESC_MSB);

            char *pp;
            long pp_size = BIO_get_mem_data(mbio, &pp);
            _friendly_name = wxString(pp, wxConvUTF8, pp_size);
            BIO_free(mbio);
            return;
          }
        }
      }
    }
  }
}

/**
 * Checks whether the certificate is valid by the chain and initializes
 * _verify_status accordingly.
 */
void AuthDialog::
verify_cert() {
  if (_cert == nullptr) {
    _verify_result = -1;
    return;
  }

  // Create a new X509_STORE.
  X509_STORE *store = X509_STORE_new();
  X509_STORE_set_default_paths(store);

  // Add in the well-known certificate authorities.
  load_certificates_from_der_ram(store, (const char *)ca_bundle_data, ca_bundle_data_len);

  // Create the X509_STORE_CTX for verifying the cert and chain.
  X509_STORE_CTX *ctx = X509_STORE_CTX_new();
  X509_STORE_CTX_init(ctx, store, _cert, _stack);
  X509_STORE_CTX_set_cert(ctx, _cert);

  if (X509_verify_cert(ctx)) {
    _verify_result = 0;
  } else {
    _verify_result = X509_STORE_CTX_get_error(ctx);
  }

  X509_STORE_CTX_free(ctx);

  X509_STORE_free(store);

  cerr << "Got certificate from " << _friendly_name.mb_str()
       << ", verify_result = " << _verify_result << "\n";
}

/**
 * Reads a chain of trusted certificates from the indicated data buffer and
 * adds them to the X509_STORE object.  The data buffer should be DER-
 * formatted.  Returns the number of certificates read on success, or 0 on
 * failure.
 *
 * You should call this only with trusted, locally-stored certificates; not
 * with certificates received from an untrusted source.
 */
int AuthDialog::
load_certificates_from_der_ram(X509_STORE *store,
                               const char *data, size_t data_size) {
  int count = 0;

#if OPENSSL_VERSION_NUMBER >= 0x00908000L
  // Beginning in 0.9.8, d2i_X509() accepted a const unsigned char **.
  const unsigned char *bp, *bp_end;
#else
  // Prior to 0.9.8, d2i_X509() accepted an unsigned char **.
  unsigned char *bp, *bp_end;
#endif

  bp = (unsigned char *)data;
  bp_end = bp + data_size;
  X509 *x509 = d2i_X509(nullptr, &bp, bp_end - bp);
  while (x509 != nullptr) {
    X509_STORE_add_cert(store, x509);
    ++count;
    x509 = d2i_X509(nullptr, &bp, bp_end - bp);
  }

  return count;
}

/**
 * Arranges the text and controls within the dialog.
 */
void AuthDialog::
layout() {
  wxString header, text;
  get_text(header, text);

  wxPanel *panel = new wxPanel(this);
  wxBoxSizer *vsizer = new wxBoxSizer(wxVERTICAL);

  wxFont font = panel->GetFont();
  wxFont *bold_font = wxTheFontList->FindOrCreateFont
    ((int)(font.GetPointSize() * 1.5),
     font.GetFamily(), font.GetStyle(), wxFONTWEIGHT_BOLD);

  if (!header.IsEmpty()) {
    wxStaticText *text0 = new wxStaticText
      (panel, wxID_ANY, header, wxDefaultPosition, wxDefaultSize,
       wxALIGN_CENTER);
    text0->SetFont(*bold_font);
    vsizer->Add(text0, 0, wxCENTER | wxALL, 10);
  }

  wxStaticText *text1 = new wxStaticText
    (panel, wxID_ANY, text, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
  text1->Wrap(400);
  vsizer->Add(text1, 0, wxCENTER | wxALL, 10);

  // Create the run  cancel buttons.
  wxBoxSizer *bsizer = new wxBoxSizer(wxHORIZONTAL);

  if (_verify_result == 0 && _cert != nullptr) {
    wxButton *run_button = new wxButton(panel, wxID_OK, _T("Run"));
    bsizer->Add(run_button, 0, wxALIGN_CENTER | wxALL, 5);
  }

  if (_cert != nullptr) {
    wxButton *view_button = new wxButton(panel, VIEW_CERT_BUTTON, _T("View Certificate"));
    bsizer->Add(view_button, 0, wxALIGN_CENTER | wxALL, 5);
  }

  wxButton *cancel_button = new wxButton(panel, wxID_CANCEL, _T("Cancel"));
  bsizer->Add(cancel_button, 0, wxALIGN_CENTER | wxALL, 5);

  vsizer->Add(bsizer, 0, wxALIGN_CENTER | wxALL, 5);

  panel->SetSizer(vsizer);
  panel->SetAutoLayout(true);
  vsizer->Fit(this);
}

/**
 * Fills in the text appropriate to display in the dialog box, based on the
 * certificate read so far.
 */
void AuthDialog::
get_text(wxString &header, wxString &text) {
  switch (_verify_result) {
  case -1:
    header = _T("No signature!");
    text = no_cert_text;
    break;

  case 0:
    text.Printf(verified_cert_text, _friendly_name.c_str(), _friendly_name.c_str(), _friendly_name.c_str());
    break;

  case X509_V_ERR_CERT_NOT_YET_VALID:
  case X509_V_ERR_CERT_HAS_EXPIRED:
  case X509_V_ERR_CRL_NOT_YET_VALID:
  case X509_V_ERR_CRL_HAS_EXPIRED:
    header = _T("Expired signature!");
    text.Printf(expired_cert_text, _friendly_name.c_str());
    break;

  case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
    header = _T("Unverified signature!");
    text.Printf(unknown_auth_cert_text, _friendly_name.c_str());
    break;

  case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
  case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
    header = _T("Unverified signature!");
    text = self_signed_cert_text;
    break;

  default:
    header = _T("Unverified signature!");
    text.Printf(generic_error_cert_text, _verify_result);
  }
}


// The event table for ViewCertDialog.
BEGIN_EVENT_TABLE(ViewCertDialog, wxDialog)
    EVT_BUTTON(wxID_OK, ViewCertDialog::run_clicked)
    EVT_BUTTON(wxID_CANCEL, ViewCertDialog::cancel_clicked)
END_EVENT_TABLE()

/**
 *
 */
ViewCertDialog::
ViewCertDialog(AuthDialog *auth_dialog, X509 *cert) :
wxDialog(nullptr, wxID_ANY, _T("View Certificate"), wxDefaultPosition,
         wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
  _auth_dialog(auth_dialog),
  _cert(cert)
{
  layout();
}

/**
 *
 */
ViewCertDialog::
~ViewCertDialog() {
  if (_auth_dialog != nullptr) {
    _auth_dialog->_view_cert_dialog = nullptr;
  }
}

/**
 * The user clicks the "Run" button.
 */
void ViewCertDialog::
run_clicked(wxCommandEvent &event) {
  if (_auth_dialog != nullptr){
    _auth_dialog->approve_cert();
  }
  Destroy();
}

/**
 * The user clicks the "Cancel" button.
 */
void ViewCertDialog::
cancel_clicked(wxCommandEvent &event) {
  if (_auth_dialog != nullptr){
    _auth_dialog->Destroy();
  }
  Destroy();
}

/**
 * Arranges the text and controls within the dialog.
 */
void ViewCertDialog::
layout() {
  // Format the certificate text for display in the dialog.
  assert(_cert != nullptr);

  BIO *mbio = BIO_new(BIO_s_mem());
  X509_print(mbio, _cert);

  char *pp;
  long pp_size = BIO_get_mem_data(mbio, &pp);
  wxString cert_body(pp, wxConvUTF8, pp_size);
  BIO_free(mbio);

  wxPanel *panel = new wxPanel(this);
  wxBoxSizer *vsizer = new wxBoxSizer(wxVERTICAL);

  wxScrolledWindow *slwin = new wxScrolledWindow
    (panel, -1, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxHSCROLL | wxBORDER_SUNKEN);
  slwin->SetScrollRate(20, 20);

  wxBoxSizer *slsizer = new wxBoxSizer(wxVERTICAL);

  wxStaticText *text1 = new wxStaticText
    (slwin, wxID_ANY, cert_body, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
  slsizer->Add(text1, 0, wxEXPAND, 0);
  slwin->SetSizer(slsizer);

  vsizer->Add(slwin, 1, wxEXPAND | wxALL, 10);

  // Create the run  cancel buttons.
  wxBoxSizer *bsizer = new wxBoxSizer(wxHORIZONTAL);

  wxButton *run_button = new wxButton(panel, wxID_OK, _T("Run"));
  bsizer->Add(run_button, 0, wxALIGN_CENTER | wxALL, 5);

  wxButton *cancel_button = new wxButton(panel, wxID_CANCEL, _T("Cancel"));
  bsizer->Add(cancel_button, 0, wxALIGN_CENTER | wxALL, 5);

  vsizer->Add(bsizer, 0, wxALIGN_CENTER | wxALL, 5);

  panel->SetSizer(vsizer);
  panel->SetAutoLayout(true);
  vsizer->Fit(this);

  // Make sure the resulting window is at least a certain size.
  int width, height;
  GetSize(&width, &height);
  SetSize(std::max(width, 600), std::max(height, 400));
}
