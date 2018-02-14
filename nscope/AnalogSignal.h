#ifndef ANALOGSIGNAL_H_
#define ANALOGSIGNAL_H_
#include "Table.h"
#include "TGComboBox.h"
#include "pixie16app_export.h"

class AnalogSignal: public Table
{
public:
  AnalogSignal(const TGWindow * p, const TGWindow * main, /*char **/string name, int columns=3,
	      int rows=16, int NumModules=13);
	
	virtual ~AnalogSignal();
	Bool_t ProcessMessage (Long_t msg, Long_t parm1, Long_t parm2);
	int change_values (Long_t mod);
	int load_info (Long_t mod);
	void setModuleNumber(int moduleNr) {
	  modNumber = (short)moduleNr;
	  numericMod->SetIntNumber(modNumber);
	};
		
	short int modNumber; 
	short int chanNumber;
	bool Load_Once;
	Pixel_t color;
	TGVerticalFrame* ColumnSign, *ColumnGain;
	TGComboBox *lstBox[17], *lstBoxGain[17];
	TGNumberEntry* chanCopy;
	int pol_temp;
	int gain_temp;
	float offset_temp;
};

#endif /*ANALOGSIGNAL_H_*/
