#ifndef PULSESHAPE_H_
#define PULSESHAPE_H_
#include "Table.h"
#include "pixie16app_export.h"
class PulseShape: public Table
{
public:
  PulseShape(const TGWindow * p, const TGWindow * main, /*char **/ string name, int columns=3,
	      int rows=16,int NumModules=13);
	virtual ~PulseShape();
	Bool_t ProcessMessage (Long_t msg, Long_t parm1, Long_t parm2);
	
	short int modNumber;
	short int chanNumber;
	char tmp[10];
	float tlength,tdelay;
	bool Load_Once;
	int change_values (Long_t mod);
	int load_info (Long_t mod);
	void setModuleNumber(int moduleNr) {
	  modNumber = (short)moduleNr;
	  numericMod->SetIntNumber(modNumber);
	};
	
	TGNumberEntry* chanCopy;
//	int PulseShape::LoadInfo(Long_t mod);
};

#endif /*PULSESHAPE_H_*/
