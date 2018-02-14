#ifndef BASELINE_H_
#define BASELINE_H_
#include "Table.h"
#include "pixie16app_export.h"
class Baseline: public Table
{
public:

Baseline(const TGWindow * p, const TGWindow * main, 
	 /*char **/string name, int columns=3,
	      int rows=16, int NumModules=13);

	virtual ~Baseline();
		int change_values (Long_t mod);
	int load_info (Long_t mod);
	Bool_t ProcessMessage (Long_t msg, Long_t parm1, Long_t parm2);
	void SetModuleNumber(int moduleNr) {
	  modNumber = (short)moduleNr;
	  numericMod->SetIntNumber(modNumber);
	};
	
	short int modNumber;
	bool Load_Once;
	TGNumberEntry* chanCopy;
	float blcut,blpercent;
	unsigned short chanNumber;
};

#endif /*BASELINE_H_*/
