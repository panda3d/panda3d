// Filename: loom.cxx
// Created by:  cary (23Sep98)
// 
////////////////////////////////////////////////////////////////////

#include "loom_internal.h"
#include <algo.h>

void app_service::DoIt(void* data)
{
   Action ret;
   app_service* me = (app_service*)data;
   unsigned long s = 0, n = 0;
   condition_variable* c = condition_variable::Null;

   me->_thread = thread::self();
   if (me->_init != NULL)
      (*me->_init)();
   while ((ret = (*me->_service)(s, n, c)) != DONE) {
      switch (ret) {
      case RESERVICE:
         break;  // we're about to do this anyway
      case YIELD:
         thread::yield();
         break;
      case SLEEP:
         thread::sleep(s, n);
         s = n = 0;
         break;
      case WAIT:
         c->wait();
         c = condition_variable::Null;
         break;
      }
   }
   if (me->_cleanup != NULL)
      (*me->_cleanup)();
   {
      mutex_lock l(*main_thread_mutex);
      me->_thread = thread::Null;
      service_list::iterator i=find(task_list->begin(), task_list->end(), me);
      if (i == task_list->end())
         throw thread_fatal(-1);
      task_list->erase(i);
   }
   thread::exit(NULL);
}

void RegisterAppService(vv_func init, av_func service, vv_func cleanup,
                        vv_func info)
{
   app_service* app = new app_service(init, service, cleanup, info);
   if (task_list == (service_list *)0L) {
      task_list = new service_list;
      main_thread_mutex = new mutex;
   }
   mutex_lock l(*main_thread_mutex);
   task_list->push_back(app);
}

void SendMainThreadMessage(main_thread_message& m)
{
   main_thread_message *mess = new main_thread_message(m);
   {
      mutex_lock l(*message_mutex);
      while (!main_thread_empty_flag)
         main_thread_empty->wait();
      message_to_main_thread = mess;
      main_thread_empty_flag = false;
      main_thread_full->signal();
   }
}
