// Filename: config_display.cxx
// Created by:  drose (06Oct99)
// 
////////////////////////////////////////////////////////////////////

#include "config_display.h"
#include "graphicsStateGuardian.h"
#include "savedFrameBuffer.h"
#include "graphicsPipe.h"
#include "interactiveGraphicsPipe.h"
#include "noninteractiveGraphicsPipe.h"
#include "graphicsWindow.h"
#include "graphicsChannel.h"
#include "hardwareChannel.h"
#include "textureContext.h"

Configure(config_display);
NotifyCategoryDef(display, "");
NotifyCategoryDef(gsg, display_cat);

static Config::ConfigTable::Symbol *disp;
static Config::ConfigTable::Symbol *guard;
static Config::ConfigTable::Symbol *preferred_pipe;
static Config::ConfigTable::Symbol *preferred_window;
static Config::ConfigTable::Symbol *preferred_gsg;

ConfigureFn(config_display) {
  GraphicsStateGuardian::init_type();
  GraphicsStateGuardian::GsgParam::init_type();
  GraphicsStateGuardian::GsgWindow::init_type();
  SavedFrameBuffer::init_type();
  GraphicsPipe::init_type();
  GraphicsPipe::PipeParam::init_type();
  GraphicsPipe::PipeSpec::init_type();
  InteractiveGraphicsPipe::init_type();
  NoninteractiveGraphicsPipe::init_type();
  GraphicsWindow::init_type();
  GraphicsWindow::WindowParam::init_type();
  GraphicsWindow::WindowProps::init_type();
  GraphicsWindow::WindowPipe::init_type();
  GraphicsChannel::init_type();
  HardwareChannel::init_type();
  TextureContext::init_type();

  disp = new Config::ConfigTable::Symbol;
  guard = new Config::ConfigTable::Symbol;
  preferred_pipe = new Config::ConfigTable::Symbol;
  preferred_window = new Config::ConfigTable::Symbol;
  preferred_gsg = new Config::ConfigTable::Symbol;

  config_display.GetAll("load-display", *disp);
  config_display.GetAll("load-gsg", *guard);

  config_display.GetAll("preferred-pipe", *preferred_pipe);
  config_display.GetAll("preferred-window", *preferred_window);
  config_display.GetAll("preferred-gsg", *preferred_gsg);
}

const string pipe_spec_machine = config_display.GetString("pipe-machine", "");
const string pipe_spec_filename = config_display.GetString("pipe-filename",
							   "outfile-%03f.rib");
const int pipe_spec_pipe_number = config_display.GetInt("pipe-number", -1);
const bool pipe_spec_is_file = config_display.Defined("pipe-filename")
                               || config_display.GetBool("pipe-file", false);
const bool pipe_spec_is_remote = config_display.Defined("pipe-machine")
                                 || config_display.GetBool("pipe-remote",
							   false);


Config::ConfigTable::Symbol::iterator pipe_modules_begin(void) {
  return disp->begin();
}

Config::ConfigTable::Symbol::iterator pipe_modules_end(void) {
  return disp->end();
}

Config::ConfigTable::Symbol::iterator gsg_modules_begin(void) {
  return guard->begin();
}

Config::ConfigTable::Symbol::iterator gsg_modules_end(void) {
  return guard->end();
}


Config::ConfigTable::Symbol::iterator preferred_pipe_begin(void) {
  return preferred_pipe->begin();
}

Config::ConfigTable::Symbol::iterator preferred_pipe_end(void) {
  return preferred_pipe->end();
}

Config::ConfigTable::Symbol::iterator preferred_window_begin(void) {
  return preferred_window->begin();
}

Config::ConfigTable::Symbol::iterator preferred_window_end(void) {
  return preferred_window->end();
}

Config::ConfigTable::Symbol::iterator preferred_gsg_begin(void) {
  return preferred_gsg->begin();
}

Config::ConfigTable::Symbol::iterator preferred_gsg_end(void) {
  return preferred_gsg->end();
}
