/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file directd.cxx
 * @author skyler
 * @date 2002-04-08
 * Based on test_tcp_*.* by drose.
 */

// This define tells the windows headers to include job objects:
#define _WIN32_WINNT 0x0500

#include "directd.h"
/*#include "pandaFramework.h"
#include "queuedConnectionManager.h"*/

// #include <process.h> #include <Windows.h> #include "pandabase.h"

// #include "queuedConnectionManager.h" #include "queuedConnectionListener.h"
// #include "queuedConnectionReader.h" #include "connectionWriter.h"
#include "netAddress.h"
#include "connection.h"
#include "datagramIterator.h"
#include "netDatagram.h"

#include "pset.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_DIRECT_DIRECTD)
  #error Buildsystem error: BUILDING_DIRECT_DIRECTD not defined
#endif

using std::cerr;
using std::cout;
using std::endl;
using std::string;

namespace {
  // ...This section is part of the old stuff from the original
  // implementation.  The new stuff that uses job objects doesn't need this
  // stuff:

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

    // If we can't open the process with PROCESS_TERMINATE rights, then we
    // give up immediately.
    hProc = OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE, FALSE, dwPID);
    if(hProc == nullptr) {
       return TA_FAILED;
    }

    // TerminateAppEnum() posts WM_CLOSE to all windows whose PID matches your
    // process's.
    EnumWindows((WNDENUMPROC)TerminateAppEnum, (LPARAM)dwPID);

    // Wait on the handle.  If it signals, great.  If it times out, then you
    // kill it.
    if(WaitForSingleObject(hProc, dwTimeout)!=WAIT_OBJECT_0) {
       dwRet=(TerminateProcess(hProc,0)?TA_SUCCESS_KILL:TA_FAILED);
    } else {
       dwRet = TA_SUCCESS_CLEAN;
    }
    CloseHandle(hProc);

    return dwRet;
  }

  /*
      Start an application with the command line cmd.
      returns the process id of the new process/application.
  */
  DWORD
  StartApp(const string& cmd) {
    DWORD pid=0;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    if (CreateProcess(nullptr, (char*)cmd.c_str(),
        0, 0, 1, NORMAL_PRIORITY_CLASS,
        0, 0, &si, &pi)) {
      pid=pi.dwProcessId;
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
    } else {
      nout<<"CreateProcess failed: "<<cmd<<endl;
    }
    return pid;
  }

}

DirectD::DirectD() :
    _reader(&_cm, 1), _writer(&_cm, 0), _listener(&_cm, 0),
    _jobObject(0), _shutdown(false), _useOldStuff(false) {
}

DirectD::~DirectD() {
  // Close all the connections:
  ConnectionSet::iterator ci;
  for (ci = _connections.begin(); ci != _connections.end(); ++ci) {
    _cm.close_connection((*ci));
  }
  _connections.clear();

  kill_all();
}

int
DirectD::client_ready(const string& server_host, int port,
    const string& cmd) {
  std::stringstream ss;
  ss<<"!"<<cmd;
  send_one_message(server_host, port, ss.str());
  return 0;
}

int
DirectD::tell_server(const string& server_host, int port,
    const string& cmd) {
  send_one_message(server_host, port, cmd);
  return 0;
}

bool
DirectD::wait_for_servers(int count, int timeout_ms) {
  if (count <= 0) {
    return true;
  }
  // The timeout is a rough estimate, we may wait slightly longer.
  const int wait_ms=200;
  int cycles=timeout_ms/wait_ms;
  while (cycles--) {
    check_for_new_clients();
    check_for_lost_connection();
    /*
        The following can be more generalized with a little work.
        check_for_datagrams() could take a handler function as an arugment, maybe.
    */
    // check_for_datagrams(); Process all available datagrams.
    while (_reader.data_available()) {
      NetDatagram datagram;
      if (_reader.get_data(datagram)) {
        cout << count << ": Server at " << datagram.get_address()
            << " is ready." << endl;
        datagram.dump_hex(nout);
        // handle_datagram(datagram);
        DatagramIterator di(datagram);
        string s=di.get_string();
        if (s=="r" && !--count) {
          return true;
        }
      }
    }

    // Yield the timeslice before we poll again.
    // PR_Sleep(PR_MillisecondsToInterval(wait_ms));
    Sleep(wait_ms);
  }
  // We've waited long enough, assume they're not going to be ready in the
  // time we want them:
  return false;
}

int
DirectD::server_ready(const string& client_host, int port) {
  send_one_message(client_host, port, "r");
  return 0;
}


void
DirectD::start_app(const string& cmd) {
  nout<<"start_app(cmd="<<cmd<<")"<<endl;
  if (_useOldStuff) {
    _pids.push_back(StartApp(cmd));
    nout<<"    pid="<<_pids.back()<<endl;
  } else {
    if (!_jobObject) {
      _jobObject=CreateJobObject(0, 0);
      if (!_jobObject) {
        nout<<"CreateProcess failed: no _jobObject: "<<GetLastError()<<endl;
        return;
      }
    }
    DWORD pid=0;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    if (CreateProcess(nullptr, (char*)cmd.c_str(),
        0, 0, 1, NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED,
        0, 0, &si, &pi)) {
      // The process must be created with CREATE_SUSPENDED to give us a chance
      // to get the handle into our sgJobObject before the child processes
      // starts sub-processes.
      if (!AssignProcessToJobObject(_jobObject, pi.hProcess)) {
        // ...The assign failed.
        cerr<<"StartJob AssignProcessToJobObject Error: "<<GetLastError()<<endl;
      }
      CloseHandle(pi.hProcess); //?????
      // Because we called CreateProcess with the CREATE_SUSPEND flag, we must
      // explicitly resume the processes main thread.
      if (ResumeThread(pi.hThread) == -1) {
        cerr<<"StartJob ResumeThread Error: "<<GetLastError()<<endl;
      }
      CloseHandle(pi.hThread);
    } else {
      nout<<"StartJob CreateProcess failed: "<<cmd<<endl;
    }
  }
}

void
DirectD::kill_app(int index) {
  if (_useOldStuff) {
    int i = _pids.size() - 1 - index % _pids.size();
    PidStack::iterator pi = _pids.begin() + i;
    if (pi!=_pids.end()) {
      nout<<"trying kill "<<(*pi)<<endl;
      TerminateApp((*pi), 1000);
      _pids.erase(pi);
    }
  } else {
    cerr<<"kill_app(index) not implemented, calling kill_all() instead."<<endl;
    kill_all();
  }
}

void
DirectD::kill_all() {
  if (_useOldStuff) {
    PidStack::reverse_iterator pi;
    for (pi = _pids.rbegin(); pi != _pids.rend(); ++pi) {
      nout<<"trying kill "<<(*pi)<<endl;
      TerminateApp((*pi), 1000);
    }
    _pids.clear();
  } else {
    if (!_jobObject) {
      cerr<<"kill_all(): No open _jobObject"<<endl;
    } else if (!TerminateJobObject(_jobObject, 0)) {
      cerr<<"kill_all() TerminateJobObject Error: "<<GetLastError()<<endl;
    }
    CloseHandle(_jobObject);
    _jobObject=0;
  }
}

void
DirectD::send_command(const string& cmd) {
  NetDatagram datagram;
  datagram.add_string(cmd);
  // Send the datagram.
  ConnectionSet::iterator ci;
  for (ci = _connections.begin(); ci != _connections.end(); ++ci) {
    _writer.send(datagram, (*ci));
  }
}

void
DirectD::handle_datagram(NetDatagram& datagram){
  DatagramIterator di(datagram);
  string cmd=di.get_string();
  handle_command(cmd);
}

void
DirectD::handle_command(const string& cmd) {
  nout<<"DirectD::handle_command: "<<cmd<<endl;
}

void
DirectD::send_one_message(const string& host_name,
    int port,
    const string& message) {
  NetAddress host;
  if (!host.set_host(host_name, port)) {
    nout << "Unknown host: " << host_name << "\n";
  }

  const int timeout_ms=5000;
  PT(Connection) c = _cm.open_TCP_client_connection(host, timeout_ms);
  if (c.is_null()) {
    nout << "No connection.\n";
    return;
  }

  nout << "Successfully opened TCP connection to " << host_name
       << " on port "
       << c->get_address().get_port() << " and IP "
       << c->get_address() << "\n";

  // _reader.add_connection(c);

  NetDatagram datagram;
  datagram.add_string(message);
  _writer.send(datagram, c);

  // PR_Sleep(PR_MillisecondsToInterval(200)); wait_for_servers(1, 10*1000);
  // _reader.remove_connection(c);
  _cm.close_connection(c);
}

int
DirectD::connect_to(const string& host_name, int port) {
  NetAddress host;
  if (!host.set_host(host_name, port)) {
    nout << "Unknown host: " << host_name << "\n";
  }

  const int timeout_ms=5000;
  PT(Connection) c = _cm.open_TCP_client_connection(host, timeout_ms);
  if (c.is_null()) {
    nout << "No connection.\n";
    return 0;
  }

  nout << "Successfully opened TCP connection to " << host_name
       << " on port "
       << c->get_address().get_port() << " and IP "
       << c->get_address() << "\n";

  _reader.add_connection(c);
  _connections.insert(c);
  return  c->get_address().get_port();
}

void
DirectD::disconnect_from(const string& host_name, int port) {
  nout<<"disconnect_from(\""<<host_name<<", port="<<port<<")"<<endl;
  for (ConnectionSet::iterator i=_connections.begin(); i != _connections.end(); ++i) {
    nout<<"    found "<<(*i)->get_address().get_ip_string()<<", port "<<(*i)->get_address().get_port()<<endl;
    if ((*i)->get_address().get_ip_string()==host_name) {
      nout<<"    disconnecting."<<endl;
      _reader.remove_connection((*i));
      _cm.close_connection((*i));
      _connections.erase(i);
      break;
    }
  }
}

void
DirectD::check_for_lost_connection() {
  while (_cm.reset_connection_available()) {
    PT(Connection) c;
    if (_cm.get_reset_connection(c)) {
      nout<<"Lost connection from "<<c->get_address()<<endl;
      _connections.erase(c);
      _cm.close_connection(c);
    }
  }
}

void
DirectD::check_for_datagrams(){
  // Process all available datagrams.
  while (_reader.data_available()) {
    NetDatagram datagram;
    if (_reader.get_data(datagram)) {
      nout << "Got datagram " /*<< datagram <<*/ "from "
           << datagram.get_address() << endl;
      datagram.dump_hex(nout);
      handle_datagram(datagram);
    }
  }
}

void
DirectD::listen_to(int port, int backlog) {
  PT(Connection) rendezvous = _cm.open_TCP_server_rendezvous(port, backlog);
  if (rendezvous.is_null()) {
    nout << "Cannot grab port " << port << ".\n";
    exit(1);
  }
  nout << "Listening for connections on port " << port << "\n";
  _listener.add_connection(rendezvous);
}

void
DirectD::check_for_new_clients() {
  while (_listener.new_connection_available()) {
    PT(Connection) rv;
    NetAddress address;
    PT(Connection) new_connection;
    if (_listener.get_new_connection(rv, address, new_connection)) {
      nout << "Got connection from " << address << "\n";
      _reader.add_connection(new_connection);
      _connections.insert(new_connection);
    }
  }
}
