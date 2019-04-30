#ifndef SPEC_DEFS
#define SPEC_DEFS

#define  LABRSP_KEY             1500     /* LaBr singles spectra (1-32) (32768x32)  */
#define  DESP_KEY               1501     /* E-counter spectra           (32768x64)  */
#define  ESP_KEY                1502     /* DE-counter spectra          (32768x16)  */
#define  EDESP_KEY              1503     /* DE-E-counter spectra        (2048x2048) */
#define  TLABRSP_KEY            1504     /* LaBr time spectra (1-32)    (32768x32)  */
#define  PPAC_KEY               1505     /* PPAC spectra 				(32768x4)   */
#define  TPPAC_KEY				1506	 /* PPAC time spectra (1-32)	(32768x32)	*/
#define	 EDESS_KEY				1507	 /* DE-E-counter spectra		(2048x2048)	*/
#define  LABRCSP_KEY			1508	 /* LaBr cal. spectra (1-32)    (32768x32)  */

#define  LABRSP_XSIZE           32768   /* LaBr energy spectra		*/
#define  DESP_XSIZE             32768   /* E-counter spectra        */
#define  ESP_XSIZE              32768   /* DE-counter spectra       */
#define  EDESP_XSIZE            2048    /* DE-E-counter spectra     */
#define  TLABRSP_XSIZE          32768   /* LaBr time spectra        */
#define  PPAC_XSIZE             32768   /* PPAC spectra             */
#define  TPPAC_XSIZE			32768	/* PPAC time spectra		*/
#define  EDESS_XSIZE			2048	/* DE-E-counter spectra		*/
#define  LABRCSP_XSIZE			32768	/* LaBr energy spectra		*/

#define  LABRSP_YSIZE           32      /* LaBr time spectra	 	*/
#define  DESP_YSIZE             64      /* E-counter spectra        */
#define  ESP_YSIZE              16      /* DE-counter spectra       */
#define  EDESP_YSIZE            2048    /* DE-E-counter spectra     */
#define  TLABRSP_YSIZE          32      /* Thickness spectra        */
#define  PPAC_YSIZE             4       /* PPAC spectra             */
#define  TPPAC_YSIZE			32		/* PPAC time spectra		*/
#define  EDESS_YSIZE			2048	/* DE-E-counter spectra		*/
#define  LABRCSP_YSIZE			32		/* LaBr energy spectra		*/


#define  LABRSP_SIZE    (4*LABRSP_XSIZE*LABRSP_YSIZE)   /* 6291456  6 MB    */
#define  DESP_SIZE      (4*DESP_XSIZE    *DESP_YSIZE)   /* 8388608  8 MB    */
#define  ESP_SIZE       (4*DESP_XSIZE   *DESP_YSIZE)    /* 2097152  2 MB    */
#define  EDESP_SIZE     (4*EDESP_XSIZE  *EDESP_YSIZE)   /* 33554432 32 MB   */
#define  TLABRSP_SIZE   (4*TLABRSP_XSIZE*TLABRSP_YSIZE) /* 6291456  6 MB    */
#define  PPAC_SIZE      (4*PPAC_XSIZE   *PPAC_YSIZE)	/* 					*/
#define  TPPAC_SIZE		(4*TPPAC_XSIZE  *TPPAC_YSIZE)	/*					*/
#define  EDESS_SIZE     (4*EDESS_XSIZE *EDESS_YSIZE)	/*					*/
#define	 LABRCSP_SIZE   (4*LABRCSP_XSIZE*LABRCSP_YSIZE) /*                  */

#endif // SPEC_DEFS
