#include "Main.h"

using namespace std;

int
main (int argc, char **argv)
{
  TApplication theApp ("App", &argc, argv);
 
  Main mainWindow (gClient->GetRoot ());

  theApp.Run ();

  return 0;
}
