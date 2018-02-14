#define  LABRSP_KEY             1500     /* LaBr singles spectra (1-48) (32768x48)  */
#define  DESP_KEY               1501     /* E-counter spectra           (32768x64)  */
#define  ESP_KEY                1502     /* DE-counter spectra          (32768x16)  */
#define  EDESP_KEY              1503     /* DE-E-counter spectra        (2048x2048) */
#define  TLABRSP_KEY            1504     /* LaBr time spectra (1-48)    (32768x48)  */


#define  LABRSP_XSIZE           32768   /* LaBr time spectra (1-48) */
#define  DESP_XSIZE             32768   /* E-counter spectra        */
#define  ESP_XSIZE              32768   /* DE-counter spectra       */
#define  EDESP_XSIZE            2048    /* DE-E-counter spectra     */
#define  TLABRSP_XSIZE          32768   /* Thickness spectra        */

#define  LABRSP_YSIZE           48      /* LaBr time spectra (1-48) */
#define  DESP_YSIZE             64      /* E-counter spectra        */
#define  ESP_YSIZE              16      /* DE-counter spectra       */
#define  EDESP_YSIZE            2048    /* DE-E-counter spectra     */
#define  TLABRSP_YSIZE          64      /* Thickness spectra        */


#define  LABRSP_SIZE    (4*LABRSP_XSIZE*LABRSP_YSIZE)   /* 6291456  6 MB    */
#define  DESP_SIZE      (4*DESP_XSIZE    *DESP_YSIZE)   /* 8388608  8 MB    */
#define  ESP_SIZE       (4*DESP_XSIZE   *DESP_YSIZE)    /* 2097152  2 MB    */
#define  EDESP_SIZE     (4*EDESP_XSIZE  *EDESP_YSIZE)   /* 16777216 16 MB   */
#define  TLABRSP_SIZE   (4*TLABRSP_XSIZE*TLABRSP_YSIZE) /* 6291456  6 MB    */
