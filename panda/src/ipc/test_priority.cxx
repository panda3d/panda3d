// Filename: test_priority.cxx
// Created by:  cary (16Sep98)
// 
////////////////////////////////////////////////////////////////////

#include <pandabase.h>
#include <stdlib.h>
#include <stdio.h>
#include "ipc_thread.h"

// test thread priorities

#if defined(__arm__) && defined(__atmos__)
#define flush ""
#endif /* __arm__ */

static void func(void*);

static int loop_iterations;

static mutex print_mutex;
#define PRINTMSG(x) { mutex_lock l(print_mutex); x << flush; }

int main(int argc, char** argv)
{
   unsigned long s1, s2, n1, n2;
   char buf[20];

   if ((argc != 2) || (argv[1][0] == '-')) {
      cerr << "Usage: " << argv[0] << " loop_iterations" << endl;
      exit(1);
   }
   loop_iterations = atoi(argv[1]);
   thread::get_time(s1, n1);
   for (int i=0; i<loop_iterations; ++i);
   thread::get_time(s2, n2);
   if (n2 > n1) {
      n2 -= n1;
      s2 -= s1;
   } else {
      n2 = n2 + 1000000000 - n1;
      s2 = s2 - 1 - s1;
   }
   sprintf(buf, "%d.%03d", s2, n2/1000000);
   cout << argv[0] << ": doing " << loop_iterations
	<< " loop itterations (approx " << buf << " seconds per loop)"
	<< endl;
   PRINTMSG(cout << "main: creating h1" << endl);
   thread::create(func, (void*)"h1", thread::PRIORITY_HIGH);
   PRINTMSG(cout << "main: creating m1" << endl);
   thread::create(func, (void*)"m1", thread::PRIORITY_NORMAL);
   PRINTMSG(cout << "main: creating l1" << endl);
   thread::create(func, (void*)"l1", thread::PRIORITY_LOW);
   PRINTMSG(cout << "main: creating h2" << endl);
   thread::create(func, (void*)"h2", thread::PRIORITY_HIGH);
   PRINTMSG(cout << "main: creating m2" << endl);
   thread::create(func, (void*)"m2", thread::PRIORITY_NORMAL);
   PRINTMSG(cout << "main: creating l2" << endl);
   thread::self()->set_priority(thread::PRIORITY_LOW);
   func((void*)"l2");

   return 0;
}

static void func(void* arg)
{
   char *name = (char *)arg;

   while (1) {
      PRINTMSG(cout << name << ": entering 1st compute-bound loop" << endl);
      int i;
      for (i=0; i<loop_iterations; ++i);
      PRINTMSG(cout << name << ": left compute-bound loop; yielding" << endl);
      thread::yield();
      PRINTMSG(cout << name << ": entering 2nd compute-bound loop" << endl);
      for (i=0; i<loop_iterations; ++i);
      PRINTMSG(cout << name
	       << ": left compute-bound loop; sleeping for 2 seconds" << endl);
      thread::sleep(2);
   }
}
