/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dSplashWindow.cxx
 * @author drose
 * @date 2009-06-17
 */

#include "p3dSplashWindow.h"
#include "wstring_encode.h"

// We use the public domain stb_image library for loading images.  Define the
// stb_image implementation.  We only use it in this unit.
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using std::max;
using std::min;
using std::string;

// The number of pixels to move the block per byte downloaded, when we don't
// know the actual file size we're downloading.
const double P3DSplashWindow::_unknown_progress_rate = 1.0 / 4096;

/**
 * By the time the SplashWindow is created, the instance has received both its
 * fparams and its wparams.  Copy them both into this class for reference.
 */
P3DSplashWindow::
P3DSplashWindow(P3DInstance *inst, bool make_visible) :
  _inst(inst),
  _fparams(inst->get_fparams()),
  _wparams(inst->get_wparams())
{
  _visible = make_visible;
  _fgcolor_r = 0x00;
  _fgcolor_g = 0x00;
  _fgcolor_b = 0x00;
  _bgcolor_r = 0xff;
  _bgcolor_g = 0xff;
  _bgcolor_b = 0xff;
  _barcolor_r = 0x6c;
  _barcolor_g = 0xa5;
  _barcolor_b = 0xe0;
  _bar_bgcolor_r = _bgcolor_r;
  _bar_bgcolor_g = _bgcolor_g;
  _bar_bgcolor_b = _bgcolor_b;
  _bar_border = 1;
  _bar_bottom = 24;
  _bar_width = 398;
  _bar_height = 22;
  _bar_width_ratio = 0.6;
  _bar_height_ratio = 0.1;
  _font_family = "Helvetica";
  _font_size = 12;
  _font_style = FS_normal;
  _font_weight = FW_normal;
  _button_width = 0;
  _button_height = 0;
  _button_x = 0;
  _button_y = 0;
  _button_active = false;
  _mouse_x = -1;
  _mouse_y = -1;
  _mouse_down = false;
  _button_depressed = false;
  _bstate = BS_hidden;
}

/**
 *
 */
P3DSplashWindow::
~P3DSplashWindow() {
}

/**
 * Changes the window parameters, e.g.  to resize or reposition the window; or
 * sets the parameters for the first time, creating the initial window.
 */
void P3DSplashWindow::
set_wparams(const P3DWindowParams &wparams) {
  _wparams = wparams;
  _win_width = _wparams.get_win_width();
  _win_height = _wparams.get_win_height();
}

/**
 * Makes the splash window visible or invisible, so as not to compete with the
 * embedded Panda window in the same space.
 */
void P3DSplashWindow::
set_visible(bool visible) {
  nout << "P3DSplashWindow::set_visible(" << visible << ")\n";
  _visible = visible;
}

/**
 * Specifies the name of a JPEG or PNG image file that is displayed in the
 * center of the splash window.
 *
 * image_placement defines the specific context in which this particular image
 * is displayed.  It is similar to the P3DInstance's image_type, but it is a
 * more specific, lower-level usage.
 */
void P3DSplashWindow::
set_image_filename(const string &image_filename, ImagePlacement image_placement) {
}

/**
 * Specifies the color that is used to display the text above the loading bar.
 *
 * This may only be set before wparams is set.
 */
void P3DSplashWindow::
set_fgcolor(int r, int g, int b) {
  nout << "fgcolor " << r << ", " << g << ", " << b << "\n";
  _fgcolor_r = r;
  _fgcolor_g = g;
  _fgcolor_b = b;
}

/**
 * Specifies the solid color that is displayed behind the splash image, if
 * any, or before the splash image is loaded.
 *
 * This may only be set before wparams is set.
 */
void P3DSplashWindow::
set_bgcolor(int r, int g, int b) {
  nout << "bgcolor " << r << ", " << g << ", " << b << "\n";
  _bgcolor_r = r;
  _bgcolor_g = g;
  _bgcolor_b = b;
}

/**
 * Specifies the color that is used to fill the loading bar.
 *
 * This may only be set before wparams is set.
 */
void P3DSplashWindow::
set_barcolor(int r, int g, int b) {
  nout << "barcolor " << r << ", " << g << ", " << b << "\n";
  _barcolor_r = r;
  _barcolor_g = g;
  _barcolor_b = b;
}

/**
 * Specifies the solid color that is displayed behind the loading bar.
 *
 * This may only be set before wparams is set.
 */
void P3DSplashWindow::
set_bar_bgcolor(int r, int g, int b) {
  nout << "bar_bgcolor " << r << ", " << g << ", " << b << "\n";
  _bar_bgcolor_r = r;
  _bar_bgcolor_g = g;
  _bar_bgcolor_b = b;
}

/**
 * Sets the width in pixels of the border around the loading bar, or 0 not to
 * draw a bar at all.
 *
 * This may only be set before wparams is set.
 */
void P3DSplashWindow::
set_bar_border(int border) {
  nout << "bar_border " << border << "\n";
  _bar_border = border;
}

/**
 * Sets the amount of background pixels between the bottom edge of the window
 * and the bottom edge of the loading bar border.
 *
 * This may only be set before wparams is set.
 */
void P3DSplashWindow::
set_bar_bottom(int bottom) {
  nout << "bar_bottom " << bottom << "\n";
  _bar_bottom = bottom;
}

/**
 * Sets the width of the loading bar.  If percent is true, it is interpreted
 * as a percentage of the window width.  If false, it is interpreted as an
 * absolute width in pixels.
 *
 * This may only be set before wparams is set.
 */
void P3DSplashWindow::
set_bar_width(int width, bool percent) {
  nout << "bar_width " << width;
  if (percent) {
    nout << '%';
  }
  nout << '\n';

  if (percent) {
    _bar_width = -1;
    _bar_width_ratio = abs(width) / 100.0;
  } else {
    _bar_width = abs(width);
    _bar_width_ratio = 1.0;
  }
}

/**
 * Sets the height of the loading bar.  If percent is true, it is interpreted
 * as a percentage of the window height.  If false, it is interpreted as an
 * absolute height in pixels.
 *
 * This may only be set before wparams is set.
 */
void P3DSplashWindow::
set_bar_height(int height, bool percent) {
  nout << "bar_height " << height;
  if (percent) {
    nout << '%';
  }
  nout << '\n';

  if (percent) {
    _bar_height = -1;
    _bar_height_ratio = abs(height) / 100.0;
  } else {
    _bar_height = abs(height);
    _bar_height_ratio = 1.0;
  }
}

/**
 * Sets the font family of the text above the loading bar.
 *
 * This may only be set before wparams is set.
 */
void P3DSplashWindow::
set_font_family(const string &family) {
  nout << "font_family " << family << "\n";
  _font_family = family;
}

/**
 * Sets the font size in pixels of the text above the loading bar.  The
 * default value is 12.
 *
 * This may only be set before wparams is set.
 */
void P3DSplashWindow::
set_font_size(int size) {
  nout << "font_size " << size << "\n";
  _font_size = size;
}

/**
 * Sets the font style of the text above the loading bar.
 *
 * This may only be set before wparams is set.
 */
void P3DSplashWindow::
set_font_style(FontStyle style) {
  nout << "font_style " << style << "\n";
  _font_style = style;
}

/**
 * Sets the font weight of the text above the loading bar.  The default is
 * FW_normal.  It should be a multiple of 100 in the range 100-900.
 *
 * This may only be set before wparams is set.
 */
void P3DSplashWindow::
set_font_weight(int weight) {
  nout << "font_weight " << weight << "\n";
  _font_weight = weight;
}

/**
 * Specifies the text that is displayed above the install progress bar.
 */
void P3DSplashWindow::
set_install_label(const string &install_label) {
}

/**
 * Moves the install progress bar from 0.0 to 1.0.
 */
void P3DSplashWindow::
set_install_progress(double install_progress,
                     bool is_progress_known, size_t received_data) {
}

/**
 * Deals with the event callback from the OS window system.  Returns true if
 * the event is handled, false if ignored.
 */
bool P3DSplashWindow::
handle_event(const P3D_event_data &event) {
  return false;
}

/**
 * Sets whether the button should be visible and active (true) or invisible
 * and inactive (false).  If active, the button image will be displayed in the
 * window, and a click event will be generated when the user clicks the
 * button.
 */
void P3DSplashWindow::
set_button_active(bool flag) {
  _button_active = flag;

  // Now light up the button according to the current mouse position.
  set_mouse_data(_mouse_x, _mouse_y, _mouse_down);
}

/**
 * The Panda window is asking us to manage keyboard focus in proxy for it.
 * This is used on Vista, where the Panda window may be disallowed from
 * directly assigning itself keyboard focus.
 */
void P3DSplashWindow::
request_keyboard_focus() {
}

/**
 * Reads the image filename and sets image parameters width, height,
 * num_channels, and data.  Returns true on success, false on failure.
 */
bool P3DSplashWindow::
read_image_data(ImageData &image, string &data,
                const string &image_filename) {
  image._width = 0;
  image._height = 0;
  image._num_channels = 0;
  data.clear();

  if (image_filename.empty()) {
    return false;
  }

  nout << "Reading splash file image: " << image_filename << "\n";

  unsigned char *imgdata = stbi_load(image_filename.c_str(), &image._width,
                                     &image._height, &image._num_channels, 0);

  if (imgdata == nullptr) {
    nout << "Couldn't read splash file image: " << image_filename << "\n";
    const char *reason = stbi_failure_reason();
    if (reason != nullptr) {
      nout << "stbi_failure_reason: " << reason << "\n";
    }
    return false;
  }

  size_t data_size = image._width * image._height * image._num_channels;
  data.resize(data_size);
  memcpy(&data[0], imgdata, data_size);

  stbi_image_free(imgdata);
  return true;
}

/**
 * Given the window width and height, determine the rectangle in which to
 * place the progress bar.
 */
void P3DSplashWindow::
get_bar_placement(int &bar_x, int &bar_y,
                  int &bar_width, int &bar_height) {

  bar_width = (int)(_win_width * _bar_width_ratio);
  bar_height = (int)(_win_height * _bar_height_ratio);

  if (_bar_width >= 0) {
    bar_width = min(bar_width, _bar_width);
  }
  if (_bar_height >= 0) {
    bar_height = min(bar_height, _bar_height);
  }

  // Horizontally center the bar, and set it at a fixed distance from the
  // bottom edge of the splash window.
  bar_x = (_win_width - bar_width) / 2;
  bar_y = _win_height - _bar_bottom - _bar_border - bar_height;
}

/**
 * Specifies the image that contains the "ready" button image, which in turn
 * determines the clickable dimensions of the button within the window.
 */
void P3DSplashWindow::
set_button_range(const ImageData &image) {
  // The clickable area has a certain minimum size, even if it's a very small
  // image.
  _button_width = max(image._width, 64);
  _button_height = max(image._height, 64);

  // But it can't be larger than the window itself.
  _button_width = min(_button_width, _win_width);
  _button_height = min(_button_height, _win_height);

  // Compute the top-left corner of the button image in window coordinates.
  _button_x = (_win_width - _button_width) / 2;
  _button_y = (_win_height - _button_height) / 2;
}

/**
 * Intended to be called by the subclasses as the mouse is tracked through the
 * window, whether the button is currently active or not.  This updates the
 * internal state of the mouse pointer, and also (if the button is active)
 * updates the button state appropriately, and generates the click event when
 * the mouse button transitions from down to up over the button area.
 */
void P3DSplashWindow::
set_mouse_data(int mouse_x, int mouse_y, bool mouse_down) {
  ButtonState orig_bstate = _bstate;

  _mouse_x = mouse_x;
  _mouse_y = mouse_y;
  _mouse_down = mouse_down;

  ButtonState bstate = BS_hidden;
  bool click_detected = false;

  if (!_button_active) {
    // The button isn't active, so it's hidden, regardless of the mouse
    // position.
    bstate = BS_hidden;
  } else {
    // Is the mouse pointer within the button region?
    bool is_within = (_mouse_x >= _button_x && _mouse_x < _button_x + _button_width &&
                      _mouse_y >= _button_y && _mouse_y < _button_y + _button_height);
    if (is_within) {
      // The mouse is within the button region.  This means either click or
      // rollover state, according to the mouse button.
      if (_mouse_down) {
        // We only count it mouse-down if you've clicked down while over the
        // button (or you never released the button since the last time you
        // clicked down).  Clicking down somewhere else and dragging over the
        // button doesn't count.
        if (orig_bstate == BS_rollover || _button_depressed) {
          _button_depressed = true;
          bstate = BS_click;
        } else {
          // Otherwise, we're mousing over the button region with the button
          // held down.  Hmm, don't think the button should light up.
          bstate = BS_ready;
        }
      } else {
        _button_depressed = false;
        if (orig_bstate == BS_click) {
          // If we just transitioned from mouse down to mouse up, this means a
          // click.  And the button automatically hides itself after a
          // successful click.
          bstate = BS_hidden;
          _button_active = false;
          click_detected = true;
        } else {
          bstate = BS_rollover;
        }
      }
    } else {
      // The mouse is not within the button region.  This means ready state.
      bstate = BS_ready;
      if (!_mouse_down) {
        _button_depressed = false;
      }
    }
  }

  set_bstate(bstate);

  // If we detected a click operation in the above, make the callback here, at
  // the end of the method, after we have finished updating the button state.
  if (click_detected) {
    button_click_detected();
  }
}

/**
 * Called when a button click by the user is detected in set_mouse_data(),
 * this method simply turns around and notifies the instance.  It's a virtual
 * method to give subclasses a chance to redirect this message to the main
 * thread or process, as necessary.
 *
 * Note that this method might be called in a sub-thread.
 */
void P3DSplashWindow::
button_click_detected() {
  assert(_inst != nullptr);
  nout << "Play button clicked by user\n";
  _inst->splash_button_clicked_sub_thread();
}

/**
 * Changes the button state as the mouse interacts with it.
 */
void P3DSplashWindow::
set_bstate(ButtonState bstate) {
  if (_bstate != bstate) {
    _bstate = bstate;
    // If we've changed button states, we need to refresh the window.
    refresh();
  }
}

/**
 * Requests that the window will be repainted.  This may or may not be
 * implemented for a particular specialization of P3DSplashWindow.
 */
void P3DSplashWindow::
refresh() {
}
