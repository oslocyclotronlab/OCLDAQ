#include "lock.h"
#include <stdlib.h>
#include <iostream>

lock::lock(std::string name)
{
  name_base="/var/lock/";
  name_base+=name;
  l=fopen(name_base.c_str(),"r");
  if(l==NULL) //if the file does not exist, the device is free
    {
      l=fopen(name_base.c_str(),"w");
      if(l==NULL)
	{
	  std::cout << name_base << std::endl;
	  printf("can't make a lock file\n");
	  printf("cowardly bailing out ...\n");
	  exit(1);
	}
      fprintf(l,"lock");
      fclose(l);
    }
  else
    {
      printf("the device seems to be locked by another process (%s)\n",name_base.c_str());
      exit(1);
    }
}

lock::~lock()
{
  ldelete();
}

void lock::ldelete()
{
  // printf(name_base.c_str());
  remove(name_base.c_str());
}
