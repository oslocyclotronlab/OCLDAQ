#ifndef SCOPEDT_H_
#define SCOPEDT_H_



#include "Table.h"
#include "pixie16app_export.h"

class ScopedT:public Table
{

 public:
  ScopedT(const TGWindow * p, const TGWindow * main, /*char **/string name, 
	  int columns = 2, int rows = 16);
  virtual ~ScopedT();
  TGNumberEntry *chanCopy;
  short int chanNumber;
  short int modNumber;
  char tmp[10];
  int change_values (Long_t mod);
  int load_info (Long_t mod);
  Bool_t ProcessMessage (Long_t msg, Long_t parm1, Long_t parm2);
  bool Load_Once;
  float decay;
};


#endif /*SCOPEDT_H_*/
