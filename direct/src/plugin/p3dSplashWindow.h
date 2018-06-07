/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dSplashWindow.h
 * @author drose
 * @date 2009-06-17
 */

#ifndef P3DSPLASHWINDOW_H
#define P3DSPLASHWINDOW_H

#include "p3d_plugin_common.h"
#include "p3dFileParams.h"
#include "p3dWindowParams.h"

class P3DInstance;

/**
 * This window is displayed temporarily, in place of an instance's actual
 * window, during the initial launch of an applet, and also during the initial
 * download of Panda3D code if necessary.
 *
 * This is the base implementation; it contains no specific code to open a
 * window.
 */
class P3DSplashWindow {
public:
  P3DSplashWindow(P3DInstance *inst, bool make_visible);
  virtual ~P3DSplashWindow();

  inline const P3DFileParams &get_fparams() const;

  virtual void set_wparams(const P3DWindowParams &wparams);
  inline const P3DWindowParams &get_wparams() const;

  virtual void set_visible(bool visible);
  inline bool get_visible() const;

  enum ImagePlacement {
    IP_background,
    IP_button_ready,
    IP_button_rollover,
    IP_button_click,
    IP_none
  };
  enum FontStyle {
    FS_normal,
    FS_italic,
    FS_oblique
  };
  enum FontWeight {
    FW_normal = 400,
    FW_medium = 500,
    FW_bold = 700,
    FW_black = 900
  };

  virtual void set_image_filename(const std::string &image_filename,
                                  ImagePlacement image_placement);
  void set_fgcolor(int r, int g, int b);
  void set_bgcolor(int r, int g, int b);
  void set_barcolor(int r, int g, int b);
  void set_bar_bgcolor(int r, int g, int b);
  void set_bar_border(int border);
  void set_bar_bottom(int bottom);
  void set_bar_width(int width, bool percent=false);
  void set_bar_height(int height, bool percent=false);
  void set_font_family(const std::string &family);
  void set_font_size(int size);
  void set_font_style(FontStyle style);
  void set_font_weight(int weight);
  virtual void set_install_label(const std::string &install_label);
  virtual void set_install_progress(double install_progress,
                                    bool is_progress_known, size_t received_data);

  virtual bool handle_event(const P3D_event_data &event);

  virtual void set_button_active(bool flag);
  virtual void request_keyboard_focus();

protected:
  // This ImageData base class provides minimal functionality for storing a
  // loaded image.  Most of the real meat of this class is provided by the
  // various subclasses.
  class ImageData {
  public:
    inline ImageData();
    int _width, _height, _num_channels;
  };

  bool read_image_data(ImageData &image, std::string &data,
                       const std::string &image_filename);
  void get_bar_placement(int &bar_x, int &bar_y,
                         int &bar_width, int &bar_height);
  void set_button_range(const ImageData &image);
  void set_mouse_data(int mouse_x, int mouse_y, bool mouse_down);

  // The current visual state of the button.
  enum ButtonState {
    BS_hidden,
    BS_ready,
    BS_rollover,
    BS_click,
  };

  virtual void button_click_detected();
  virtual void set_bstate(ButtonState bstate);
  virtual void refresh();

protected:
  P3DInstance *_inst;
  P3DFileParams _fparams;
  P3DWindowParams _wparams;
  int _win_width, _win_height;
  bool _visible;

  int _fgcolor_r, _fgcolor_g, _fgcolor_b;
  int _bgcolor_r, _bgcolor_g, _bgcolor_b;
  int _barcolor_r, _barcolor_g, _barcolor_b;
  int _bar_bgcolor_r, _bar_bgcolor_g, _bar_bgcolor_b;
  int _bar_border;

private:
  int _bar_bottom;
  int _bar_width, _bar_height;
  double _bar_width_ratio, _bar_height_ratio;

protected:
  std::string _font_family;
  int _font_size;
  FontStyle _font_style;
  int _font_weight;

  // The region of the window for accepting button clicks.
  int _button_width, _button_height;
  int _button_x, _button_y;
  bool _button_active;

  // Tracking the mouse pointer within the window, for the purposes of
  // clicking the button.
  int _mouse_x, _mouse_y;
  bool _mouse_down;
  bool _button_depressed;
  ButtonState _bstate;

  static const double _unknown_progress_rate;
};

#include "p3dSplashWindow.I"

#endif
