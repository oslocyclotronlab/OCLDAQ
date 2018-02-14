#ifndef TRIGGERFILTER_H_
#define TRIGGERFILTER_H_
#include "Table.h"
#include "pixie16app_export.h"
class TriggerFilter: public Table
{
public:
	TriggerFilter(const TGWindow * p, const TGWindow * main, 
		      /*char **/string name, int columns=4, int rows=16, int NumModules=13);
	virtual ~TriggerFilter();
	Bool_t ProcessMessage (Long_t msg, Long_t parm1, Long_t parm2);
	int change_values (Long_t mod);
	int load_info (Long_t mod);
	void SetModuleNumber(int moduleNr) {
	  modNumber = (short)moduleNr;
	  numericMod->SetIntNumber(modNumber);
	}

	short int chanNumber;
	short int modNumber; 
	TGNumberEntry* chanCopy;
	bool Load_Once;
	char tmp[10];
	float tpeak,tgap,thresh;
};

#endif /*TRIGGERFILTER_H_*/
