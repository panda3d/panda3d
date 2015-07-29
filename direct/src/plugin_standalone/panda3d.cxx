// Filename: panda3d.cxx
// Created by:  drose (03Jun09)
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

#include "panda3d.h"
#include "load_plugin.h"
#include "p3d_plugin_config.h"
#include "find_root_dir.h"
#include "pandaSystem.h"
#include "dtool_platform.h"
#include "pandaVersion.h"
#include "panda_getopt.h"

#include <ctype.h>
#include <sstream>
#include <algorithm>
#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#endif

////////////////////////////////////////////////////////////////////
//     Function: Panda3D::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
Panda3D::
Panda3D(bool console_environment) : Panda3DBase(console_environment) {
  // We use the runtime PandaSystem setting for this value, rather
  // than the hard-compiled-in setting, to allow users to override
  // this with a Config.prc variable if needed.
  _host_url = PandaSystem::get_package_host_url();
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3D::run_command_line
//       Access: Public
//  Description: Starts the program going, with command-line arguments.  
//               Returns 0 on success, nonzero on failure.
////////////////////////////////////////////////////////////////////
int Panda3D::
run_command_line(int argc, char *argv[]) {
  extern char *optarg;
  extern int optind;

  // We prefix a "+" sign to tell getopt not to parse options
  // following the first not-option parameter.  (These will be passed
  // into the sub-process.)
  const char *optstr = "+mu:M:Sp:nfw:t:s:o:l:iVUPh";

  bool allow_multiple = false;

  int flag = getopt(argc, argv, optstr);

  while (flag != EOF) {
    switch (flag) {
    case 'm':
      allow_multiple = true;
      break;

    case 'u':
      _host_url = optarg;
      break;

    case 'M':
      _super_mirror_url = optarg;
      break;

    case 'S':
      _enable_security = true;
      break;

    case 'p':
      _this_platform = optarg;
      _coreapi_platform = optarg;
      break;

    case 'n':
      _verify_contents = P3D_VC_normal;
      break;

    case 'f':
      _verify_contents = P3D_VC_force;
      break;

    case 'w':
      if (strcmp(optarg, "toplevel") == 0) {
        _window_type = P3D_WT_toplevel;
      } else if (strcmp(optarg, "embedded") == 0) {
        _window_type = P3D_WT_embedded;
      } else if (strcmp(optarg, "fullscreen") == 0) {
        _window_type = P3D_WT_fullscreen;
      } else if (strcmp(optarg, "hidden") == 0) {
        _window_type = P3D_WT_hidden;
      } else {
        cerr << "Invalid value for -w: " << optarg << "\n";
        return 1;
      }
      break;

    case 't':
      if (!parse_token(optarg)) {
        cerr << "Web tokens (-t) must be of the form token=value: " << optarg << "\n";
        return 1;
      }
      break;

    case 's':
      if (!parse_int_pair(optarg, _win_width, _win_height)) {
        cerr << "Invalid value for -s: " << optarg << "\n";
        return 1;
      }
      _got_win_size = true;
      break;

    case 'o':
      if (!parse_int_pair(optarg, _win_x, _win_y)) {
        cerr << "Invalid value for -o: " << optarg << "\n";
        return 1;
      }
      break;

    case 'l':
      _log_dirname = Filename::from_os_specific(optarg).to_os_specific();
      _log_basename = "p3dcore";
      break;

    case 'i':
      {
        P3D_token token;
        token._keyword = "keep_pythonpath";
        token._value = "1";
        _tokens.push_back(token);
        token._keyword = "interactive_console";
        token._value = "1";
        _tokens.push_back(token);

        // We should also ignore control-C in this case, so that an
        // interrupt will be delivered to the subordinate Python
        // process and return to a command shell, and won't just kill
        // the panda3d process.
#ifdef _WIN32
        SetConsoleCtrlHandler(NULL, true);
#else
        struct sigaction ignore;
        memset(&ignore, 0, sizeof(ignore));
        ignore.sa_handler = SIG_IGN;
        sigaction(SIGINT, &ignore, NULL);
#endif  // _WIN32
      }
      break;

    case 'V':
      cout << P3D_PLUGIN_MAJOR_VERSION << "."
           << P3D_PLUGIN_MINOR_VERSION << "."
           << P3D_PLUGIN_SEQUENCE_VERSION;
#ifndef PANDA_OFFICIAL_VERSION
      cout << "c";
#endif
      cout << "\n";
      exit(0);

    case 'U':
      cout << _host_url << "\n";
      exit(0);

    case 'P':
      cout << DTOOL_PLATFORM << "\n";
      exit(0);

    case 'h':
    case '?':
    case '+':
    default:
      usage();
      return 1;
    }
    flag = getopt(argc, argv, optstr);
  }

  argc -= (optind-1);
  argv += (optind-1);

  if (argc < 2 && _exit_with_last_instance) {
    // No instances on the command line--that *might* be an error.
    usage();
    return 1;
  }

  if (!post_arg_processing()) {
    return 1;
  }

  int num_instance_filenames, num_instance_args;
  char **instance_filenames, **instance_args;

  if (argc > 1) {
    if (allow_multiple) {
      // With -m, the remaining arguments are all instance filenames.
      num_instance_filenames = argc - 1;
      instance_filenames = argv + 1;
      num_instance_args = 0;
      instance_args = argv + argc;
      
    } else {
      // Without -m, there is one instance filename, and everything else
      // gets delivered to that instance.
      num_instance_filenames = 1;
      instance_filenames = argv + 1;
      num_instance_args = argc - 2;
      instance_args = argv + 2;
    }
    
    if (_window_type == P3D_WT_embedded) {
      // The user asked for an embedded window.  Create a toplevel
      // window to be its parent, of the requested size.
      if (_win_width == 0 && _win_height == 0) {
        _win_width = 800;
        _win_height = 600;
      }
      
      make_parent_window();
      
      // Center the child window(s) within the parent window.
#ifdef _WIN32
      assert(_parent_window._window_handle_type == P3D_WHT_win_hwnd);
      HWND parent_hwnd = _parent_window._handle._win_hwnd._hwnd;
      
      RECT rect;
      GetClientRect(parent_hwnd, &rect);
      
      _win_x = (int)(rect.right * 0.1);
      _win_y = (int)(rect.bottom * 0.1);
      _win_width = (int)(rect.right * 0.8);
      _win_height = (int)(rect.bottom * 0.8);
#endif
      
      // Subdivide the window into num_x_spans * num_y_spans sub-windows.
      int num_y_spans = int(sqrt((double)num_instance_filenames));
      int num_x_spans = (num_instance_filenames + num_y_spans - 1) / num_y_spans;
      
      int origin_x = _win_x;
      int origin_y = _win_y;
      _win_width = _win_width / num_x_spans;
      _win_height = _win_height / num_y_spans;
      _got_win_size = true;
      
      for (int yi = 0; yi < num_y_spans; ++yi) {
        for (int xi = 0; xi < num_x_spans; ++xi) {
          int i = yi * num_x_spans + xi;
          if (i >= num_instance_filenames) {
            continue;
          }
          
          // Create instance i at window slot (xi, yi).
          _win_x = origin_x + xi * _win_width;
          _win_y = origin_y + yi * _win_height;
          
          P3D_instance *inst = create_instance
            (instance_filenames[i], true,
             instance_args, num_instance_args);
          _instances.insert(inst);
        }
      }
      
    } else {
      // Not an embedded window.  Create each window with the same parameters.
      for (int i = 0; i < num_instance_filenames; ++i) {
        P3D_instance *inst = create_instance
          (instance_filenames[i], true, 
           instance_args, num_instance_args);
        _instances.insert(inst);
      }
    }
  }
    
  run_main_loop();

  // All instances have finished; we can exit.
  unload_plugin(cerr);
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3D::post_arg_processing
//       Access: Protected
//  Description: Sets up some internal state after processing the
//               command-line arguments.  Returns true on success,
//               false on failure.
////////////////////////////////////////////////////////////////////
bool Panda3D::
post_arg_processing() {
  // Now is a good time to assign _root_dir.
  _root_dir = find_root_dir();

  // Set host_url_prefix to end with a slash.
  _host_url_prefix = _host_url;
  if (!_host_url_prefix.empty() && _host_url_prefix[_host_url_prefix.length() - 1] != '/') {
    _host_url_prefix += '/';
  }
  _download_url_prefix = _host_url_prefix;

  // If the "super mirror" URL is a filename, convert it to a file:// url.
  if (!_super_mirror_url.empty()) {
    if (!is_url(_super_mirror_url)) {
      Filename filename = Filename::from_os_specific(_super_mirror_url);
      filename.make_absolute();
      string path = filename.to_os_generic();
      if (!path.empty() && path[0] != '/') {
        // On Windows, a leading drive letter must be preceded by an
        // additional slash.
        path = "/" + path;
      }
      _super_mirror_url = "file://" + path;
    }

    // And make sure the super_mirror_url_prefix ends with a slash.
    _super_mirror_url_prefix = _super_mirror_url;
    if (!_super_mirror_url_prefix.empty() && _super_mirror_url_prefix[_super_mirror_url_prefix.length() - 1] != '/') {
      _super_mirror_url_prefix += '/';
    }
  }

  if (!get_plugin()) {
    cerr << "Unable to load Panda3D plugin.\n";
    return false;
  }

  // Set up the "super mirror" URL, if specified.
  if (!_super_mirror_url.empty()) {
    P3D_set_super_mirror_ptr(_super_mirror_url.c_str());
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3D::get_plugin
//       Access: Protected
//  Description: Downloads the contents.xml file from the named URL
//               and attempts to use it to load the core API.  Returns
//               true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool Panda3D::
get_plugin() {
  // First, look for the existing contents.xml file.
  bool success = false;

  Filename contents_filename = Filename(Filename::from_os_specific(_root_dir), "contents.xml");
  if (_verify_contents != P3D_VC_force) {
    if (read_contents_file(contents_filename, false)) {
      if (_verify_contents == P3D_VC_none || time(NULL) < _contents_expiration) {
        // Got the file, and it's good.
        success = true;
      }
    }
  }

  if (!success) {
    // Couldn't read it (or it wasn't current enough), so go get a new
    // one.
    HTTPClient *http = HTTPClient::get_global_ptr();
    
    // Try the super_mirror first.
    if (!_super_mirror_url_prefix.empty()) {
      // We don't bother putting a uniquifying query string when we're
      // downloading this file from the super_mirror.  The super_mirror
      // is by definition a cache, so it doesn't make sense to bust
      // caches here.
      string url = _super_mirror_url_prefix + "contents.xml";
      PT(HTTPChannel) channel = http->make_channel(false);
      channel->get_document(url);
      
      Filename tempfile = Filename::temporary("", "p3d_");
      if (!channel->download_to_file(tempfile)) {
        cerr << "Unable to download " << url << "\n";
        tempfile.unlink();
      } else {
        // Successfully downloaded from the super_mirror; try to read it.
        success = read_contents_file(tempfile, true);
        tempfile.unlink();
      }
    }

    if (!success) {
      // Go download contents.xml from the actual host.
      ostringstream strm;
      strm << _host_url_prefix << "contents.xml";
      // Append a uniquifying query string to the URL to force the
      // download to go all the way through any caches.  We use the time
      // in seconds; that's unique enough.
      strm << "?" << time(NULL);
      string url = strm.str();
      
      // We might as well explicitly request the cache to be disabled too,
      // since we have an interface for that via HTTPChannel.
      DocumentSpec request(url);
      request.set_cache_control(DocumentSpec::CC_no_cache);
      
      PT(HTTPChannel) channel = http->make_channel(false);
      channel->get_document(request);
      
      // Since we have to download some of it, might as well ask the core
      // API to check all of it.
      if (_verify_contents == P3D_VC_none) {
        _verify_contents = P3D_VC_normal;
      }
      
      // First, download it to a temporary file.
      Filename tempfile = Filename::temporary("", "p3d_");
      if (!channel->download_to_file(tempfile)) {
        cerr << "Unable to download " << url << "\n";
        
        // Couldn't download, but try to read the existing contents.xml
        // file anyway.  Maybe it's good enough.
        success = read_contents_file(contents_filename, false);
        
      } else {
        // Successfully downloaded; read it and move it into place.
        success = read_contents_file(tempfile, true);
      }

      tempfile.unlink();
    }
  }

  if (success) {
    // Now that we've downloaded the contents file successfully, start
    // the Core API.
    success = get_core_api();
  }

  return success;
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3D::read_contents_file
//       Access: Protected
//  Description: Attempts to open and read the contents.xml file on
//               disk.  Copies the file to its standard location
//               on success.  Returns true on success, false on
//               failure.
////////////////////////////////////////////////////////////////////
bool Panda3D::
read_contents_file(const Filename &contents_filename, bool fresh_download) {
  string os_contents_filename = contents_filename.to_os_specific();
  TiXmlDocument doc(os_contents_filename.c_str());
  if (!doc.LoadFile()) {
    return false;
  }

  bool found_core_package = false;

  TiXmlElement *xcontents = doc.FirstChildElement("contents");
  if (xcontents != NULL) {
    int max_age = P3D_CONTENTS_DEFAULT_MAX_AGE;
    xcontents->Attribute("max_age", &max_age);

    // Get the latest possible expiration time, based on the max_age
    // indication.  Any expiration time later than this is in error.
    time_t now = time(NULL);
    _contents_expiration = now + (time_t)max_age;

    if (fresh_download) {
      // Update the XML with the new download information.
      TiXmlElement *xorig = xcontents->FirstChildElement("orig");
      while (xorig != NULL) {
        xcontents->RemoveChild(xorig);
        xorig = xcontents->FirstChildElement("orig");
      }

      xorig = new TiXmlElement("orig");
      xcontents->LinkEndChild(xorig);
      
      xorig->SetAttribute("expiration", (int)_contents_expiration);

    } else {
      // Read the expiration time from the XML.
      int expiration = 0;
      TiXmlElement *xorig = xcontents->FirstChildElement("orig");
      if (xorig != NULL) {
        xorig->Attribute("expiration", &expiration);
      }
      
      _contents_expiration = min(_contents_expiration, (time_t)expiration);
    }

    // Look for the <host> entry; it might point us at a different
    // download URL, and it might mention some mirrors.
    find_host(xcontents);

    // Now look for the core API package.
    _coreapi_set_ver = "";
    TiXmlElement *xpackage = xcontents->FirstChildElement("package");
    while (xpackage != NULL) {
      const char *name = xpackage->Attribute("name");
      if (name != NULL && strcmp(name, "coreapi") == 0) {
        const char *platform = xpackage->Attribute("platform");
        if (platform != NULL && _coreapi_platform == string(platform)) {
          _coreapi_dll.load_xml(xpackage);
          const char *set_ver = xpackage->Attribute("set_ver");
          if (set_ver != NULL) {
            _coreapi_set_ver = set_ver;
          }
          found_core_package = true;
          break;
        }
      }
    
      xpackage = xpackage->NextSiblingElement("package");
    }
  }

  if (!found_core_package) {
    // Couldn't find the coreapi package description.
    nout << "No coreapi package defined in contents file for "
         << _coreapi_platform << "\n";
    return false;
  }

  // Check the coreapi_set_ver token.  If it is given, it specifies a
  // minimum Core API version number we expect to find.  If we didn't
  // find that number, perhaps our contents.xml is out of date.
  string coreapi_set_ver = lookup_token("coreapi_set_ver");
  if (!coreapi_set_ver.empty()) {
    nout << "Instance asked for Core API set_ver " << coreapi_set_ver
         << ", we found " << _coreapi_set_ver << "\n";
    // But don't bother if we just freshly downloaded it.
    if (!fresh_download) {
      if (compare_seq(coreapi_set_ver, _coreapi_set_ver) > 0) {
        // The requested set_ver value is higher than the one we have on
        // file; our contents.xml file must be out of date after all.
        nout << "expiring contents.xml\n";
        _contents_expiration = 0;
      }
    }
  }

  // Success.  Now copy the file into place.
  Filename standard_filename = Filename(Filename::from_os_specific(_root_dir), "contents.xml");
  if (fresh_download) {
    Filename tempfile = Filename::temporary("", "p3d_");
    string os_specific = tempfile.to_os_specific();
    if (!doc.SaveFile(os_specific.c_str())) {
      nout << "Couldn't write to " << tempfile << "\n";
      tempfile.unlink();
      return false;
    }
    tempfile.rename_to(standard_filename);

  } else {
    if (contents_filename != standard_filename) {
      if (!contents_filename.rename_to(standard_filename)) {
        nout << "Couldn't move contents.xml to " << standard_filename << "\n";
        contents_filename.unlink();
        return false;
      }
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3D::find_host
//       Access: Protected
//  Description: Scans the <contents> element for the matching <host>
//               element.
////////////////////////////////////////////////////////////////////
void Panda3D::
find_host(TiXmlElement *xcontents) {
  TiXmlElement *xhost = xcontents->FirstChildElement("host");
  if (xhost != NULL) {
    const char *url = xhost->Attribute("url");
    if (url != NULL && _host_url == string(url)) {
      // We're the primary host.  This is the normal case.
      read_xhost(xhost);
      return;
      
    } else {
      // We're not the primary host; perhaps we're an alternate host.
      TiXmlElement *xalthost = xhost->FirstChildElement("alt_host");
      while (xalthost != NULL) {
        const char *url = xalthost->Attribute("url");
        if (url != NULL && _host_url == string(url)) {
          // Yep, we're this alternate host.
          read_xhost(xhost);
          return;
        }
        xalthost = xalthost->NextSiblingElement("alt_host");
      }
    }

    // Hmm, didn't find the URL we used mentioned.  Assume we're the
    // primary host.
    read_xhost(xhost);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3D::read_xhost
//       Access: Protected
//  Description: Reads the host data from the <host> (or <alt_host>)
//               entry in the contents.xml file.
////////////////////////////////////////////////////////////////////
void Panda3D::
read_xhost(TiXmlElement *xhost) {
  // Get the "download" URL, which is the source from which we
  // download everything other than the contents.xml file.
  const char *download_url = xhost->Attribute("download_url");
  if (download_url == NULL) {
    download_url = xhost->Attribute("url");
  }

  if (download_url != NULL) {
    _download_url_prefix = download_url;
  } else {
    _download_url_prefix = _host_url_prefix;
  }
  if (!_download_url_prefix.empty()) {
    if (_download_url_prefix[_download_url_prefix.size() - 1] != '/') {
      _download_url_prefix += "/";
    }
  }
        
  TiXmlElement *xmirror = xhost->FirstChildElement("mirror");
  while (xmirror != NULL) {
    const char *url = xmirror->Attribute("url");
    if (url != NULL) {
      add_mirror(url);
    }
    xmirror = xmirror->NextSiblingElement("mirror");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3D::add_mirror
//       Access: Protected
//  Description: Adds a new URL to serve as a mirror for this host.
//               The mirrors will be consulted first, before
//               consulting the host directly.
////////////////////////////////////////////////////////////////////
void Panda3D::
add_mirror(string mirror_url) {
  // Ensure the URL ends in a slash.
  if (!mirror_url.empty() && mirror_url[mirror_url.size() - 1] != '/') {
    mirror_url += '/';
  }
  
  // Add it to the _mirrors list, but only if it's not already
  // there.
  if (find(_mirrors.begin(), _mirrors.end(), mirror_url) == _mirrors.end()) {
    _mirrors.push_back(mirror_url);
  }
}
    
////////////////////////////////////////////////////////////////////
//     Function: Panda3D::choose_random_mirrors
//       Access: Public
//  Description: Selects num_mirrors elements, chosen at random, from
//               the _mirrors list.  Adds the selected mirrors to
//               result.  If there are fewer than num_mirrors elements
//               in the list, adds only as many mirrors as we can get.
////////////////////////////////////////////////////////////////////
void Panda3D::
choose_random_mirrors(vector_string &result, int num_mirrors) {
  pvector<size_t> selected;

  size_t num_to_select = min(_mirrors.size(), (size_t)num_mirrors);
  while (num_to_select > 0) {
    size_t i = (size_t)(((double)rand() / (double)RAND_MAX) * _mirrors.size());
    while (find(selected.begin(), selected.end(), i) != selected.end()) {
      // Already found this i, find a new one.
      i = (size_t)(((double)rand() / (double)RAND_MAX) * _mirrors.size());
    }
    selected.push_back(i);
    result.push_back(_mirrors[i]);
    --num_to_select;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: Panda3D::get_core_api
//       Access: Protected
//  Description: Checks the core API DLL file against the
//               specification in the contents file, and downloads it
//               if necessary.
////////////////////////////////////////////////////////////////////
bool Panda3D::
get_core_api() {
  if (!_coreapi_dll.quick_verify(_root_dir)) {
    // The DLL file needs to be downloaded.  Build up our list of
    // URL's to attempt to download it from, in reverse order.
    string url;
    vector_string core_urls;

    // Our last act of desperation: hit the original host, with a
    // query uniquifier, to break through any caches.
    ostringstream strm;
    strm << _download_url_prefix << _coreapi_dll.get_filename()
         << "?" << time(NULL);
    url = strm.str();
    core_urls.push_back(url);

    // Before we try that, we'll hit the original host, without a
    // uniquifier.
    url = _download_url_prefix;
    url += _coreapi_dll.get_filename();
    core_urls.push_back(url);

    // And before we try that, we'll try two mirrors, at random.
    vector_string mirrors;
    choose_random_mirrors(mirrors, 2);
    for (vector_string::iterator si = mirrors.begin();
         si != mirrors.end(); 
         ++si) {
      url = (*si) + _coreapi_dll.get_filename();
      core_urls.push_back(url);
    }

    // The very first thing we'll try is the super_mirror, if we have
    // one.
    if (!_super_mirror_url_prefix.empty()) {
      url = _super_mirror_url_prefix + _coreapi_dll.get_filename();
      core_urls.push_back(url);
    }

    // Now pick URL's off the list, and try them, until we have
    // success.
    Filename pathname = Filename::from_os_specific(_coreapi_dll.get_pathname(_root_dir));
    pathname.make_dir();
    HTTPClient *http = HTTPClient::get_global_ptr();

    bool success = false;
    while (!core_urls.empty()) {
      url = core_urls.back();
      core_urls.pop_back();
    
      PT(HTTPChannel) channel = http->get_document(url);
      if (!channel->download_to_file(pathname)) {
        cerr << "Unable to download " << url << "\n";

      } else if (!_coreapi_dll.full_verify(_root_dir)) {
        cerr << "Mismatched download for " << url << "\n";

      } else {
        // successfully downloaded!
        success = true;
        break;
      }
    }

    if (!success) {
      cerr << "Couldn't download core API.\n";
      return false;
    }

    // Since we had to download some of it, might as well ask the core
    // API to check all of it.
    if (_verify_contents == P3D_VC_none) {
      _verify_contents = P3D_VC_normal;
    }
  }

  // Now we've got the DLL.  Load it.
  string pathname = _coreapi_dll.get_pathname(_root_dir);

#ifdef P3D_PLUGIN_P3D_PLUGIN
  // This is a convenience macro for development.  If defined and
  // nonempty, it indicates the name of the plugin DLL that we will
  // actually run, even after downloading a possibly different
  // (presumably older) version.  Its purpose is to simplify iteration
  // on the plugin DLL.
  string override_filename = P3D_PLUGIN_P3D_PLUGIN;
  if (!override_filename.empty()) {
    pathname = override_filename;
  }
#endif  // P3D_PLUGIN_P3D_PLUGIN

  bool trusted_environment = !_enable_security;

  Filename contents_filename = Filename(Filename::from_os_specific(_root_dir), "contents.xml");
  if (!load_plugin(pathname, contents_filename.to_os_specific(),
                   _host_url, _verify_contents, _this_platform, _log_dirname,
                   _log_basename, trusted_environment, _console_environment,
                   _root_dir, "", cerr)) {
    cerr << "Unable to launch core API in " << pathname << "\n";
    return false;
  }

  // Successfully loaded.
#ifdef PANDA_OFFICIAL_VERSION
  static const bool official = true;
#else
  static const bool official = false;
#endif

  // Format the coreapi_timestamp as a string, for passing as a
  // parameter.
  ostringstream stream;
  stream << _coreapi_dll.get_timestamp();
  string coreapi_timestamp = stream.str();

  P3D_set_plugin_version_ptr(P3D_PLUGIN_MAJOR_VERSION, P3D_PLUGIN_MINOR_VERSION,
                             P3D_PLUGIN_SEQUENCE_VERSION, official,
                             PANDA_DISTRIBUTOR,
                             _host_url.c_str(), coreapi_timestamp.c_str(),
                             _coreapi_set_ver.c_str());

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Panda3D::usage
//       Access: Protected
//  Description: Reports the available command-line options.
////////////////////////////////////////////////////////////////////
void Panda3D::
usage() {
  cerr
    << "\nThis is panda3d version " 
    << P3D_PLUGIN_MAJOR_VERSION << "."
    << P3D_PLUGIN_MINOR_VERSION << "."
    << P3D_PLUGIN_SEQUENCE_VERSION;
#ifndef PANDA_OFFICIAL_VERSION
  cerr << "c";
#endif

  cerr
    << "\n\nUsage:\n"
    << "   panda3d [opts] file.p3d [args]\n"
    << "   panda3d -m [opts] file_a.p3d file_b.p3d [file_c.p3d ...]\n\n"
  
    << "This program is used to execute a Panda3D application bundle stored\n"
    << "in a .p3d file.  In the first form, without the -m option, it\n"
    << "executes one application; remaining arguments following the\n"
    << "application name are passed into the application.  In the second\n"
    << "form, with the -m option, it can execute multiple applications\n"
    << "simultaneously, though in this form arguments cannot be passed into\n"
    << "the applications.\n\n"

    << "Options:\n\n"

    << "  -m\n"
    << "    Indicates that multiple application filenames will be passed on\n"
    << "    the command line.  All applications will be run at the same\n"
    << "    time, but additional arguments may not be passed to any of the\n"
    << "    applictions.\n\n"

    << "  -t token=value\n"
    << "    Defines a web token or parameter to pass to the application(s).\n"
    << "    This simulates a <param> entry in an <object> tag.\n\n"

    << "  -w [toplevel|embedded|fullscreen|hidden]\n"
    << "    Specify the type of graphic window to create.  If you specify\n"
    << "    \"embedded\", a new window is created to be the parent.\n\n"

    << "  -s width,height\n"
    << "    Specify the size of the graphic window.\n\n"

    << "  -o x,y\n"
    << "    Specify the position (origin) of the graphic window on the\n"
    << "    screen, or on the parent window.  If you specify -1,-1\n"
    << "    the default position will be used, and a value of -2,-2\n"
    << "    means that the window will be centered on the screen.\n\n"

    << "  -l log_dirname\n"
    << "    Specify the full path to the directory in which log files are\n"
    << "    to be written.  If this is not specified, the default is to send\n"
    << "    the application output to the console.\n\n"

    << "  -n\n"
    << "    Allow a network connect to the Panda3D download server, to check\n"
    << "    if a new version is available (but only if the current version\n"
    << "    appears to be out-of-date).  The default behavior, if both -n\n"
    << "    and -f are omitted, is not to contact the server at all, unless\n"
    << "    the local contents do not exist or cannot be read.\n\n"

    << "  -f\n"
    << "    Force an initial contact of the Panda3D download server, even\n"
    << "    if the local contents appear to be current.  This is mainly\n"
    << "    useful when testing local republishes.\n\n"

    << "  -i\n"
    << "    Runs the application interactively.  This requires that the application\n"
    << "    was built with -D on the packp3d command line.  If so, this option will\n"
    << "    create an interactive Python prompt after the application has loaded.\n"
    << "    It will also retain the PYTHONPATH environment variable from the user's\n"
    << "    environment, allowing Python files on disk to shadow the same-named\n"
    << "    Python files within the p3d file, for rapid iteration on the Python\n"
    << "    code.\n\n"

    << "  -S\n"
    << "    Runs the application with security enabled, as if it were embedded in\n"
    << "    a web page.\n\n"

    << "  -u url\n"

    << "    Specify the URL of the Panda3D download server.  This is the host\n"
    << "    from which the plugin itself will be downloaded if necessary.  The\n"
    << "    default is \"" << _host_url << "\" .\n\n"

    << "  -M super_mirror_url\n"
    << "    Specifies the \"super mirror\" URL, the special URL that is consulted\n"
    << "    first before downloading any package file referenced by a p3d file.\n"
    << "    This is primarily intended to support pre-installing a downloadable\n"
    << "    Panda3D tree on the local machine, to allow p3d applications to\n"
    << "    execute without requiring an internet connection.\n\n"

    << "  -p platform\n"
    << "    Specify the platform to masquerade as.  The default is \""
    << DTOOL_PLATFORM << "\" .\n\n"

    << "  -V\n"
    << "    Output only the plugin version string and exit immediately.\n\n"

    << "  -U\n"
    << "    Output only the plugin host URL and exit immediately.\n\n"

    << "  -P\n"
    << "    Output only the plugin platform string and exit immediately.\n\n";
}
