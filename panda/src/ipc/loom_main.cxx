// Filename: loom_main.cxx
// Created by:  cary (23Sep98)
// 
////////////////////////////////////////////////////////////////////

#include "loom_internal.h"
#include <dconfig.h>
#include <load_dso.h>
#include <filename.h>

#include <string>

mutex main_thread_print_mutex;

service_list *task_list = (service_list *)0L;
mutex *main_thread_mutex = mutex::Null;
main_thread_message* message_to_main_thread = (main_thread_message*)0L;
mutex *message_mutex;
condition_variable* main_thread_full = condition_variable::Null;
condition_variable* main_thread_empty = condition_variable::Null;
bool main_thread_empty_flag;

int main(int argc, char** argv)
{
   for (int i=0; i<argc; ++i)
      if (argv[i] != (char*)0L) {
         Filename filename = argv[i];
         if (filename.get_extension() == ".so") {
           filename.set_type(Filename::T_dso);
           load_dso(filename);
         }
      }
   if (task_list == (service_list*)0L) {
      // really, this shouldn't happen.  But if none of the loaded SOs do
      // anything, well, we should just exit.
      cerr << "None of the loaded SOs had any work to do." << endl;
      return 0;
   }

   message_mutex = new mutex;
   main_thread_full = new condition_variable(*message_mutex);
   main_thread_empty = new condition_variable(*message_mutex);
   main_thread_empty_flag = true;

   main_thread_mutex->lock();
   service_list::iterator j;
   for (j=task_list->begin(); j!=task_list->end(); ++j)
      (*j)->start();
   main_thread_mutex->unlock();

   while (!task_list->empty()) {
      unsigned long s, n;
      mutex_lock l(*message_mutex);
      thread::get_time(s, n, 2, 0);  // 2 seconds from now
      main_thread_full->timedwait(s, n);
      if (!main_thread_empty_flag) {
         switch (message_to_main_thread->get_message()) {
         case main_thread_message::LOAD:
            {
              Filename filename = message_to_main_thread->get_lib();
              filename.set_type(Filename::F_dso);
              load_dso(filename);
            }
         case main_thread_message::RESCAN:
            for (j=task_list->begin(); j!=task_list->end(); ++j)
               (*j)->start();
            break;
         case main_thread_message::INFO:
            for (j=task_list->begin(); j!=task_list->end(); ++j)
               (*j)->info();
            break;
         default:
            // really should never be here
            break;
         }
         delete message_to_main_thread;
         message_to_main_thread = (main_thread_message*)0L;
         main_thread_empty_flag = true;
         main_thread_empty->signal();
      }
   }
   thread::sleep(0, 500000000); // give some time for cleanup (1/2 sec)
   thread::exit(NULL);
}
