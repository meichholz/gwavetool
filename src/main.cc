/* ======================================================================

main.cc

Startup code and everything.

Basically just instantiating the application object.

This program is free software
See file COPYING for details.

====================================================================== */

#include "main.h"
#include "app.h"

const char szAppTitle[]="GTK Wave Tool";
const char szAppName[]="gwavetool";

const char szVersion[]=VERSION;

/* ======================================================================
 * debug printing
 * ====================================================================== */

void debug_printf(int debug_class,const char *szFormat, ...)
{
  // if (!(debug_class && ::optDebugFlags) return;
  char szBuffer[1024];
  va_list ap;
  va_start(ap,szFormat);
  vsnprintf(szBuffer,1024,szFormat,ap);
  szBuffer[sizeof(szBuffer)-1]='\0';
  fprintf(stderr,"DEBUG: %s\n",szBuffer);
  va_end(ap);
}

int main(int argc, char *argv[])
{

  TApp *papp=new TApp(argc,argv);
  papp->Run();
  delete papp;

  return 0;
}
