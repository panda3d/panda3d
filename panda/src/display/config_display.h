// Filename: config_display.h
// Created by:  drose (06Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_DISPLAY_H
#define CONFIG_DISPLAY_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>
#include <dconfig.h>

#include <string>
#include <vector>

NotifyCategoryDecl(display, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(gsg, EXPCL_PANDA, EXPTP_PANDA);

extern const string pipe_spec_machine;
extern const string pipe_spec_filename;
extern const int pipe_spec_pipe_number;
extern const bool pipe_spec_is_file;
extern const bool pipe_spec_is_remote;

extern const float gsg_clear_r;
extern const float gsg_clear_g;
extern const float gsg_clear_b;

extern Config::ConfigTable::Symbol::iterator pipe_modules_begin(void);
extern Config::ConfigTable::Symbol::iterator pipe_modules_end(void);
extern Config::ConfigTable::Symbol::iterator gsg_modules_begin(void);
extern Config::ConfigTable::Symbol::iterator gsg_modules_end(void);

extern Config::ConfigTable::Symbol::iterator preferred_pipe_begin();
extern Config::ConfigTable::Symbol::iterator preferred_pipe_end();
extern Config::ConfigTable::Symbol::iterator preferred_window_begin();
extern Config::ConfigTable::Symbol::iterator preferred_window_end();
extern Config::ConfigTable::Symbol::iterator preferred_gsg_begin();
extern Config::ConfigTable::Symbol::iterator preferred_gsg_end();

extern EXPCL_PANDA void init_libdisplay();

#endif /* CONFIG_DISPLAY_H */
