// Filename: PPLogger.cpp
// Created by:  atrestman (14Sept09)
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

#include "stdafx.h"

#include "windows.h"
#include "PPLogger.h"
#include "mkdir_complete.h"
#include "wstring_encode.h"

std::ofstream PPLogger::m_logfile;
bool PPLogger::m_isOpen = false;

PPLogger::PPLogger( )
{
}

PPLogger::~PPLogger( )
{
}

void PPLogger::Open( const std::string &rootDir ) 
{
  if (!m_isOpen) {
    // Note that this logfile name may not be specified at runtime.  It
    // must be compiled in if it is specified at all.

    std::string log_directory;
  // Allow the developer to compile in the log directory.
#ifdef P3D_PLUGIN_LOG_DIRECTORY
    if (log_directory.empty()) {
      log_directory = P3D_PLUGIN_LOG_DIRECTORY;
    }
#endif
    
    // Failing that, we write logfiles to Panda3D/log.
    if (log_directory.empty()) {
      log_directory = rootDir + "/log";
    }
    mkdir_complete(log_directory, std::cerr);

    // Ensure that the log directory ends with a slash.
    if (!log_directory.empty() && log_directory[log_directory.size() - 1] != '/') {
#ifdef _WIN32
      if (log_directory[log_directory.size() - 1] != '\\')
#endif
        log_directory += "/";
    }
    
    // Construct the logfile pathname.
    
    std::string log_basename;
#ifdef P3D_PLUGIN_LOG_BASENAME1
    if (log_basename.empty()) {
      log_basename = P3D_PLUGIN_LOG_BASENAME1;
    }
#endif
    if (log_basename.empty()) {
      log_basename = "p3dplugin";
    }

    if (!log_basename.empty()) {
      std::string log_pathname = log_directory;
      log_pathname += log_basename;
      log_pathname += ".log";

      m_logfile.close();
      m_logfile.clear();
      wstring log_pathname_w;
      string_to_wstring(log_pathname_w, log_pathname);
      m_logfile.open(log_pathname_w.c_str(), std::ios::out | std::ios::trunc);
      m_logfile.setf(std::ios::unitbuf);
    }

    // If we didn't have a logfile name compiled in, we throw away log
    // output by the simple expedient of never actually opening the
    // ofstream.
    m_isOpen = true;
  }
}
