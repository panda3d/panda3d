// Filename: directd.cxx
// Created by:  skyler 2002.04.08
// Based on test_tcp_*.* by drose.
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

#include "directd.h"
/*#include "pandaFramework.h"
#include "queuedConnectionManager.h"*/

//#include <process.h>
//#include <Windows.h>
//#include "pandabase.h"

//#include "queuedConnectionManager.h"
#include "queuedConnectionListener.h"
#include "queuedConnectionReader.h"
#include "connectionWriter.h"
#include "netAddress.h"
#include "connection.h"
#include "datagramIterator.h"
#include "netDatagram.h"

#include "pset.h"



namespace {
  // The following is from an MSDN example:
  
  #define TA_FAILED 0
  #define TA_SUCCESS_CLEAN 1
  #define TA_SUCCESS_KILL 2
  #define TA_SUCCESS_16 3

  BOOL CALLBACK
  TerminateAppEnum(HWND hwnd, LPARAM lParam) {
    DWORD dwID;
    GetWindowThreadProcessId(hwnd, &dwID);
    if(dwID == (DWORD)lParam) {
       PostMessage(hwnd, WM_CLOSE, 0, 0);
    }
    return TRUE;
  }

  /*
      DWORD WINAPI TerminateApp(DWORD dwPID, DWORD dwTimeout)

      Purpose:
        Shut down a 32-Bit Process

      Parameters:
        dwPID
           Process ID of the process to shut down.

        dwTimeout
           Wait time in milliseconds before shutting down the process.

      Return Value:
        TA_FAILED - If the shutdown failed.
        TA_SUCCESS_CLEAN - If the process was shutdown using WM_CLOSE.
        TA_SUCCESS_KILL - if the process was shut down with
           TerminateProcess().
  */ 
  DWORD WINAPI
  TerminateApp(DWORD dwPID, DWORD dwTimeout) {
    HANDLE   hProc;
    DWORD   dwRet;

    // If we can't open the process with PROCESS_TERMINATE rights,
    // then we give up immediately.
    hProc = OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE, FALSE, dwPID);
    if(hProc == NULL) {
       return TA_FAILED;
    }

    // TerminateAppEnum() posts WM_CLOSE to all windows whose PID
    // matches your process's.
    EnumWindows((WNDENUMPROC)TerminateAppEnum, (LPARAM)dwPID);

    // Wait on the handle. If it signals, great. If it times out,
    // then you kill it.
    if(WaitForSingleObject(hProc, dwTimeout)!=WAIT_OBJECT_0) {
       dwRet=(TerminateProcess(hProc,0)?TA_SUCCESS_KILL:TA_FAILED);
    } else {
       dwRet = TA_SUCCESS_CLEAN;
    }
    CloseHandle(hProc);

    return dwRet;
  }

  /*
      returns process id.
  */
  DWORD
  StartApp(const string& cmd) {
    DWORD pid=0;
    STARTUPINFO si; 
    PROCESS_INFORMATION pi; 
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO); 
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    if (CreateProcess(NULL, (char*)cmd.c_str(), 
        0, 0, 1, NORMAL_PRIORITY_CLASS, 
        0, 0, &si, &pi)) {
      pid=pi.dwProcessId;
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
    } else {
      cerr<<"CreateProcess failed: "<<cmd<<endl;
    }
    return pid;
  }

}


DirectD::DirectD() :
    _host_name("localhost"),
    _port(8001), _app_pid(0),
    _controller(false), _verbose(false) {
}

DirectD::~DirectD() {

}

void
DirectD::set_host_name(const string& host_name) {
  _host_name=host_name;
}

void
DirectD::set_port(int port) {
  _port=port;
}

void
DirectD::send_start_app(const string& cmd) {
  
}

void
DirectD::start_app(const string& cmd) {
  _app_pid=StartApp(cmd);
}

void
DirectD::send_kill_app(const string& cmd) {
  
}

void
DirectD::kill_app() {
  if (_app_pid) {
    cerr<<"trying k "<<_app_pid<<endl;
    TerminateApp(_app_pid, 1000);
  }
}

void
DirectD::spawn_background_server() {
  stringstream ss;
  ss<<"directd -s "<<_host_name.c_str()<<" "<<_port;
  DWORD serverPID = StartApp(ss.str());
}

void
DirectD::run_server() {
  if (_verbose) cerr<<"server"<<endl;

  PT(Connection) rendezvous = _cm.open_TCP_server_rendezvous(_port, 5);

  if (rendezvous.is_null()) {
    nout << "Cannot grab port " << _port << ".\n";
    exit(1);
  }

  if (_verbose) nout << "Listening for connections on port " << _port << "\n";

  QueuedConnectionListener listener(&_cm, 0);
  listener.add_connection(rendezvous);

  typedef pset< PT(Connection) > Clients;
  Clients clients;

  QueuedConnectionReader reader(&_cm, 1);
  ConnectionWriter writer(&_cm, 1);

  bool shutdown = false;
  while (!shutdown) {
    // Check for new clients.
    while (listener.new_connection_available()) {
      PT(Connection) rv;
      NetAddress address;
      PT(Connection) new_connection;
      if (listener.get_new_connection(rv, address, new_connection)) {
        if (_verbose) nout << "Got connection from " << address << "\n";
        reader.add_connection(new_connection);
        clients.insert(new_connection);
      }
    }

    // Check for reset clients.
    while (_cm.reset_connection_available()) {
      PT(Connection) connection;
      if (_cm.get_reset_connection(connection)) {
        if (_verbose) {
          nout << "Lost connection from "
               << connection->get_address() << "\n";
        }
        clients.erase(connection);
        _cm.close_connection(connection);
      }
    }

    // Process all available datagrams.
    while (reader.data_available()) {
      NetDatagram datagram;
      if (reader.get_data(datagram)) {
        if (_verbose) {
          nout << "Got datagram " /*<< datagram <<*/ "from "
               << datagram.get_address() << ", sending to "
               << clients.size() << " clients.\n";
          if (_verbose) datagram.dump_hex(nout);
        }
        DatagramIterator di(datagram);
        string cmd=di.get_string();
        if (_verbose) {
          cerr<<"server: "<<cmd<<endl;
        }
        if (cmd[0]=='q' && cmd.size()==1) {
          shutdown=true;
        }

        Clients::iterator ci;
        for (ci = clients.begin(); ci != clients.end(); ++ci) {
          writer.send(datagram, (*ci));
        }

        if (datagram.get_length() <= 1) {
          // An empty datagram, shut down the server:
          shutdown = true;
        }
      }
    }

    // Yield the timeslice before we poll again.
    PR_Sleep(PR_MillisecondsToInterval(100));
  }
}

void
DirectD::run_client() {
  if (!_controller) {
    cerr<<"client"<<endl;
  }

  NetAddress host;
  if (!host.set_host(_host_name, _port)) {
    nout << "Unknown host: " << _host_name << "\n";
  }

  PT(Connection) c = _cm.open_TCP_client_connection(host, 5000);

  if (c.is_null()) {
    nout << "No connection.\n";
    exit(1);
  }

  nout << "Successfully opened TCP connection to " << _host_name
       << " on port "
       << c->get_address().get_port() << " and IP "
       << c->get_address() << "\n";

  QueuedConnectionReader reader(&_cm, 0);
  reader.add_connection(c);
  ConnectionWriter writer(&_cm, 0);

  bool lost_connection = false;

  while (!cin.fail() && !lost_connection) {
    NetDatagram datagram;
    if (_controller) {
      cout << "directd send: " << flush;
      string d;
      cin >> d;
      datagram.add_string(d);
      // Send the datagram.
      writer.send(datagram, c);
      if (d[0] == 'q' && d.size()==1) {
        // ...user entered quit command.
        _cm.close_connection(c);
        return;
      }
    }

    // Check for a lost connection.
    while (_cm.reset_connection_available()) {
      PT(Connection) connection;
      if (_cm.get_reset_connection(connection)) {
        nout << "Lost connection from "
             << connection->get_address() << "\n";
        _cm.close_connection(connection);
        if (connection == c) {
          lost_connection = true;
        }
      }
    }

    // Now poll for new datagrams on the socket.
    while (reader.data_available()) {
      if (reader.get_data(datagram)) {
        nout << "Got datagram " /*<< datagram*/ << "from "
             << datagram.get_address() << "\n";
        datagram.dump_hex(nout);
        if (!_controller) {
          DatagramIterator di(datagram);
          string cmd=di.get_string();
          cerr<<"cmd="<<cmd<<endl;
          switch (cmd[0]) {
          case 's':
            start_app("calc");
            break;
          case 'k':
            kill_app();
            break;
          default:
            cerr<<"unknown command: "<<cmd<<endl;
          }
        }
      }
    }
  }
  nout << "Exiting\n";
}

void
DirectD::run_controller() {
  cerr<<"controller"<<endl;
  _controller=true;
  run_client();
}


int
main(int argc, char *argv[]) {
  if (argc > 1 && strcmp(argv[1], "--help")==0) {
    cerr<<"directd [-c <host>] <port>\n"
    "    -c        run as client (else run as server).\n"
    "    host      e.g. localhost\n"
    "    port      default 8001\n";
    return 1;
  }

  cerr<<"directd"<<endl;
  DirectD directd;
  if (argc >= 3) {
    string host=argv[argc-2];
    directd.set_host_name(host);
  }
  char run_as=' ';
  if (argc > 1) {
    directd.set_port(atoi(argv[argc-1]));
    if (strlen(argv[1]) > 1 && argv[1][0] == '-') {
      run_as=argv[1][1];
    }
  }
  switch (run_as) {
  case 's':
    directd.run_server();
    break;
  case 'c':
    directd.run_client();
    break;
  default:
    directd.spawn_background_server();
    PR_Sleep(PR_MillisecondsToInterval(1000));
    directd.run_controller();
    break;
  }
  
  return 0;
}
