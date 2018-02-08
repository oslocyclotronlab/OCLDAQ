#ifndef ENERGYFILTER_H_
#define ENERGYFILTER_H_
#include "Table.h"
#include "pixie16app_export.h"

class EnergyFilter: public Table
{
public:
  EnergyFilter(const TGWindow * p, const TGWindow * main, /*char **/ string name, 
		 int columns=3, int rows=16, int NumModules=13);
	Bool_t ProcessMessage (Long_t msg, Long_t parm1, Long_t parm2);
	
	int load_info (Long_t mod);
	int change_values (Long_t mod);
	void setModuleNumber(int moduleNr) {
	  modNumber = (short)moduleNr;
	  numericMod->SetIntNumber(modNumber);
	};

	TGNumberEntry* filterRange ;
	virtual ~EnergyFilter();
	unsigned short modNumber;
	unsigned int  fRange;
	bool Load_Once;
	float risetime, flattop;
	unsigned short chanNumber;
	TGNumberEntry* chanCopy; 
};

#endif /*ENERGYFILTER_H_*/
