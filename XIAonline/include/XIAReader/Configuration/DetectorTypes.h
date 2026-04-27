//
// Created by Vetle Wegner Ingeberg on 27/04/2026.
//

#ifndef DETECTORTYPES_H
#define DETECTORTYPES_H

#define NUM_SI_DE_DET 64
#define NUM_SI_DE_TEL 8

enum DetectorType {
    invalid,    //!< Invalid address
    labr,       //!< Is a labr detector
    deDet,      //!< Is a Delta-E segment
    eDet,       //!< Is a E detector
    eGuard,     //!< Is a E guard ring
    ppac,       //!< Is a PPAC
    rfchan,     //!< Is a RF channel
    qint,       //!< Is a charge integrator
    oscarF,     //!< Front detector (OSCAR particle)
    oscarB,     //!< Back detector (OSCAR particle)
    any,        //!< Any detector
    unused      //!< Is a unused XIA channel
};

#endif // DETECTORTYPES_H
