// Filename: config_char.h
// Created by:  drose (28Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_CHAR_H
#define CONFIG_CHAR_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>

// CPPParser can't handle token-pasting to a keyword.
#ifndef CPPPARSER
NotifyCategoryDecl(char, EXPCL_PANDA, EXPTP_PANDA);
#endif

#endif
