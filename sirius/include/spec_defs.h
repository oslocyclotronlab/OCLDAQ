#ifndef SPEC_DEFS
#define SPEC_DEFS

#define  LABRSP_KEY             1500     /* LaBr singles spectra (1-32) (32768x32)  */
#define  DESP_KEY               1501     /* DE-counter spectra          (32768x64)  */
#define  ESP_KEY                1502     /* E-counter spectra           (32768x8)   */
#define  PPAC_KEY               1503     /* PPAC spectra 				(32768x4)   */
#define  EDESP_KEY              1504     /* DE-E-counter spectra        (2048x2048) */
#define  EDECC_KEY              1505     /* DE-E-counter calibrated     (2048x2048) */
#define	 EDESS_KEY				1506	 /* DE-E-counter spectra		(2048x2048)	*/
#define  TLABRSP_KEY            1507     /* LaBr time spectra (1-32)    (32768x32)  */
#define  TPPAC_KEY				1508	 /* PPAC time spectra (1-32)	(32768x32)	*/
#define  LABRCSP_KEY			1509	 /* LaBr cal. spectra (1-32)    (32768x32)  */
#define  LABRCFD_KEY			1510	 /* LaBr spectra(CFD fail)(1-32)(32768x32)  */
#define  DECFD_KEY				1511     /* DE CFD fail spectra (1-64)  (32768x64)  */
#define  ECFD_KEY               1512     /* E CFD fail spectra (1-8)    (32768x8)   */
#define  GUARD_KEY              1513	 /* E guard ring spectra (1-8)  (32768x8)   */

#define  LABRSP_XSIZE           32768   /* LaBr energy spectra		*/
#define  DESP_XSIZE             32768   /* E-counter spectra        */
#define  ESP_XSIZE              32768   /* DE-counter spectra       */
#define  EDESP_XSIZE            2048    /* DE-E-counter spectra     */
#define  EDECC_XSIZE            2048    /* DE-E-counter spectra     */
#define  TLABRSP_XSIZE          32768   /* LaBr time spectra        */
#define  PPAC_XSIZE             32768   /* PPAC spectra             */
#define  TPPAC_XSIZE			32768	/* PPAC time spectra		*/
#define  EDESS_XSIZE			2048	/* DE-E-counter spectra		*/
#define  LABRCSP_XSIZE			32768	/* LaBr energy spectra		*/
#define  LABRCFD_XSIZE          32768   /* LaBr CFD fail spectra    */
#define  DECFD_XSIZE            32768   /* DE CFD fail spectra      */
#define  ECFD_XSIZE             32768   /* E CFD fail spectra       */
#define  GUARD_XSIZE            32768   /* Guard ring spectra       */

#define  LABRSP_YSIZE           32      /* LaBr time spectra	 	*/
#define  DESP_YSIZE             64      /* E-counter spectra        */
#define  ESP_YSIZE              8       /* DE-counter spectra       */
#define  EDESP_YSIZE            2048    /* DE-E-counter spectra     */
#define  EDECC_YSIZE            2048    /* DE-E-counter spectra     */
#define  TLABRSP_YSIZE          32      /* Thickness spectra        */
#define  PPAC_YSIZE             4       /* PPAC spectra             */
#define  TPPAC_YSIZE			32		/* PPAC time spectra		*/
#define  EDESS_YSIZE			2048	/* DE-E-counter spectra		*/
#define  LABRCSP_YSIZE			32		/* LaBr energy spectra		*/
#define  LABRCFD_YSIZE          32      /* LaBr CFD fail spectra    */
#define  DECFD_YSIZE            64      /* DE CFD fail spectra      */
#define  ECFD_YSIZE             8       /* E CFD fail spectra       */
#define  GUARD_YSIZE            8       /* Guard ring spectra       */


#define  LABRSP_SIZE    (4*LABRSP_XSIZE*LABRSP_YSIZE)   /* 6291456  6 MB    */
#define  DESP_SIZE      (4*DESP_XSIZE    *DESP_YSIZE)   /* 8388608  8 MB    */
#define  ESP_SIZE       (4*DESP_XSIZE   *DESP_YSIZE)    /* 2097152  2 MB    */
#define  EDESP_SIZE     (4*EDESP_XSIZE  *EDESP_YSIZE)   /* 33554432 32 MB   */
#define  EDECC_SIZE     (4*EDECC_XSIZE  *EDECC_YSIZE)   /* 33554432 32 MB   */
#define  TLABRSP_SIZE   (4*TLABRSP_XSIZE*TLABRSP_YSIZE) /* 6291456  6 MB    */
#define  PPAC_SIZE      (4*PPAC_XSIZE   *PPAC_YSIZE)	/* 					*/
#define  TPPAC_SIZE		(4*TPPAC_XSIZE  *TPPAC_YSIZE)	/*					*/
#define  EDESS_SIZE     (4*EDESS_XSIZE *EDESS_YSIZE)	/*					*/
#define	 LABRCSP_SIZE   (4*LABRCSP_XSIZE*LABRCSP_YSIZE) /*                  */
#define  LABRCFD_SIZE   (4*LABRCFD_XSIZE*LABRCFD_YSIZE)
#define  DECFD_SIZE     (4*DECFD_XSIZE*DECFD_YSIZE)
#define  ECFD_SIZE      (4*ECFD_XSIZE*ECFD_YSIZE)
#define  GUARD_SIZE     (4*GUARD_XSIZE*GUARD_YSIZE)

#endif // SPEC_DEFS
