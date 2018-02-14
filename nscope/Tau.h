#ifndef TAU_H_
#define TAU_H_
#include "Table.h"
#include "pixie16app_export.h"

class Tau:public Table
{
public:
  Tau (const TGWindow * p, const TGWindow * main, /*char **/string name, int columns = 2,
       int rows = 16, int NumModules=13);
  virtual ~ Tau ();
  TGNumberEntry *chanCopy;
  short int chanNumber;
  short int modNumber;
  char tmp[10];
  int change_values (Long_t mod);
  int load_info (Long_t mod);
  Bool_t ProcessMessage (Long_t msg, Long_t parm1, Long_t parm2);
  void SetModuleNumber(int moduleNr) {
    modNumber = (short)moduleNr;
    numericMod->SetIntNumber(modNumber);
  };
  bool Load_Once;

  float decay;
};

#endif /*TAU_H_ */
