/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaConversionServer.cxx
 * @author cbrunner
 * @author Derzsi Dániel
 * @date 2020-10-01
 */

#ifdef _WIN32
#include <direct.h>  // for chdir
#endif

#include "mayaConversionServer.h"
#include "mayaToEgg.h"
#include "eggToMaya.h"
#include "virtualFileSystem.h"
#include "filename.h"

/**
 * Initializes the Maya conversion server.
 */
MayaConversionServer::
MayaConversionServer() : _qListener(&_qManager, 0), _qReader(&_qManager, 0),
  _cWriter(&_qManager, 0) {
}

/**
 * Checks for any network activity and handles it, if appropriate, and then
 * returns.  This must be called periodically
 */
void MayaConversionServer::
poll() {
  // Listen for new connections
  _qListener.poll();

  // If we have a new connection from a client create a new connection pointer
  // and add it to the reader list
  if (_qListener.new_connection_available()) {
    PT(Connection) rendezvous;
    PT(Connection) connection;
    NetAddress address;

    if (_qListener.get_new_connection(rendezvous, address, connection)) {
      _qReader.add_connection(connection);
      _clients.insert(connection);
    }
  }

  // Check for reset clients
  if (_qManager.reset_connection_available()) {
    PT(Connection) connection;

    if (_qManager.get_reset_connection(connection)) {
      _clients.erase(connection);
      _qManager.close_connection(connection);
    }
  }

  // Poll the readers (created above) and if they have data process it
  _qReader.poll();

  if (_qReader.data_available()) {
    // Grab the incoming data and unpack it
    NetDatagram datagram;

    if (_qReader.get_data(datagram)) {
      DatagramIterator data(datagram);

      // First data should be the "argc" (argument count) from the client
      int argc = data.get_uint8();

      // Now we have to get clever because the rest of the data comes as
      // strings and parse_command_line() expects arguments of the standard
      // argc, argv*[] variety.  First, we need a string vector to hold all
      // the strings from the datagram.  We also need a char * array to keep
      // track of all the pointers we're gonna malloc.  Needed later for
      // cleanup.
      vector_string vargv;
      std::vector<char *> buffers;

      // Get the strings from the datagram and put them into the string vector
      for (int i = 0; i < argc; i++) {
        vargv.push_back(data.get_string().c_str());
      }

      // Last string is the current directory the client was run from.  Not
      // part of the argument list, but we still need it
      std::string cwd = data.get_string();

      // We allocate some memory to hold the pointers to the pointers we're
      // going to pass in to parse_command_line().
      char **cargv = (char**) malloc(sizeof(char**) * argc);

      // Loop through the string arguments we got from the datagram and
      // convert them to const char *'s.  parse_command_line() expects char
      // *'s, so we have to copy these const versions into fresh char *, since
      // there is no casting from const char * to char *.
      for (int i = 0; i < argc; i++) {
        // string to const char *
        const char *cptr = vargv[i].c_str();
        // memory allocated for a new char * of size of the string
        char *buffer = (char *) malloc(vargv[i].capacity() + 1);

        // Copy the const char * to the char *
        strcpy(buffer, cptr);
        // put this into the arry we defined above.  This is what will
        // eventually be passed to parse_command_line()
        cargv[i] = buffer;
        // keep track of the pointers to the allocated memory for cleanup
        // later
        buffers.push_back(buffer);
      }

      // Change to the client's current dir
#ifdef _WIN64
      _chdir(cwd.c_str());
#else
      chdir(cwd.c_str());
#endif

      // Change the VirtualFileSystem's current dir as well
      VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
      vfs->chdir(Filename::from_os_specific(cwd));

      // Next, we'll need to read the conversion type.
      // Are we converting from egg to maya or from maya to egg?
      int conversion_type = data.get_uint8();
      bool converted = false;

      switch (conversion_type) {
        case ConversionType::CT_maya_to_egg:
        {
          MayaToEgg egg;

          // Pass in the 'new' argc and argv we got from the client
          if (egg.parse_command_line(argc, cargv, false) == ProgramBase::ExitCode::EC_not_exited) {
            // Actually run the damn thing
            converted = egg.run();
          }

          break;
        }
        case ConversionType::CT_egg_to_maya:
        {
          EggToMaya maya;
          maya.set_exit_on_failure(false);

          // Pass in the 'new' argc and argv we got from the client
          if (maya.parse_command_line(argc, cargv, false) == ProgramBase::ExitCode::EC_not_exited) {
            // Actually run the damn thing
            converted = maya.run();
          }

          break;
        }
      }

      // Let's send the result back to the client
      NetDatagram response;

      // The first and only part of the response is the success value
      response.add_bool(converted);

      // Send the response
      if (!_cWriter.send(response, datagram.get_connection())) {
        // Looks like we couldn't send the response
        nout << "Could not send response to the client.\n";
      }

      std::cout.flush();

      // Cleanup first, release the string vector
      vargv.clear();
      // No, iterate through the char * vector and cleanup the malloc'd
      // pointers
      std::vector<char *>::iterator vi;
      for ( vi = buffers.begin() ; vi != buffers.end(); vi++) {
        free(*vi);
      }
      // Clean up the malloc'd pointer pointer
      free(cargv);
    } // qReader.get_data

    Clients::iterator ci;
    for (ci = _clients.begin(); ci != _clients.end(); ++ci) {
      _qManager.close_connection(*ci);
    }
  } // qReader.data_available
} // poll

/**
 * Blocks the current thread and listens to conversion requests.
 */
void MayaConversionServer::
listen() {
  // Open a rendezvous port for receiving new connections from the client
  PT(Connection) rend = _qManager.open_TCP_server_rendezvous(4242, 100);

  if (rend.is_null()) {
    nout << "Port opening failed!\n";
    return;
  }

  nout << "Server opened on port 4242, waiting for requests...\n";

  // Add this connection to the listeners list
  _qListener.add_connection(rend);

  // Main loop.  Keep polling for connections, but don't eat up all the CPU.
  while (true) {
    this->poll();
    Thread::force_yield();
  }
}
