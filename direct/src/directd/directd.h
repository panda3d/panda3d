/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file directd.h
 * @author skyler
 * @date 2002-04-08
 */

#include <process.h>
#include "pandabase.h"
#include "directsymbols.h"
#include "queuedConnectionManager.h"
#include "queuedConnectionReader.h"
#include "connectionWriter.h"
#include "queuedConnectionListener.h"
#include <windows.h>


#ifdef CPPPARSER //[
// hack for interrogate
typedef int intptr_t;
typedef int HANDLE;
#endif //]


/**
 * DirectD is a client/server app for starting panda/direct.
 *
 * Usage: Start a directd server on each of the machines you which to start
 * panda on.
 *
 * Start a directd client on the controlling machine or import ShowBaseGlobal
 * with the xxxxx flag in your Configrc.  The client will connect each of the
 * servers in the xxxxx list in your Configrc.
 *
 * There are two API groups in this class, they are:
 *
 * listen_to() client_ready() or tell_server() wait_for_servers()
 * server_ready()
 *
 * and:
 *
 * connect_to() send_command() disconnect_from()
 *
 * The second group was from a more general implementation of DirectD.  The
 * first group summarizes the main intents of DirectD. Both groups are
 * presented in order chronologically by their intended usage.  The first
 * group will probably provide everthing needed for DirectD.
 */
class EXPCL_DIRECT_DIRECTD DirectD {
PUBLISHED:
  DirectD();
  ~DirectD();

/**
 * Call listen_to in the server.  port is a rendezvous port.
 *
 * backlog refers to how many connections can queue up before you handle them.
 * Consider setting backlog to the count you send to wait_for_servers(); or
 * higher.
 */
  void listen_to(int port, int backlog=8);

/**
 * Call this function from the client when import ShowbaseGlobal is nearly
 * finished.  cmd: a cli command that will be executed on the remote machine.
 * A new connection will be created and closed.  If you want to send more than
 * one command, you should use connect_to(), send_command(), and
 * disconnect_from().
 */
  int client_ready(const std::string& server_host, int port, const std::string& cmd);

/**
 * Tell the server to do the command cmd.  cmd is one of the following:
 * "k[<n>]"    Kill the most recent application started with client_ready() or
 * "!". Or kill the nth most recent or 'a' for All.  E.g.  "k", "k0", "k2",
 * "ka". "q"         Tell the server to quit.  "!cmd"      Exectue the cmd on
 * the server (this is a dos shell command; if you want a bash command,
 * include bash in the command e.g.  "!bash pwd").  When you call
 * client_ready(), it prefixes "!" for you.  A new connection will be created
 * and closed.
 */
  int tell_server(const std::string& server_host, int port, const std::string& cmd);

/**
 * Call this function from the client after calling <count> client_ready()
 * calls.
 *
 * Call listen_to(port) prior to calling wait_for_servers() (or better yet,
 * prior to calling client_ready()).
 *
 * timeout_ms defaults to two minutes.
 */
  bool wait_for_servers(int count, int timeout_ms=2*60*1000);

/**
 * Call this function from the server when import ShowbaseGlobal is nearly
 * finished.
 */
  int server_ready(const std::string& client_host, int port);

/**
 * Call connect_to from client for each server.  returns the port number of
 * the connection (which is different from the rendezvous port used in the
 * second argument).  The return value can be used for the port arguemnt in
 * disconnect_from().
 */
  int connect_to(const std::string& server_host, int port);

/**
 * This is the counterpart to connect_to().  Pass the same server_host as for
 * connect_to(), but pass the return value from connect_to() for the port, not
 * the port passed to connect_to().
 */
  void disconnect_from(const std::string& server_host, int port);

/**
 * Send the same command string to all current connections.
 */
  void send_command(const std::string& cmd);

protected:
  void start_app(const std::string& cmd);
  void kill_app(int index);
  void kill_all();
  virtual void handle_command(const std::string& cmd);
  void handle_datagram(NetDatagram& datagram);
  void send_one_message(const std::string& host_name,
      int port, const std::string& message);

  QueuedConnectionManager _cm;
  QueuedConnectionReader _reader;
  ConnectionWriter _writer;
  QueuedConnectionListener _listener;

  // Start of old stuff: This is used to switch to the original method of
  // starting applications.  It can be used on old systems that don't support
  // job objects.  Eventually this stuff should be removed.
  bool _useOldStuff;
  typedef pvector< long /*intptr_t*/ > PidStack;
  PidStack _pids;
  // End of old stuff

  typedef pset< PT(Connection) > ConnectionSet;
  ConnectionSet _connections;
  HANDLE _jobObject;
  bool _shutdown;

  void check_for_new_clients();
  void check_for_datagrams();
  void check_for_lost_connection();
};
