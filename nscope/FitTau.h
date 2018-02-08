#ifndef FITTAU_H_
#define FITTAU_H_

#include <iostream>

#include "pixie16app_export.h"

using namespace std;

class FitTau
{
  
 public:
  FitTau();
  virtual ~ FitTau ();

  short int chanNumber;
  short int modNumber;

  int IdentifyTracePulse (unsigned short *trace, unsigned int traceSize,
			 unsigned int trigLen, unsigned int trigGap,
			 double *trigLeadSum, double *trigTrailSum,
			 double trigThresh, unsigned int *peak, 
			 unsigned int *valley);
  int TauFromMoments (unsigned short *trace, unsigned int traceSize,
		      double dt, double *tau);
  int TauFromFit (unsigned short *trace, unsigned int traceSize, 
		  double dt, double *tau);
  int BinTrace (double *trace, unsigned int traceSize, double *bins,
		unsigned int binNum, double *binCounts);
  double ArrayMax (double *a, unsigned int ArraySize, unsigned int *Index);
  int FindTau (unsigned short ModNum, unsigned short ChanNum, double *Tau);

};

#endif /* FITTAU_H_ */
