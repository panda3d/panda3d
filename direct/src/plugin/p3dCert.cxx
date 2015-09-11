// Filename: p3dCert.cxx
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

#include "p3dCert.h"
#include "p3dCert_strings.h"
#include "wstring_encode.h"
#include "mkdir_complete.h"

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Text_Display.H>

#include <cassert>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <locale.h>

#define BUTTON_WIDTH 180 // fit the Russian text
#define BUTTON_SPACE 10

#include "ca_bundle_data_src.c"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <malloc.h>

#define snprintf sprintf_s
#endif

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

static LanguageIndex li = LI_default;

#if defined(_WIN32)
static LanguageIndex detect_language() {
  // This function was introduced in Windows Vista; it may not be available
  // on older systems.
  typedef BOOL (*GUPL)(DWORD, PULONG, PZZWSTR, PULONG);
  GUPL pGetUserPreferredUILanguages = (GUPL)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")),
                                                           TEXT("GetUserPreferredUILanguages"));
  if (pGetUserPreferredUILanguages != NULL) {
    ULONG num_langs = 0;
    ULONG buffer_size = 0;
    pGetUserPreferredUILanguages(8, &num_langs, NULL, &buffer_size);
    PZZWSTR buffer = (PZZWSTR)_alloca(buffer_size);
    if (pGetUserPreferredUILanguages(8, &num_langs, buffer, &buffer_size)) {
      for (ULONG i = 0; i < num_langs; ++i) {
        size_t len = wcslen(buffer);
        if (len >= 2 && (buffer[2] == 0 || buffer[2] == L'-')) {
          // It may be a two-letter code; match it in our list.
          for (int j = 0; j < LI_COUNT; ++j) {
            const char *lang_code = language_codes[j];
            if (lang_code != NULL && lang_code[0] == buffer[0] &&
                                     lang_code[1] == buffer[1]) {
              return (LanguageIndex)j;
            }
          }
        }
        buffer += len + 1;
      }
    }
  }

  // Fall back to the old Windows XP function.
  LANGID lang = GetUserDefaultUILanguage() & 0x3ff;
  if (lang == 0) {
    return LI_default;
  }

  for (int i = 0; i < LI_COUNT; ++i) {
    if (language_ids[i] != 0 && language_ids[i] == lang) {
      return (LanguageIndex)i;
    }
  }
  return LI_default;
}

#elif defined(__APPLE__)
static LanguageIndex detect_language() {
  // Get and iterate through the list of preferred languages.
  CFArrayRef langs = CFLocaleCopyPreferredLanguages();
  CFIndex num_langs = CFArrayGetCount(langs);

  for (long i = 0; i < num_langs; ++i) {
    CFStringRef lang = (CFStringRef)CFArrayGetValueAtIndex(langs, i);

    CFIndex length = CFStringGetLength(lang);
    if (length < 2) {
      continue;
    }

    CFIndex max_size = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
    char *buffer = (char *)alloca(max_size);
    if (!CFStringGetCString(lang, buffer, max_size, kCFStringEncodingUTF8)) {
      continue;
    }

    if (isalnum(buffer[2])) {
      // It's not a two-letter code.
      continue;
    }

    // See if we support this language.
    for (int j = 0; j < LI_COUNT; ++j) {
      const char *lang_code = language_codes[j];
      if (lang_code != NULL && strncasecmp(buffer, lang_code, 2) == 0) {
        CFRelease(langs);
        return (LanguageIndex)j;
      }
    }
  }

  CFRelease(langs);
  return LI_default;
}

#else
static LanguageIndex detect_language() {
  // First consult the LANGUAGE variable, which is a GNU extension that can
  // contain multiple languages in order of preference.
  const char *lang = getenv("LANGUAGE");
  while (lang != NULL && lang[0] != 0) {
    size_t len;
    const char *next = strchr(lang, ':');
    if (next == NULL) {
      len = strlen(lang);
    } else {
      len = (next - lang);
      ++next;
    }

    if (len >= 2 && !isalnum(lang[2])) {
      // It may be a two-letter language code; match it in our list.
      for (int i = 0; i < LI_COUNT; ++i) {
        const char *lang_code = language_codes[i];
        if (lang_code != NULL && strncasecmp(lang, lang_code, 2) == 0) {
          return (LanguageIndex)i;
        }
      }
    }

    lang = next;
  }

  // Fall back to the C locale functions.
  setlocale(LC_ALL, "");
  lang = setlocale(LC_MESSAGES, NULL);

  if (lang == NULL || lang[0] == 0 || strcmp(lang, "C") == 0) {
    // Try the LANG variable.
    lang = getenv("LANG");
  }

  if (lang == NULL || strlen(lang) < 2 || isalnum(lang[2])) {
    // Couldn't extract a meaningful two-letter code.
    return LI_default;
  }

  // It may be a two-letter language code; match it in our list.
  for (int i = 0; i < LI_COUNT; ++i) {
    const char *lang_code = language_codes[i];
    if (lang_code != NULL && strncasecmp(lang, lang_code, 2) == 0) {
      return (LanguageIndex)i;
    }
  }
  return LI_default;
}
#endif

#ifdef _WIN32
int WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
  OpenSSL_add_all_algorithms();

  LPWSTR *argv;
  int argc;
  argv = CommandLineToArgvW(pCmdLine, &argc);
  if (argv == NULL || argc != 2) {
    cerr << "usage: p3dcert cert_filename cert_dir\n";
    return 1;
  }

  li = detect_language();

  wstring cert_filename (argv[0]);
  wstring cert_dir (argv[1]);
  LocalFree(argv);

  AuthDialog *dialog = new AuthDialog(cert_filename, cert_dir);
  dialog->show();

  return Fl::run();
}

#else // _WIN32
int main(int argc, char **argv) {
  OpenSSL_add_all_algorithms();

  if (argc != 3) {
    cerr << "usage: p3dcert cert_filename cert_dir\n";
    return 1;
  }

  li = detect_language();

  string cert_filename (argv[1]);
  string cert_dir (argv[2]);

  AuthDialog *dialog = new AuthDialog(cert_filename, cert_dir);
  dialog->show(1, argv);

  return Fl::run();
}
#endif // _WIN32

////////////////////////////////////////////////////////////////////
//     Function: AuthDialog::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
#ifdef _WIN32
AuthDialog::
AuthDialog(const wstring &cert_filename, const wstring &cert_dir) :
#else
AuthDialog::
AuthDialog(const string &cert_filename, const string &cert_dir) :
#endif
  Fl_Window(435, 242, new_application_title[li]),
  _cert_dir(cert_dir)
{
  _view_cert_dialog = NULL;

  _cert = NULL;
  _stack = NULL;
  _verify_result = -1;

  // Center the window on the screen.
  position((Fl::w() - w()) / 2, (Fl::h() - h()) / 2);
  set_modal();

  read_cert_file(cert_filename);
  get_friendly_name();
  verify_cert();
  layout();
}

////////////////////////////////////////////////////////////////////
//     Function: AuthDialog::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
AuthDialog::
~AuthDialog() {
  if (_view_cert_dialog != NULL) {
    _view_cert_dialog->hide();
  }

  if (_cert != NULL) {
    X509_free(_cert);
    _cert = NULL;
  }
  if (_stack != NULL) {
    sk_X509_free(_stack);
    _stack = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AuthDialog::run_clicked
//       Access: Public
//  Description: The user clicks the "Run" button.
////////////////////////////////////////////////////////////////////
void AuthDialog::
run_clicked(Fl_Widget *w, void *data) {
  AuthDialog *dlg = (AuthDialog *) data;
  dlg->approve_cert();
}

////////////////////////////////////////////////////////////////////
//     Function: AuthDialog::run_clicked
//       Access: Public
//  Description: The user clicks the "View Certificate" button.
////////////////////////////////////////////////////////////////////
void AuthDialog::
view_cert_clicked(Fl_Widget *w, void *data) {
  AuthDialog *dlg = (AuthDialog *) data;

  if (dlg->_view_cert_dialog != NULL) {
    dlg->_view_cert_dialog->hide();
  }
  dlg->hide();
  dlg->_view_cert_dialog = new ViewCertDialog(dlg, dlg->_cert);
  dlg->_view_cert_dialog->show();
}

////////////////////////////////////////////////////////////////////
//     Function: AuthDialog::run_clicked
//       Access: Public
//  Description: The user clicks the "Cancel" button.
////////////////////////////////////////////////////////////////////
void AuthDialog::
cancel_clicked(Fl_Widget *w, void *data) {
  AuthDialog *dlg = (AuthDialog *) data;
  dlg->hide();
}

////////////////////////////////////////////////////////////////////
//     Function: AuthDialog::approve_cert
//       Access: Public
//  Description: Writes the certificate into the _cert_dir, so
//               that it will be found by the P3DInstanceManager and
//               known to be approved.
////////////////////////////////////////////////////////////////////
void AuthDialog::
approve_cert() {
  assert(_cert != NULL);

  // Make sure the directory exists.
#ifdef _WIN32
  mkdir_complete_w(_cert_dir, cerr);
#else
  mkdir_complete(_cert_dir, cerr);
#endif

  // Look for an unused filename.
  int i = 1;
  size_t buf_length = _cert_dir.length() + 100;

  // Sure, there's a slight race condition right now: another process
  // might attempt to create the same filename.  So what.
  FILE *fp = NULL;

#ifdef _WIN32
  wchar_t *buf = new wchar_t[buf_length];

  while (true) {
    swprintf(buf, L"%s/p%d.crt", _cert_dir.c_str(), i);
    assert(wcslen(buf) < buf_length);

    // Check if it already exists.  If not, take it.
    if (GetFileAttributesW(buf) == -1) {
      break;
    }
    ++i;
  }
  fp = _wfopen(buf, L"w");

#else // _WIN32
  char *buf = new char[buf_length];

  while (true) {
    sprintf(buf, "%s/p%d.crt", _cert_dir.c_str(), i);
    assert(strlen(buf) < buf_length);

    // Check if it already exists.  If not, take it.
    struct stat statbuf;
    if (stat(buf, &statbuf) != 0) {
      break;
    }
    ++i;
  }
  fp = fopen(buf, "w");
#endif // _WIN32

  if (fp != NULL) {
    PEM_write_X509(fp, _cert);
    fclose(fp);
  }

  hide();
}

////////////////////////////////////////////////////////////////////
//     Function: AuthDialog::read_cert_file
//       Access: Private
//  Description: Reads the list of certificates in the pem filename
//               passed on the command line into _cert and _stack.
////////////////////////////////////////////////////////////////////
#ifdef _WIN32
void AuthDialog::
read_cert_file(const wstring &cert_filename) {
#else
void AuthDialog::
read_cert_file(const string &cert_filename) {
#endif

  FILE *fp = NULL;
#ifdef _WIN32
  fp = _wfopen(cert_filename.c_str(), L"r");
#else // _WIN32
  fp = fopen(cert_filename.c_str(), "r");
#endif  // _WIN32

  if (fp == NULL) {
#ifdef _WIN32
    wcerr << L"Couldn't read " << cert_filename.c_str() << L"\n";
#else
    cerr << "Couldn't read " << cert_filename.c_str() << "\n";
#endif
    return;
  }
  _cert = PEM_read_X509(fp, NULL, NULL, (void *)"");
  if (_cert == NULL) {
#ifdef _WIN32
    wcerr << L"Could not read certificate in " << cert_filename.c_str() << L".\n";
#else
    cerr << "Could not read certificate in " << cert_filename.c_str() << ".\n";
#endif
    fclose(fp);
    return;
  }

  // Build up a STACK of the remaining certificates in the file.
  _stack = sk_X509_new(NULL);
  X509 *c = PEM_read_X509(fp, NULL, NULL, (void *)"");
  while (c != NULL) {
    sk_X509_push(_stack, c);
    c = PEM_read_X509(fp, NULL, NULL, (void *)"");
  }

  fclose(fp);
}

////////////////////////////////////////////////////////////////////
//     Function: AuthDialog::get_friendly_name
//       Access: Private
//  Description: Extracts the "friendly name" from the certificate:
//               the common name or email name.
////////////////////////////////////////////////////////////////////
void AuthDialog::
get_friendly_name() {
  if (_cert == NULL) {
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
    if (xname != NULL) {
      int pos = X509_NAME_get_index_by_NID(xname, nid, -1);
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
            _friendly_name = string(pp, pp_size);
            BIO_free(mbio);
            return;
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AuthDialog::verify_cert
//       Access: Private
//  Description: Checks whether the certificate is valid by the chain
//               and initializes _verify_status accordingly.
////////////////////////////////////////////////////////////////////
void AuthDialog::
verify_cert() {
  if (_cert == NULL) {
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

  cerr << "Got certificate from " << _friendly_name.c_str()
       << ", verify_result = " << _verify_result << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: AuthDialog::load_certificates_from_der_ram
//       Access: Public
//  Description: Reads a chain of trusted certificates from the
//               indicated data buffer and adds them to the X509_STORE
//               object.  The data buffer should be DER-formatted.
//               Returns the number of certificates read on success,
//               or 0 on failure.
//
//               You should call this only with trusted,
//               locally-stored certificates; not with certificates
//               received from an untrusted source.
////////////////////////////////////////////////////////////////////
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
  X509 *x509 = d2i_X509(NULL, &bp, bp_end - bp);
  while (x509 != NULL) {
    X509_STORE_add_cert(store, x509);
    ++count;
    x509 = d2i_X509(NULL, &bp, bp_end - bp);
  }

  return count;
}

////////////////////////////////////////////////////////////////////
//     Function: AuthDialog::layout
//       Access: Private
//  Description: Arranges the text and controls within the dialog.
////////////////////////////////////////////////////////////////////
void AuthDialog::
layout() {
  get_text(_header, sizeof _header, _text, sizeof _text);

  // Now replicate out any @ signs in the text to avoid FlTk's escape
  // sequences.
  int j = 0;
  for (int i = 0; _text[i] != '\0'; ++i) {
    _text_clean[j++] = _text[i];
    if (_text[i] == '@') {
      _text_clean[j++] = _text[i];
    }
  }
  _text_clean[j] = '\0';
  assert(strlen(_text_clean) < sizeof(_text_clean));

  int next_y = 35;
  if (strlen(_header) > 0) {
    Fl_Box *text0 = new Fl_Box(w() / 2, next_y, 0, 25, _header);
    text0->align(FL_ALIGN_TOP | FL_ALIGN_CENTER);
    text0->labelfont(FL_BOLD);
    text0->labelsize(text0->labelsize() * 1.5);
    next_y += 25;
  }

  Fl_Box *text1 = new Fl_Box(17, next_y, 400, 180, _text_clean);
  text1->align(FL_ALIGN_TOP | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
  next_y += 180;

  short nbuttons = 1;
  if (_cert != NULL) {
    nbuttons++;
    if (_verify_result == 0) {
      nbuttons++;
    }
  }
  short bx = (w() - nbuttons * BUTTON_WIDTH - (nbuttons - 1) * BUTTON_SPACE) / 2;

  if (_verify_result == 0 && _cert != NULL) {
    Fl_Return_Button *run_button = new Fl_Return_Button(bx, next_y, BUTTON_WIDTH, 25, run_title[li]);
    run_button->callback(this->run_clicked, this);
    bx += BUTTON_WIDTH + BUTTON_SPACE;
  }

  if (_cert != NULL) {
    Fl_Button *view_button = new Fl_Button(bx, next_y, BUTTON_WIDTH, 25, show_cert_title[li]);
    view_button->callback(this->view_cert_clicked, this);
    bx += BUTTON_WIDTH + BUTTON_SPACE;
  }

  Fl_Button *cancel_button;
  cancel_button = new Fl_Button(bx, next_y, BUTTON_WIDTH, 25, cancel_title[li]);
  cancel_button->callback(this->cancel_clicked, this);

  next_y += 42;
  size(435, next_y);

  end();
  set_modal();
}

////////////////////////////////////////////////////////////////////
//     Function: AuthDialog::get_text
//       Access: Private
//  Description: Fills in the text appropriate to display in the
//               dialog box, based on the certificate read so far.
////////////////////////////////////////////////////////////////////
void AuthDialog::
get_text(char *header, size_t hlen, char *text, size_t tlen) {
  switch (_verify_result) {
  case -1:
    strncpy(header, no_cert_title[li], hlen);
    strncpy(text, no_cert_text[li], tlen);
    break;

  case 0:
    snprintf(text, tlen, verified_cert_text[li], _friendly_name.c_str(),
                        _friendly_name.c_str(), _friendly_name.c_str());
    break;

  case X509_V_ERR_CERT_NOT_YET_VALID:
  case X509_V_ERR_CERT_HAS_EXPIRED:
  case X509_V_ERR_CRL_NOT_YET_VALID:
  case X509_V_ERR_CRL_HAS_EXPIRED:
    strncpy(header, expired_cert_title[li], hlen);
    snprintf(text, tlen, expired_cert_text[li], _friendly_name.c_str());
    break;

  case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
    strncpy(header, unverified_cert_title[li], hlen);
    strncpy(text, unknown_auth_cert_text[li], tlen);
    break;

  case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
  case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
    strncpy(header, unverified_cert_title[li], hlen);
    strncpy(text, self_signed_cert_text[li], tlen);
    break;

  default:
    strncpy(header, unverified_cert_title[li], hlen);
    snprintf(text, tlen, generic_error_cert_text[li], _verify_result);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ViewCertDialog::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ViewCertDialog::
ViewCertDialog(AuthDialog *auth_dialog, X509 *cert) :
  Fl_Window(600, 400, show_cert_title[li]),
  _auth_dialog(auth_dialog),
  _cert(cert)
{
  // Center the window on the screen.
  position((Fl::w() - w()) / 2, (Fl::h() - h()) / 2);
  set_modal();

  layout();
}

////////////////////////////////////////////////////////////////////
//     Function: ViewCertDialog::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
ViewCertDialog::
~ViewCertDialog() {
  if (_auth_dialog != NULL) {
    _auth_dialog->_view_cert_dialog = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ViewCertDialog::run_clicked
//       Access: Public
//  Description: The user clicks the "Run" button.
////////////////////////////////////////////////////////////////////
void ViewCertDialog::
run_clicked(Fl_Widget *w, void *data) {
  ViewCertDialog *dlg = (ViewCertDialog *) data;
  if (dlg->_auth_dialog != NULL){
    dlg->_auth_dialog->approve_cert();
  }
  dlg->hide();
}

////////////////////////////////////////////////////////////////////
//     Function: ViewCertDialog::run_clicked
//       Access: Public
//  Description: The user clicks the "Cancel" button.
////////////////////////////////////////////////////////////////////
void ViewCertDialog::
cancel_clicked(Fl_Widget *w, void *data) {
  ViewCertDialog *dlg = (ViewCertDialog *) data;
  if (dlg->_auth_dialog != NULL){
    dlg->_auth_dialog->hide();
  }
  dlg->hide();
}

////////////////////////////////////////////////////////////////////
//     Function: ViewCertDialog::layout
//       Access: Private
//  Description: Arranges the text and controls within the dialog.
////////////////////////////////////////////////////////////////////
void ViewCertDialog::
layout() {
  // Format the certificate text for display in the dialog.
  assert(_cert != NULL);

  BIO *mbio = BIO_new(BIO_s_mem());
  X509_print(mbio, _cert);

  char *pp;
  long pp_size = BIO_get_mem_data(mbio, &pp);
  string cert_body(pp, pp_size);
  BIO_free(mbio);

  Fl_Text_Buffer *buffer = new Fl_Text_Buffer;
  buffer->append(cert_body.c_str());

  Fl_Text_Display *text = new Fl_Text_Display(20, 20, 565, 320);
  text->buffer(buffer);

  short bx = (w() - BUTTON_WIDTH * 2 - BUTTON_SPACE) / 2;

  Fl_Return_Button *run_button = new Fl_Return_Button(bx, 360, BUTTON_WIDTH, 25, run_title[li]);
  run_button->callback(this->run_clicked, this);

  bx += BUTTON_WIDTH + BUTTON_SPACE;

  Fl_Button *cancel_button = new Fl_Button(bx, 360, BUTTON_WIDTH, 25, cancel_title[li]);
  cancel_button->callback(this->cancel_clicked, this);

  end();
  set_modal();
}
