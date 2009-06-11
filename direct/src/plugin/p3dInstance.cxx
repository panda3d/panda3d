// Filename: p3dInstance.cxx
// Created by:  drose (29May09)
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

#include "p3dInstance.h"
#include "p3dInstanceManager.h"

#include <sstream>

int P3DInstance::_next_instance_id = 0;

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DInstance::
P3DInstance(P3D_request_ready_func *func,
            const string &p3d_filename, 
            P3D_window_type window_type,
            int win_x, int win_y,
            int win_width, int win_height,
            P3D_window_handle parent_window,
            const P3D_token tokens[], size_t num_tokens) :
  _func(func),
  _p3d_filename(p3d_filename),
  _window_type(window_type),
  _win_x(win_x), _win_y(win_y),
  _win_width(win_width), _win_height(win_height),
  _parent_window(parent_window)
{
  fill_tokens(tokens, num_tokens);

  _instance_id = _next_instance_id;
  ++_next_instance_id;

  INIT_LOCK(_request_lock);

  // For the moment, all sessions will be unique.
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  ostringstream strm;
  strm << inst_mgr->get_unique_session_index();
  _session_key = strm.str();

  _python_version = "python24";

  _session = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DInstance::
~P3DInstance() {
  assert(_session == NULL);

  DESTROY_LOCK(_request_lock);

  // TODO: empty _pending_requests queue.
}


////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::has_property
//       Access: Public
//  Description: Returns true if the instance has the named property,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool P3DInstance::
has_property(const string &property_name) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::get_property
//       Access: Public
//  Description: Returns the value of the named property, or empty
//               string if there is no such property.  Properties are
//               created by the script run within the instance; they
//               are used for communicating between scripting
//               languages (for instance, communication between the
//               Python-based Panda application, and the Javascript on
//               the containing web page).
////////////////////////////////////////////////////////////////////
string P3DInstance::
get_property(const string &property_name) const {
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::set_property
//       Access: Public
//  Description: Changes the value of the named property.  It is an
//               error to call this on a property that does not
//               already exist.
////////////////////////////////////////////////////////////////////
void P3DInstance::
set_property(const string &property_name, const string &value) {
}


////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::has_request
//       Access: Public
//  Description: Returns true if the instance has any pending requests
//               at the time of this call, false otherwise.
////////////////////////////////////////////////////////////////////
bool P3DInstance::
has_request() {
  ACQUIRE_LOCK(_request_lock);
  bool any_requests = !_pending_requests.empty();
  RELEASE_LOCK(_request_lock);
  return any_requests;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::get_request
//       Access: Public
//  Description: Returns a newly-allocated P3D_request corresponding
//               to the pending request for the host, or NULL if there
//               is no pending request.  If the return value is
//               non-NULL, it should eventually be passed back to
//               finish_request() for cleanup.
////////////////////////////////////////////////////////////////////
P3D_request *P3DInstance::
get_request() {
  P3D_request *result = NULL;
  ACQUIRE_LOCK(_request_lock);
  if (!_pending_requests.empty()) {
    result = _pending_requests.front();
    _pending_requests.pop_front();
  }
  RELEASE_LOCK(_request_lock);

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::add_request
//       Access: Public
//  Description: May be called in any thread to add a new P3D_request
//               to the pending_request queue for this instance.
////////////////////////////////////////////////////////////////////
void P3DInstance::
add_request(P3D_request *request) {
  cerr << "adding a request\n";
  assert(request->_instance == this);

  ACQUIRE_LOCK(_request_lock);
  _pending_requests.push_back(request);
  RELEASE_LOCK(_request_lock);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  inst_mgr->signal_request_ready();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::finish_request
//       Access: Public
//  Description: Deallocates a previously-returned request from
//               get_request().  If handled is true, the request has
//               been handled by the host; otherwise, it has been
//               ignored.
////////////////////////////////////////////////////////////////////
void P3DInstance::
finish_request(P3D_request *request, bool handled) {
  assert(request != NULL);

  // TODO.  Delete sub-pieces more aggressively.  Deal with handled flag.
  delete request;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::feed_url_stream
//       Access: Public
//  Description: Called by the host in response to a get_url or
//               post_url request, this sends the data retrieved from
//               the requested URL, a piece at a time.
////////////////////////////////////////////////////////////////////
void P3DInstance::
feed_url_stream(int unique_id,
                P3D_result_code result_code,
                int http_status_code, 
                size_t total_expected_data,
                const unsigned char *this_data, 
                size_t this_data_size) {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::lookup_token
//       Access: Public
//  Description: Returns the value associated with the first
//               appearance of the named token, or empty string if the
//               token does not appear.
////////////////////////////////////////////////////////////////////
string P3DInstance::
lookup_token(const string &keyword) const {
  Tokens::const_iterator ti;
  for (ti = _tokens.begin(); ti != _tokens.end(); ++ti) {
    if ((*ti)._keyword == keyword) {
      return (*ti)._value;
    }
  }

  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::make_xml
//       Access: Public
//  Description: Returns a newly-allocated XML structure that
//               corresponds to the data within this instance.
////////////////////////////////////////////////////////////////////
TiXmlElement *P3DInstance::
make_xml() {
  TiXmlElement *xinstance = new TiXmlElement("instance");
  xinstance->SetAttribute("id", _instance_id);
  xinstance->SetAttribute("p3d_filename", _p3d_filename);

  switch (_window_type) {
  case P3D_WT_embedded:
    xinstance->SetAttribute("window_type", "embedded");
    xinstance->SetAttribute("win_x", _win_x);
    xinstance->SetAttribute("win_y", _win_y);
    xinstance->SetAttribute("win_width", _win_width);
    xinstance->SetAttribute("win_height", _win_height);
#ifdef _WIN32
    xinstance->SetAttribute("parent_hwnd", (int)_parent_window._hwnd);
#endif
    break;

  case P3D_WT_toplevel:
    xinstance->SetAttribute("window_type", "toplevel");
    xinstance->SetAttribute("win_x", _win_x);
    xinstance->SetAttribute("win_y", _win_y);
    xinstance->SetAttribute("win_width", _win_width);
    xinstance->SetAttribute("win_height", _win_height);
    break;

  case P3D_WT_fullscreen:
    xinstance->SetAttribute("window_type", "fullscreen");
    xinstance->SetAttribute("win_width", _win_width);
    xinstance->SetAttribute("win_height", _win_height);
    break;

  case P3D_WT_hidden:
    xinstance->SetAttribute("window_type", "hidden");
    break;
  }

  return xinstance;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::fill_tokens
//       Access: Private
//  Description: Copies the C-style tokens array into the internal
//               C++-style _tokens vector.
////////////////////////////////////////////////////////////////////
void P3DInstance::
fill_tokens(const P3D_token tokens[], size_t num_tokens) {
  for (size_t i = 0; i < num_tokens; ++i) {
    Token token;
    if (tokens[i]._keyword != NULL) {
      token._keyword = tokens[i]._keyword;
    }
    if (tokens[i]._value != NULL) {
      token._value = tokens[i]._value;
    }
    _tokens.push_back(token);
  }
}

