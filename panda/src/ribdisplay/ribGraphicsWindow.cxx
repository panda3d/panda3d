// Filename: ribGraphicsWindow.cxx
// Created by:  drose (15Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "ribGraphicsWindow.h"
#include "ribGraphicsPipe.h"
#include "config_ribdisplay.h"

#include "ribGraphicsStateGuardian.h"
#include <ctype.h>

TypeHandle RIBGraphicsWindow::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
RIBGraphicsWindow::
RIBGraphicsWindow(GraphicsPipe *pipe) : GraphicsWindow(pipe) {
  setup_window(pipe);
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
RIBGraphicsWindow::
RIBGraphicsWindow(GraphicsPipe *pipe,
                  const GraphicsWindow::Properties &props) :
  GraphicsWindow(pipe, props)
{
  setup_window(pipe);
}


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
RIBGraphicsWindow::
~RIBGraphicsWindow(void) {
  flush_file();
}


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::flush_file
//       Access: Public
//  Description: Finishes the RIB file currently being output and
//               closes it.
////////////////////////////////////////////////////////////////////
void RIBGraphicsWindow::
flush_file() {
  if (_file_begun) {
    end_file();
  }
}


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::begin_frame
//       Access: Public, Virtual
//  Description: Prepares to write a new frame to the RIB file.
////////////////////////////////////////////////////////////////////
void RIBGraphicsWindow::
begin_frame() {
  if (!_file_begun) {
    // Open a new RIB file if we need to.
    begin_file();
  }

  _file << "FrameBegin " << _frame_number << "\n";

  if (image_per_frame()) {
    // If we're writing out an image file for each frame, specify it
    // here, inside the Frame control group.
    _file << "  Display \"" << get_image_filename()
          << "\" \"file\" \"rgba\"\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::end_frame
//       Access: Public, Virtual
//  Description: Finalizes a frame.
////////////////////////////////////////////////////////////////////
void RIBGraphicsWindow::
end_frame() {
  _file << "FrameEnd\n";

  if (rib_per_frame()) {
    // If we're outputting a RIB file for each frame, close the file now.
    end_file();
  }

  GraphicsWindow::end_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::setup_window
//       Access: Protected
//  Description: Called by the constructor to initialize whatever
//               internal structures are necessary, given the
//               indicated pipe and requested properties.
////////////////////////////////////////////////////////////////////
void RIBGraphicsWindow::
setup_window(GraphicsPipe *pipe) {
  RIBGraphicsPipe *rp = DCAST(RIBGraphicsPipe, pipe);

  _file_begun = false;

  set_rib_filename_template(rp->get_file_name());
  set_image_filename_template("");

  make_gsg();
}


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::begin_file
//       Access: Protected
//  Description: Called internally when a new RIB file needs to be
//               created and its suitable headers written.
////////////////////////////////////////////////////////////////////
void RIBGraphicsWindow::
begin_file() {
  assert(!_file_begun);
  RIBGraphicsStateGuardian *rgsg = DCAST(RIBGraphicsStateGuardian, _gsg);

  _file_begun = true;
  _current_rib_filename = Filename::text_filename(get_rib_filename());
  assert(!_current_rib_filename.empty());

  _current_rib_filename.open_write(_file);
  rgsg->reset_file(_file);

  if (!_file) {
    cerr << "Unable to write to " << _current_rib_filename << "\n";
    return;
  }

  _file <<
    "#\n"
    "# RIB file " << _current_rib_filename << "\n"
    "#\n";

  // World set-up
  _file <<
    //    "TextureCoordinates 0 1  1 1  0 0  1 0\n"
    "Option \"searchpath\" \"shader\" \"shaders:@\"\n";

  // A default ambient light for when lighting is off
  _file <<
    "LightSource \"ambientlight\" 0 \"intensity\" 1 \"lightcolor\" [ 1 1 1 ]\n"
    "Illuminate 0 1\n";


  if (!get_image_filename_template().empty() &&
      !image_per_frame()) {
    // If we have an image filename, and it's the same file for all
    // frames, specify it outside the Frame control group.  Maybe the
    // renderer will be able to generate a multi-frame output file
    // somehow.
    _file << "Display \"" << get_image_filename()
          << "\" \"file\" \"rgb\"\n";
  }

}


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::end_file
//       Access: Protected
//  Description: Called internally to wrap up the current RIB file and
//               close it.
////////////////////////////////////////////////////////////////////
void RIBGraphicsWindow::
end_file() {
  assert(_file_begun);
  RIBGraphicsStateGuardian *rgsg = DCAST(RIBGraphicsStateGuardian, _gsg);
  rgsg->reset();

  _file_begun = false;
  _file.close();

  cerr << "Wrote " << _current_rib_filename << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::format_name
//       Access: Protected
//  Description: Given a filename template which contains symbols like
//               %f etc., replaces the symbols with the appropriate
//               values to generate an actual filename.
////////////////////////////////////////////////////////////////////
string RIBGraphicsWindow::
format_name(const string &name_template) const {
  string name;

  string::const_iterator ci;
  ci = name_template.begin();
  while (ci != name_template.end()) {
    if (*ci == '%') {
      ++ci;
      string::const_iterator pi = ci;
      while (ci != name_template.end() && isdigit(*ci)) {
        ++ci;
      }
      string format_spec(pi, ci);

      if (ci != name_template.end()) {
        switch (*ci) {
        case 'f':
          // %f : insert frame number
          name += format_integer(format_spec, _frame_number);
          break;

        case 't':
          // %t : insert window title
          name += format_string(format_spec, _props._title);
          break;

        case '%':
          // %% : insert percent sign
          name += '%';
          break;

        default:
          cerr << "Invalid filename template specification: %"
               << format_spec << *ci << "\n";
        }
        ++ci;
      } else {
        cerr << "Incomplete filename template specification: %"
             << format_spec << "\n";
      }
    } else {
      name += *ci;
      ++ci;
    }
  }

  return name;
}


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::format_integer
//       Access: Protected, Static
//  Description: Formats an integer according to a %d-like format
//               specification found in a filename template.
//               format_spec is the string between the percent sign
//               and the command letter (which might be empty, or
//               might contain a field width), and number is the value
//               to format.
////////////////////////////////////////////////////////////////////
string RIBGraphicsWindow::
format_integer(const string &format_spec, int number) {
  // Get the field width requirement.  We don't care if it begins with
  // a leading zero or not, since we always pad with zeroes.
  int width = atoi(format_spec.c_str());

  string result;
  string sign;

  // Is the number negative?
  if (number < 0) {
    sign = '-';
    number = -number;
  }

  // Now build the number from the least-significant digit up.  We
  // keep going until the width runs out or the number does, whichever
  // lasts longer.
  do {
    int digit = number % 10;
    number /= 10;

    result = (char)(digit + '0') + result;
    width--;
  } while (width > 0 || number != 0);

  return sign + result;
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::format_string
//       Access: Protected, Static
//  Description: Formats a string according to a %s-like format
//               specification found in a filename template.
//               format_spec is the string between the percent sign
//               and the command letter (which might be empty, or
//               might contain a field width), and str is the value
//               to format.
////////////////////////////////////////////////////////////////////
string RIBGraphicsWindow::
format_string(const string &format_spec, const string &str) {
  int width = atoi(format_spec.c_str());
  if (width <= str.length()) {
    return str;
  }

  // Now we have to pad the string out.
  string pad;

  for (int extra = width; extra < str.length(); extra++) {
    pad += '-';
  }
  return pad + str;
}


////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::format_string
//       Access: Protected, Static
//  Description: Scans a name template for the appearance of %f (or
//               some variant), and returns true if it appears, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool RIBGraphicsWindow::
check_per_frame(const string &name_template) {
  string::const_iterator ci;
  ci = name_template.begin();
  while (ci != name_template.end()) {
    if (*ci == '%') {
      ++ci;
      string::const_iterator pi = ci;
      while (ci != name_template.end() && isdigit(*ci)) {
        ++ci;
      }

      if (ci != name_template.end()) {
        if ((*ci) == 'f') {
          return true;
        }
        ++ci;
      }
    } else {
      ++ci;
    }
  }

  return false;
}

void RIBGraphicsWindow::make_current(void) {
}

void RIBGraphicsWindow::unmake_current(void) {
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::get_gsg_type
//       Access: Public, Virtual
//  Description: Returns the TypeHandle of the kind of GSG preferred
//               by this kind of window.
////////////////////////////////////////////////////////////////////
TypeHandle RIBGraphicsWindow::
get_gsg_type() const {
  return RIBGraphicsStateGuardian::get_class_type();
}

GraphicsWindow*
RIBGraphicsWindow::make_RibGraphicsWindow(const FactoryParams &params) {
  GraphicsWindow::WindowPipe *pipe_param;
  if (!get_param_into(pipe_param, params)) {
    ribdisplay_cat.error()
      << "No pipe specified for window creation!" << endl;
    return NULL;
  }

  GraphicsPipe *pipe = pipe_param->get_pipe();

  GraphicsWindow::WindowProps *props_param;
  if (!get_param_into(props_param, params)) {
    return new RIBGraphicsWindow(pipe);
  } else {
    return new RIBGraphicsWindow(pipe, props_param->get_properties());
  }
}

TypeHandle RIBGraphicsWindow::get_class_type(void) {
  return _type_handle;
}

void RIBGraphicsWindow::init_type(void) {
  GraphicsWindow::init_type();
  register_type(_type_handle, "RIBGraphicsWindow",
                GraphicsWindow::get_class_type());
}

TypeHandle RIBGraphicsWindow::get_type(void) const {
  return get_class_type();
}
