// Filename: test_thread.cxx
// Created by:  
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

#include <pandabase.h>
#include <stdlib.h>
#include "ipc_thread.h"
#include "ipc_condition.h"

static mutex print_mutex;

static mutex m;
static condition_variable cv(m);

class CTestThread : public thread
{
  private:

    void run(void *arg)
    {
      cout << "Hey main.  This is " << (char *) arg << ".  Wake the hell up." << endl;
      cout.flush();

      cv.signal();
    }

  public:

    CTestThread(char *name) : thread(name)
    {
      cout << name << " LIVES!." << endl;
      cout.flush();

      start();
    }

    ~CTestThread(void) {}
};

class CTestThread2 : public thread
{
  private:

    void run(void *arg)
    {
      cout << "Oh- so NOW you need me." << endl;
      cout.flush();

      cv.wait();
      cv.signal();

      cout << "bastid." << endl;
      cout.flush();
    }

  public:

    CTestThread2(char *name) : thread(name)
    {
      cout << name << " LIVES!." << endl;
      cout.flush();

      start();
    }

    ~CTestThread2(void) {}
};

int main(int, char **)
{
  CTestThread *test_thread = new CTestThread("test_thread");
  CTestThread2 *test_thread2 = new CTestThread2("test_thread2");

  cout << "My name is main, and i'm going to go to sleep." << endl;
  cout.flush();

  cv.wait();

  cout << "I hate you test_thread." << endl;
  cout.flush();

  cv.signal();
  cv.wait();

  cout << "I hate you test_thread2." << endl;
  cout.flush();

  return 0;
}
