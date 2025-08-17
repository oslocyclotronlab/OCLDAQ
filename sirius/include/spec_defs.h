#ifndef SPEC_DEFS
#define SPEC_DEFS

#define  LABRSP_KEY             1500     /* LaBr singles spectra (1-32) (32768x32)  */
#define  TLABRSP_KEY            1501     /* LaBr time spectra (1-32)    (32768x32)  */
#define  LABRCSP_KEY			1502	 /* LaBr cal. spectra (1-32)    (32768x32)  */
#define  LABRCFD_KEY			1503	 /* LaBr spectra(CFD fail)(1-32)(32768x32)  */

#define  LABRSP_XSIZE           65536   /* LaBr energy spectra		*/
#define  TLABRSP_XSIZE          65536   /* LaBr time spectra        */
#define  LABRCSP_XSIZE			65536	/* LaBr energy spectra		*/
#define  LABRCFD_XSIZE          65536   /* LaBr CFD fail spectra    */

#define  LABRSP_YSIZE           12      /* LaBr time spectra	 	*/
#define  TLABRSP_YSIZE          12      /* Thickness spectra        */
#define  LABRCSP_YSIZE			12		/* LaBr energy spectra		*/
#define  LABRCFD_YSIZE          12      /* LaBr CFD fail spectra    */


#define  LABRSP_SIZE    (4*LABRSP_XSIZE*LABRSP_YSIZE)   /* 6291456  6 MB    */
#define  TLABRSP_SIZE   (4*TLABRSP_XSIZE*TLABRSP_YSIZE) /* 6291456  6 MB    */
#define	 LABRCSP_SIZE   (4*LABRCSP_XSIZE*LABRCSP_YSIZE) /*                  */
#define  LABRCFD_SIZE   (4*LABRCFD_XSIZE*LABRCFD_YSIZE)

#endif // SPEC_DEFS