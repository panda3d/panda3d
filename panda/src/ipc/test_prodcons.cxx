// Filename: test_prodcons.cxx
// Created by:  cary (16Sep98)
// 
////////////////////////////////////////////////////////////////////

#include <pandabase.h>
#include <stdlib.h>
#include "ipc_thread.h"
#include "ipc_condition.h"
#include "ipc_mutex.h"

// test condition variables and timed waits.  makes 2 'producer' threads and
// 3 'consumer' threads

static void producer(void*);
static void consumer(void*);

mutex m;
condition_variable full(m);
condition_variable empty(m);

bool empty_flag = true;
const char* message;

static const char* msgs[] = { "wolf", "fox", "hyena", "dingo" };

int main(int, char**)
{
   cerr << "main: creating producer1" << endl;
   thread::create(producer, (void*)"producer1");
   cerr << "main: creating producer2" << endl;
   thread::create(producer, (void*)"producer2");
   cerr << "main: creating consumer1" << endl;
   thread::create(consumer, (void*)"consumer1");
   cerr << "main: creating consumer2" << endl;
   thread::create(consumer, (void*)"consumer2");
   cerr << "main: creating consumer3" << endl;
   consumer((void*)"consumer3");
   return 0;
}

static int random_l(void)
{
   static mutex rand_mutex;
   mutex_lock l(rand_mutex);
   int i = rand();
   return i;
}

static void consumer(void* arg)
{
   char *name = (char *)arg;
   unsigned long s, n;

   while (1) {
      {
	 mutex_lock l(m);
	 thread::get_time(s, n, 0, 500000000);  // 1/2 second from now
	 while (empty_flag) {
	    cerr << name << ": waiting for message" << endl;
	    if (!full.timedwait(s, n)) {
	       cerr << name << ": timed out, trying again" << endl;
	       thread::get_time(s, n, 0, 500000000);
	    } else if (empty_flag)
	       cerr << name << ": woken but message already consumed" << endl;
	 }
	 cerr << name << ": got message: '" << message << "'" << endl;
	 empty_flag = true;
	 empty.signal();
      }
      thread::sleep(random_l() % 2, 1000000 * (random_l() % 1000));
   }
}

static void producer(void* arg)
{
   char *name = (char *)arg;

   while (1) {
      {
	 mutex_lock l(m);
	 while (!empty_flag) {
	    cerr << name << ": having to wait for consumer" << endl;
	    empty.wait();
	 }
	 message = msgs[random_l() % 4];
	 empty_flag = false;
	 full.signal();
	 cerr << name << ": put message: '" << message << "'" << endl;
      }
      thread::sleep(random_l() % 2, 1000000 * (random_l() % 500) + 500);
   }
}
