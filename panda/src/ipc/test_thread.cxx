// Filename: test_thread.cxx
// created by:  charles (08jun00)
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
