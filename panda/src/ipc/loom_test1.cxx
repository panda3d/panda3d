// Filename: loom_test1.cxx
// Created by:  cary (30Sep98)
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

#include "loom.h"
#include <dconfig.h>

static int test1_phase;
static int test1_phase2;
static mutex loom_test1_mutex;
static condition_variable loom_test1_cond(loom_test1_mutex);

#define PRINT(x) { mutex_lock l(main_thread_print_mutex); x; }

void loom_test1_init2(void)
{
   PRINT(cerr << "In loom_test1_init2" << endl);
   test1_phase2 = 0;
}

Action loom_test1_service2(unsigned long& s, unsigned long& n,
                           condition_variable*&)
{
   if (test1_phase2 == 0) {
      test1_phase2 = 1;
      s = 4;
      PRINT(cerr << "loom_test1_service2: sleeping for 4 seconds" << endl);
      return SLEEP;
   } else if (test1_phase2 == 1) {
      test1_phase2 = 2;
      PRINT(cerr << "loom_test1_service2: signaling loom_test1_service to continue" << endl);
      loom_test1_cond.signal();
      n = 500000000;
      PRINT(cerr << "loom_test1_service2: sleeping for 1/2 second" << endl);
      return SLEEP;
   } else if (test1_phase2 == 2) {
      test1_phase2 = 3;
      PRINT(cerr << "loom_test1_service2: sending INFO message to main thread" << endl);
      main_thread_message m(main_thread_message::INFO);
      SendMainThreadMessage(m);
      PRINT(cerr << "loom_test1_service2: returning RESERVICE" << endl);
      return RESERVICE;
   } else if (test1_phase2 == 3) {
      if (test1_phase < 19) {
         PRINT(cerr << "loom_test1_service2: returning RESERVICE" << endl);
         return RESERVICE;
      } else {
         test1_phase2 = 4;
         PRINT(cerr << "loom_test1_service2: sending INFO message to main thread" << endl);
         main_thread_message m(main_thread_message::INFO);
         SendMainThreadMessage(m);
         PRINT(cerr << "loom_test1_service2: sleeping for 3/4 second" << endl);
         n = 750000000;
         return SLEEP;
      }
   } else {
      PRINT(cerr << "loom_test1_service2: returning DONE" << endl);
      return DONE;
   }
}

void loom_test1_cleanup2(void)
{
   PRINT(cerr << "In loom_test1_cleanup2" << endl);
}

void loom_test1_init(void)
{
   PRINT(cerr << "In loom_test1_init" << endl);
   test1_phase = 0;
}

Action loom_test1_service(unsigned long& s, unsigned long& n,
                          condition_variable*& c)
{
   ++test1_phase;
   switch (test1_phase) {
   case 5:
      PRINT(cerr << "loom_test1_service: returning YIELD to service manager" << endl);
      return YIELD;
   case 7:
      s = 5;
      PRINT(cerr << "loom_test1_service: returning SLEEP to service manager (s=5)" << endl);
      return SLEEP;
   case 9:
      PRINT(cerr << "loom_test1_service: registering loom_test1_service2, etc, and returning YIELD" << endl);
      RegisterAppService(loom_test1_init2, loom_test1_service2, loom_test1_cleanup2);
      return YIELD;
   case 11:
      {
         PRINT(cerr << "loom_test1_service: sending RESCAN message to main thread, returning YIELD" << endl);
         main_thread_message m(main_thread_message::RESCAN);
         SendMainThreadMessage(m);
         return YIELD;
      }
   case 13:
      PRINT(cerr << "loom_test1_service: returning WAIT to service manager" << endl);
      loom_test1_mutex.lock();
      c = &loom_test1_cond;
      return WAIT;
   case 15:
      n = 500000000;
      PRINT(cerr << "loom_test1_service: returning SLEEP (n=500000000)" << endl);
      return SLEEP;
   case 17:
      {
         PRINT(cerr << "loom_test1_service: sending LOAD message to main thread (libloom_test2.so)" << endl);
         main_thread_message m(main_thread_message::LOAD, "libloom_test2.so");
         SendMainThreadMessage(m);
         s = 3;
         PRINT(cerr << "loom_test1_service: returning SLEEP (s=3)" << endl);
         return SLEEP;
      }
   case 19:
      {
         PRINT(cerr << "loom_test1_service: sending INFO message to main thread" << endl);
         main_thread_message m(main_thread_message::INFO);
         SendMainThreadMessage(m);
         n = 500000000;
         PRINT(cerr << "loom_test1_service: returning SLEEP (s=3)" << endl);
         return SLEEP;
      }
   case 23:
      PRINT(cerr << "loom_test1_service: returning DONE to service manager" << endl);
      return DONE;
   default:
      PRINT(cerr << "loom_test1_service: returning RESERVICE to service manager" << endl);
      return RESERVICE;
   };
}

void loom_test1_cleanup(void)
{
   PRINT(cerr << "In loom_test1_cleanup" << endl);
}

void loom_test1_info(void)
{
   PRINT(cerr << "Loom_test1_info function (phase = " << test1_phase << ")" << endl);
}


Configure(loom_test1);

ConfigureFn(loom_test1)
{
   RegisterAppService(loom_test1_init, loom_test1_service, loom_test1_cleanup,
                      loom_test1_info);
}
