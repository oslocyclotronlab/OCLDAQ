#include "FitTau.h"

FitTau::FitTau() 
{
  chanNumber = 0;
  modNumber = 0;
}

FitTau::~FitTau() 
{
}

int FitTau::IdentifyTracePulse (unsigned short *trace, // Trace array
			     unsigned int traceSize, // Size of trace array
			     unsigned int trigLen, // Trigger filter length
			     unsigned int trigGap, // Trigger filter gap
			     double *trigLeadSum, // Trigger filter lead sum
			     double *trigTrailSum, // Trigger filter trail sum
			     double trigThresh, // Trigger filter threshold
			     unsigned int *peak, // Pulse maximum location
			     unsigned int *valley) // Pulse minimum location
{
  /* Finds a pulse in an ADC trace for finding the value of tau.  Returns 
     0 for successfully finding a peak, 1 if peak not found in trace */
  
  unsigned int FIT_TRACE_SIZE=8191;
  
  /* Given a trace identify the first pulse from the start */
  
  unsigned int   i = 0; // Used to track the current position in the trace
  unsigned int   PrevMin; // A variable to store a previously found minimum
  unsigned int   PrevMax; // A variable to store a previously found maximum
  unsigned int   MaxOccur; // Max occurrence counter
  unsigned int   MinOccur; // Min occurrence counter
  unsigned short PulseMax = 0; // Pulse maximum to find the maximum location
  unsigned short PulseMin = 65535; // Pulse minimum to find the minimum location
  
  /* Accumulate TrigTrailSum over the first TrigLen points. Report an 
     error if end of trace is encountered. */
  *trigTrailSum = 0;
  while (i < trigLen && i < traceSize) {
    *trigTrailSum += (double)trace[i++];
  }
  if (i == traceSize) { return (1); }
  
  /* Step over the gap. Report an error if end of trace is encountered. */
  i += trigGap;
  if (i >= traceSize) { return (1); }
  
  /* Accumulate TrigLeadSum over the next TrigLen points. Report an 
     error if end of trace is encountered. */
  *trigLeadSum = 0;
  while (i < (2 * trigLen + trigGap) && i < traceSize) {
    *trigLeadSum += (double)trace[i++];
  }
  if (i == traceSize) { return (1); }
  
  /* Determine the trigger point i of the pulse using the running trigger 
     filter sums. The condition for triggering is the exceeding the value 
     TrigThresh by the difference (TrigLeadSum - TrigTrailSum). Report an 
     error if end of trace is encountered. */
  while ((*trigLeadSum - *trigTrailSum) < trigThresh && i < traceSize) {
    *trigLeadSum  += ((double)trace[i] - (double)trace[i - trigLen]);
    *trigTrailSum += ((double)trace[i - trigLen - trigGap] - 
		      (double)trace[i - 2 * trigLen - trigGap]);
    i++;
  }
  if (i == traceSize) { return (1); }
  
  /* Step back at the beginning of the trailing sum to make sure the 
     trigger point does not happen beyond the maximum of the pulse */
  i -= (trigLen + trigGap); 
  
  /* Find the maximum location of the pulse *Peak by slowly extending
     the right limit of the maximization interval. A maximum found repeatedly 
     4*(trigger width) times is reported as the pulse maximum. Report an error 
     if end of trace is encountered. */
  MaxOccur = 0;
  while (MaxOccur < 10 * (2 * trigLen + trigGap) && i < traceSize) {
    PrevMax = *peak;
    if (PulseMax < trace[i]) { 
      PulseMax = trace[i]; 
      *peak = i;
      MaxOccur = 0;
    } else {
      MaxOccur++;
    }
    i++;
  }
  if (i == traceSize) { return (1); }
  
  /* Find the minimum location of the pulse *Valley by slowly extending 
     the right limit of the minimization interval. A minimum found 
     repeatedly 4*(trigger width) times is reported as the pulse minimum. 
     The pulse minimum is the minimum point between the pulses or the last 
     point of the pulse determined by the maximum fitting range (set by 
     FIT_TRACE_SIZE). Report an error if end of trace is encountered. */
  MinOccur = 0;
  i = *peak;
  while (MinOccur < 10 * (2 * trigLen + trigGap) && 
	 i < *peak + FIT_TRACE_SIZE && i < traceSize) {
    PrevMin = *valley;
    if (PulseMin > trace[i]) { 
      PulseMin = trace[i]; 
      *valley = i;
      MinOccur = 0; 
    } else {
      MinOccur++;
    }
    i++;
  }
  if (i == traceSize) { return (1); }
  
  return (0);
}

int FitTau::TauFromMoments(unsigned short *trace, unsigned int traceSize,
			double dt, double *tau) {
  double sp1, sp2;
  double N = double(traceSize); // Number of samples in the sum
  double I;

  sp1 = sp2 = I = 0;
  while (I < N) {
    sp1 += (double)trace[(unsigned int)I] * ((I+1)*N*N - 3.0*(I+1)*(I+1)*N
					     + 2.0*(I+1)*(I+1)*(I+1));
    sp2 += (double)trace[(unsigned int)I] * (I*N*N - 3.0*I*I*N
					     + 2.0*I*I*I);
    I++;
  }
  /* Compute tau from the analytic formula and report in units of dt */
  *tau = 1.0 / log(sp1/sp2) * dt;
  return (0);
}
 
int FitTau::TauFromFit(unsigned short *trace, unsigned int traceSize,
		    double dt, double *tau) {
  


}
 
int FitTau::BinTrace (double *trace, unsigned int traceSize, double *bins,
		   unsigned int binNum, double *binCounts) 
{
  unsigned int i = 1;
  unsigned int j = 0;
  double minTrace = 32768;
  double maxTrace = 0;
  double h; // Bin half-width

  while (j < traceSize) {
    if (maxTrace < trace[j]) { maxTrace = trace[j]; }
    if (minTrace > trace[j]) { minTrace = trace[j]; }
    j++;
  }
  h = (maxTrace - minTrace) / binNum / 2;
  bins[0] = minTrace + h;
  bins[binNum - 1] = maxTrace - h;
  while (i < (binNum - 1)) {
    bins[i] = bins[0] + 2*h*i;
    i++;
  }
  i = 0;
  while (i < binNum) {
    j = 0;
    binCounts[i] = 0;
    while (j < traceSize) {
      if (trace[j] > (bins[i]-h) && trace[j] <= (bins[i]+h)) {
	binCounts[i]++;
      }
      j++;
    }
    i++;
  }
  return 0;

}

double FitTau::ArrayMax (double *a, 
                 unsigned int  ArraySize, 
		 unsigned int *Index)
{
   double *a_last = a + ArraySize;
   double *a_first = a;
   double *maxpos = a;
   while (a < a_last) { if (*maxpos < *a) maxpos = a; a++; }
   *Index = maxpos - a_first;
   return *maxpos;
}

int FitTau::FindTau (unsigned short ModNum, unsigned short ChanNum, double *Tau) 
{
  double dt; // Time in seconds between trace samples
  double dt2;
  unsigned short trace[RANDOMINDICES_LENGTH];
  unsigned int traceSize = RANDOMINDICES_LENGTH;
  int retval;
  unsigned int pulseFindAttempt; // Pulse finding attempt counter
  unsigned int pulseCounter; // Pulse counter
  unsigned int traceCounter; // Trace counter
  unsigned int start; // Running starting point for peak finding analysis in trace
  unsigned int peak; // Current peak location in trace
  unsigned int valley; // Current vally location in trace
  unsigned int dummy; // Dummy variable
  double trigLen; // Trigger filter length
  double trigGap; // Trigger filter gap
  double trigLeadSum; // Trigger filter leading sum
  double trigTrailSum; // Trigger filter trailing sum
  double trigThresh; // Trigger filter threshold
  double tauArray[RANDOMINDICES_LENGTH]; // Reasonable tau values from analyzed peaks
  double tauBins[RANDOMINDICES_LENGTH]; // Array of tau intervals for binning
  double tauBinCounts[RANDOMINDICES_LENGTH]; // Counts in tau bins

  /* Check if module number is valid */
  if ( ModNum >= 10 ) {
    cout << "***Error*** TauFind: invalid Pixie module number " << ModNum << endl;
    return(-1);
  }
  
  /* Check if channel number is valid */
  if (ChanNum >= 16) {
    cout << "***Error*** TauFind: invalid Pixie channel number " << ChanNum << endl;
    return(-1);
  }

  /* Get time between ADC samples */
  retval = Pixie16ReadSglChanPar("XDT", &dt, ModNum, ChanNum);
  if (retval == 0) {
    dt /= 1.0e6;
  } else {
    cout << "Error reading XDT parameter value for module " << ModNum
	 << ", channel " << ChanNum << endl;
  }
  
  /* Get trigger filter length */
  retval = Pixie16ReadSglChanPar("TRIGGER_RISETIME", &trigLen, ModNum, ChanNum);
  if (retval != 0) {
    cout << "Error reading TRIGGER_RISETIME parameter value for module " << ModNum
	 << ", channel " << ChanNum << endl;
  } 
  
  /* Get trigger filter gap */
  retval = Pixie16ReadSglChanPar("TRIGGER_FLATTOP", &trigGap, ModNum, ChanNum);
  if (retval != 0) {
    cout << "Error reading TRIGGER_FLATTOP parameter value for module " << ModNum
	 << ", channel " << ChanNum << endl;
  }

  /* Rescale trigger filter parameters */
  trigLen = (unsigned int)(((trigLen*1000)/10)/ dt*1e-8);
  trigGap = (unsigned int)(((trigGap*1000)/10)/ dt*1e-8);
  
  /* Get trigger filter threshold */
  retval = Pixie16ReadSglChanPar("TRIGGER_THRESHOLD", &trigThresh, ModNum, ChanNum);
  if (retval != 0) {
    cout << "Error reading TRIGGER_THRESHOLD parameter value for module " << ModNum 
	 << ", channel " << ChanNum << endl;
  }
  trigThresh *= (dt/1e-8);
  
  cout << "Trigger length    " << trigLen << endl
       << "Trigger gap       " << trigGap << endl
       << "Trigger threshold " << trigThresh << endl
       << "dt                " << dt << endl;

  /* Analyze 200 pulses using no more than 5000 traces to 
     avoid an infinite loop on an empty channel */
  
  pulseCounter = traceCounter = 0;
  
  while (pulseCounter < 200 && traceCounter < 5000) {
    /* Get a trace */
    if ((retval = Pixie16AcquireADCTrace(ModNum)) < 0) {
      cout << "Failed to get ADC traces in module " << ModNum << endl;
      return(-3);
    }
    
    /* Read a trace */
    if ((retval = Pixie16ReadSglChanADCTrace(trace, RANDOMINDICES_LENGTH, 
					     ModNum, ChanNum)) < 0) {
      cout << "Failed to read ADC traces from module " << ModNum 
	   << ", channel " << ChanNum << endl;
      return(-4);
    }

    /* Find pulses in trace one by one using Identify_Trace_Pulse function, 
       until the function returns an error.  Make no more than 100 attempts to 
       find pulses.  The variable start is a running starting point in the 
       trace from which pulse identification analysis begins -- start is not 
       allowed to go over the trace length.  Identify_Trace_Pulse function 
       reports baseline, peak and valley variables which isolate a pulse in the 
       trace.  Peak and valley are unsigned integer absolute shifts from the 
       start of the trace, while baseline is a double precision number. */

    start = pulseFindAttempt = 0;
    while (pulseFindAttempt < 100 && start < traceSize && 
	   IdentifyTracePulse(&trace[start], traceSize-start, trigLen, 
				trigGap, &trigLeadSum, &trigTrailSum, 
				trigThresh, &peak, &valley) == 0) {
      peak += start; // Peak and valley are reported relative to the start
      valley += start;

      /* The points between peak and valley are used to find values of tau.  
	 No less than 5*(2*trigLen+trigGap) points are allowed.  Tau is 
	 determined using a procedure from moments of the data with Legendre 
	 polynomials.  */
      if (valley - peak > 5*(2*trigLen + trigGap)) {
	TauFromMoments( &trace[peak + (valley-peak) / 10],
			valley - peak - (valley-peak) / 10,
			dt, &tauArray[pulseCounter]);
	/* Check for reasonable values of tau */
	if ((tauArray[pulseCounter] > 1e-8)) {
	  tauArray[pulseCounter++] *= 1.0e6;
	}
      }
      /* Next starting point for analysis is the previous valley minus 
	 the total trigger filter width unless valley-peak is too small. */
      if (valley-peak > 2*trigLen - trigGap) {
	start = valley - 2*trigLen - trigGap;
      } else {
	start = valley;
      }
      pulseFindAttempt++;
    }
    traceCounter++;
  }

  /* Further process the array of tau values by binning into a 
     distribution.  The maximum of the distribution is reported as
     the best estimate for tau. */
  if (pulseCounter > 5) {
    BinTrace (tauArray, pulseCounter, tauBins, 200, tauBinCounts);
    ArrayMax(tauBinCounts, 200, &dummy);
    *Tau = tauBins[dummy];
    cout << "pulseCounter " << pulseCounter << " traceCounter " 
	 << traceCounter << endl;
  } else {
    cout << "Failed to find sufficient number of pulses in channel " 
	 << ChanNum << endl;
    cout << "pulseCounter " << pulseCounter << " traceCounter " 
	 << traceCounter << endl;
    return(-5);
  }
  return(0);
  
}
