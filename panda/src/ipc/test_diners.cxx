// Filename: test_diners.cxx
// Created by:  cary (16Sep98)
// 
////////////////////////////////////////////////////////////////////

// A solution to the infamous dining philosophers, implemented using the
// threading abstraction.  This program exercises thread creation and
// destruction, mutexes, and condition variables.

#include <pandabase.h>
#include <stdlib.h>
#include "ipc_thread.h"
#include "ipc_condition.h"

#ifdef WIN32_VC
static int last_rand = 0;
#endif /* __WIN32__ */

static mutex rand_mutex;

static int random_l(void)
{
   mutex_lock l(rand_mutex);
   int i = rand();
#ifdef WIN32_VC
   last_rand = i;
#endif /* __WIN32__ */
   return i;
}

static mutex print_mutex;

#define PRINTMSG(x) { mutex_lock l(print_mutex); x; }

// n philosophers sharing n chopsticks.  Philosophers are poor folk and can't
// afford luxuries like 2 chopsticks per person.
#define N_DINERS 5

static mutex chopsticks[N_DINERS];

// At most n philosophers are allowed into the room, others would have to
// wait at the door.  This restriction demonstrates the use of condition
// variables.

static mutex room_mutex;

static condition_variable room_condition(room_mutex);
static int room_occupancy = 0;

static class philosopher* phils[N_DINERS];

class philosopher : public thread {
   private:
      void run(void* arg) {
         int id = *(int*)arg;
         delete (int*)arg;
#ifdef WIN32_VC
         rand_mutex.lock();
         srand(last_rand);
         rand_mutex.unlock();
#endif /* __WIN32__ */
         int l = id;
         int r = l+1;
         if (r == N_DINERS)
            r = 0;
         if (l & 1) {
            int t = l;
            l = r;
            r = t;
         }
         PRINTMSG(cerr << "Philosopher #" << id << " has entered the room."
                  << endl);
         int count = random_l() % 10 + 1;
         while (--count) {
            chopsticks[l].lock();
            chopsticks[r].lock();
            PRINTMSG(cerr << "Philosopher #" << id
                     << " is eating spaghetti now." << endl);
            thread::sleep(random_l()%2, random_l()%1000000000);
            chopsticks[l].unlock();
            chopsticks[r].unlock();
            PRINTMSG(cerr << "Philosopher #" << id
                     << " is pondering about life." << endl);
            thread::sleep(random_l()%2, random_l()%1000000000);
         }
         room_mutex.lock();
         --room_occupancy;
         phils[id] = (philosopher*)0L;
         room_mutex.unlock();
         room_condition.signal();
         PRINTMSG(cerr << "Philosopher #" << id << " has left the room ("
                  << room_occupancy << " left)." << endl);
      }

      // the destructor of a class that inherits from thread should never be
      // public (otherwise the thread object can be destroyed while the
      // underlying thread is still running).
      ~philosopher(void) {}
      inline void* make_arg(const int i) { return (void*)new int(i); }
   public:
      philosopher(const int id) : thread(make_arg(id)) {
         start();
      }
};

int main(int, char**)
{
   int i;
   room_mutex.lock();
   for (i=0; i<N_DINERS; ++i)
      phils[i] = new philosopher(i);
   room_occupancy = N_DINERS;
   while (1) {
      while (room_occupancy == N_DINERS) {
         PRINTMSG(cerr << "main thread about to block " << room_occupancy
                  << endl);
         room_condition.wait();
      }
      // hmm.. someone left the room.
      room_mutex.unlock();
      // sleep for a while and then create a new philosopher
      PRINTMSG(cerr << "main thread sleep" << endl);
      thread::sleep(1, 200000000);
      PRINTMSG(cerr << "main thread wake up" << endl);
      room_mutex.lock();
      for (i=0; i<N_DINERS; ++i)
         if (phils[i] == (philosopher*)0L)
            break;
      if (i == N_DINERS) {
         PRINTMSG(cerr
                  << "Contrary to what I was tolk, no one has left the room!!!"
                  << endl);
         PRINTMSG(cerr << "I give up!" << endl);
         exit(1);
      }
      phils[i] = new philosopher(i);
      ++room_occupancy;
   }
}
