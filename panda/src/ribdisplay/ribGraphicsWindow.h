// Filename: ribGraphicsWindow.h
// Created by:  drose (15Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef RIBGRAPHICSWINDOW_H
#define RIBGRAPHICSWINDOW_H

////////////////////////////////////////////////////////////////////
//
// A RIBGraphicsWindow, instead of actually being a window on some
// raster display device, instead represents a file (or sequence of
// files) on disk that will be filled with a RIB scene description
// when the scene is "rendered" to the window.  This RIB may
// subsequently be used as input to a standalone renderer such as
// prman or BMRT to generate one or more images.
//
// The output RIB filename as specified to the window is actually a
// filename template, and may contain any of the following:
//
//  %f This stands for the current frame number.  The frame number
//     starts counting at zero when the window is created and
//     increments by one each time a frame is rendered.  An optional
//     field width may appear after the %, printf-style.
//
//  %t This stands for the window title.
//
//  %% This stands for a single percent sign.
//
// If the RIB filename contains %f, a separate RIB file is generated
// for each frame rendered; otherwise, all frames are written into a
// single multi-frame RIB file.
//
//
// The image filename may also be specified.  If specified, a
// "Display" command is written to the RIB file to output the image to
// the indicated filename.  If unspecified or empty, the "Display"
// command is not written, and the renderer chooses the output
// filename.  The image filename may also contain any of the %
// sequences described above.
//
////////////////////////////////////////////////////////////////////


#include "pandabase.h"

#include "graphicsWindow.h"
#include "filename.h"

class RIBGraphicsPipe;


////////////////////////////////////////////////////////////////////
//       Class : RIBGraphicsWindow
// Description : Represents a specific RIB file (or sequence of files)
//               that can be "rendered" to.  Rendering to a RIB file
//               means writing out a scene description to the file.
//
//               The filename of the RIB file is initially taken from
//               the name of the RIBGraphicsPipe used to create the
//               window, but it may subsequently be changed via
//               set_rib_filename_template().  The filename may
//               contain any of the % sequences described above.
////////////////////////////////////////////////////////////////////
class RIBGraphicsWindow : public GraphicsWindow {
    public:

  RIBGraphicsWindow(GraphicsPipe *pipe);
  RIBGraphicsWindow(GraphicsPipe *pipe,
                    const GraphicsWindow::Properties &props);
  virtual ~RIBGraphicsWindow(void);

  INLINE void set_rib_filename_template(const string &str);
  INLINE string get_rib_filename_template() const;
  INLINE string get_rib_filename() const;
  INLINE bool rib_per_frame() const;

  INLINE void set_image_filename_template(const string &str);
  INLINE string get_image_filename_template() const;
  INLINE string get_image_filename() const;
  INLINE bool image_per_frame() const;

  INLINE void set_texture_directory(const string &directory);
  INLINE string get_texture_directory() const;
  INLINE void set_texture_extension(const string &extension);
  INLINE string get_texture_extension() const;

  void flush_file();

  virtual void begin_frame();
  virtual void end_frame();

  virtual void make_current(void);
  virtual void unmake_current(void);

  virtual TypeHandle get_gsg_type() const;
  static GraphicsWindow* make_RibGraphicsWindow(const FactoryParams &params);

protected:
  void setup_window(GraphicsPipe *pipe);

  void begin_file();
  void end_file();

  string format_name(const string &name_template) const;
  static string format_integer(const string &format_spec, int number);
  static string format_string(const string &format_spec, const string &str);

  static bool check_per_frame(const string &name_template);

  string _rib_filename_template;
  string _image_filename_template;
  bool _rib_per_frame;
  bool _image_per_frame;

  bool _file_begun;
  Filename _current_rib_filename;
  ofstream _file;

public:

  static TypeHandle get_class_type(void);
  static void init_type(void);
  virtual TypeHandle get_type(void) const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:

  static TypeHandle _type_handle;
};

#include "ribGraphicsWindow.I"

#endif
