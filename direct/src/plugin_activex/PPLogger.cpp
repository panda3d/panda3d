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

std::ofstream PPLogger::m_logfile;
bool PPLogger::m_isOpen = false;

PPLogger::PPLogger( )
{
}

PPLogger::~PPLogger( )
{
}

int PPLogger::CreateNewFolder( const std::string& dirname ) 
{
    int error( 0 );
    if ( CreateDirectory( dirname.c_str( ), NULL ) != 0 ) 
    {
        // Success!
        return error;
    }
    // Failed.
    DWORD lastError = GetLastError( );
    if ( lastError == ERROR_ALREADY_EXISTS ) 
    {
        // Not really an error: the directory is already there.
        return error;
    }
    if ( lastError == ERROR_PATH_NOT_FOUND ) 
    {
        // We need to make the parent directory first.
        std::string parent = dirname;
        if ( !parent.empty() && CreateNewFolder( parent ) ) 
        {
            // Parent successfully created.  Try again to make the child.
            if ( CreateDirectory( dirname.c_str(), NULL ) != 0) 
            {
                // Got it!
                return error;
            }
            m_logfile << "Couldn't create " << dirname << "\n";
        }
    }
    return ( error = 1 );
}

int PPLogger::CreateNewFile(const std::string& dirname, const std::string& filename) 
{
    int error( 0 );
    std::string logfilename = dirname + filename;
    HANDLE file = CreateFile( logfilename.c_str(), GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    if ( file == INVALID_HANDLE_VALUE ) 
    {
        // Try to make the parent directory first.
        std::string parent = dirname;
        if ( !parent.empty( ) && CreateNewFolder( parent ) ) 
        {
            // Parent successfully created.  Try again to make the file.
            file = CreateFile( logfilename.c_str(), GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
        }
        if ( file == INVALID_HANDLE_VALUE ) 
        {
            m_logfile << "Couldn't create " << filename << "\n";
            return ( error = 1 );
        }
    }
    CloseHandle( file );
    return error;
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

    if (!log_basename.empty()) {
      std::string log_pathname = log_directory;
      log_pathname += log_basename;
      log_pathname += ".log";

      m_logfile.close();
      m_logfile.clear();
      m_logfile.open(log_pathname.c_str(), std::ios::out | std::ios::trunc);
      m_logfile.setf(std::ios::unitbuf);
    }

    // If we didn't have a logfile name compiled in, we throw away log
    // output by the simple expedient of never actually opening the
    // ofstream.
    m_isOpen = true;
  }
}
