// Filename: loom_test2.cxx
// Created by:  cary (30Sep98)
// 
////////////////////////////////////////////////////////////////////

#include "loom.h"
#include <dconfig.h>

static int test2_phase;

#define PRINT(x) { mutex_lock l(main_thread_print_mutex); x; }

void loom_test2_init(void)
{
   PRINT(cerr << "In loom_test2_init" << endl);
   test2_phase = 0;
}

Action loom_test2_service(unsigned long& s, unsigned long&,
			  condition_variable*&)
{
   ++test2_phase;
   switch (test2_phase) {
   case 0:
      s = 5;
      PRINT(cerr << "loom_test2_service: sleeping for 5 seconds" << endl);
      return SLEEP;
   case 3:
      PRINT(cerr << "loom_test2_service: returning DONE" << endl);
      return DONE;
   default:
      PRINT(cerr << "loom_test2_service: returning RESERVICE" << endl);
      return RESERVICE;
   }
}

Configure(loom_test2);
ConfigureFn(loom_test2)
{
   RegisterAppService(loom_test2_init, loom_test2_service);
}
