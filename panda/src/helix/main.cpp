#include "dllpath.h"

#include "HelixClient.h"


// typedef for SetDLLAccessPath

#if defined _DEBUG || defined DEBUG
#include "debug.h"
#endif

#if defined(HELIX_CONFIG_NOSTATICS)
# include "globals/hxglobals.h"
#endif

DLLAccessPath statClnt;
DLLAccessPath* GetDLLAccessPath()
{
    return &statClnt;
}

int main( int argc, char *argv[] )
{
  setvbuf(stdout, NULL, _IONBF, 0);
  HelixClient* myClient = new HelixClient();
  STDOUT("%s", argv[1]);
  myClient->create_player("myPlayer", true);

  //myClient->open_url("myPlayer", argv[1]);
  myClient->open_url("myPlayer", "rage.mp3");
  
  bool done = true; 
  myClient->begin("myPlayer");
  while (done != myClient->is_done("myPlayer")) {
     //Do Nothing for now
    MSG msg;

	GetMessage(&msg, NULL, 0, 0);
	DispatchMessage(&msg);
  }

  delete myClient;
  myClient = 0;
  
  return 0;
}




    










  