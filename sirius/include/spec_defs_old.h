#define  SINGLES_KEY            1500     /* Singles spectra S1-S10      (4096)     */
#define  ESP_KEY                1501     /* E-counter spectra           (2048x64)  */
#define  DESP_KEY               1502     /* DE-counter spectra          (2048x64)  */
#define  EDESP_KEY              1503     /* DE-E-counter spectra        (2048x64)  */
#define  THICKSP_KEY            1504     /* Thickness spectra           (2048x64)  */
#define  GESP_KEY               1505     /* Ge spectra                  (4096x6)   */
#define  TGESP_KEY              1506     /* Ge T spectra                (512x6)    */
#define  NASP_KEY               1507     /* NaI spectra                 (2048x32)  */
#define  TNASP_KEY              1508     /* NaI T spectra               (512x32)   */
#define  ALFNA_KEY              1509     /* Alpha-NaI matrix            (2048x512) */
#define  ALFGE_KEY              1510     /* Alpha-Ge matrix             (2048x512) */
#define  MAT_KEY                1511     /* General purpose matrix      (2048x64)  */

#define  SINGLES_XSIZE          4096      /* Singles spectra S1-S10 */
#define  ESP_XSIZE              2048      /* E-counter spectra      */
#define  DESP_XSIZE             2048      /* DE-counter spectra     */
#define  EDESP_XSIZE            2048      /* DE-E-counter spectra   */
#define  THICKSP_XSIZE          2048      /* Thickness spectra      */
#define  GESP_XSIZE             4096      /* Ge spectra             */
#define  TGESP_XSIZE             512      /* Ge T spectra           */
#define  NASP_XSIZE             2048      /* NaI spectra            */
#define  TNASP_XSIZE             512      /* NaI T spectra          */
#define  ALFNA_XSIZE            2048      /* Alpha-NaI matrix       */
#define  ALFGE_XSIZE            2048      /* Alpha-Ge matrix        */
#define  MAT_XSIZE              2048      /* General purpose matrix */

#define  SINGLES_YSIZE            10      /* Singles spectra S1-S10 */
#define  ESP_YSIZE                64      /* E-counter spectra      */
#define  DESP_YSIZE               64      /* DE-counter spectra     */
#define  EDESP_YSIZE              64      /* DE-E-counter spectra   */
#define  THICKSP_YSIZE            64      /* Thickness spectra      */
#define  GESP_YSIZE                6      /* Ge spectra             */
#define  TGESP_YSIZE               6      /* Ge T spectra           */
#define  NASP_YSIZE               32      /* NaI spectra            */
#define  TNASP_YSIZE              32      /* NaI T spectra          */
#define  ALFNA_YSIZE             512      /* Alpha-NaI matrix       */
#define  ALFGE_YSIZE             512      /* Alpha-Ge matrix        */
#define  MAT_YSIZE                64      /* General purpose matrix */

#define  SINGLES_SIZE    (4*SINGLES_XSIZE*SINGLES_YSIZE) /* 163840    160 kB */
#define  ESP_SIZE        (4*ESP_XSIZE    *ESP_YSIZE)     /* 524288    512 kB */
#define  DESP_SIZE       (4*DESP_XSIZE   *DESP_YSIZE)    /* 524288    512 kB */
#define  EDESP_SIZE      (4*EDESP_XSIZE  *EDESP_YSIZE)   /* 524288    512 kB */
#define  THICKSP_SIZE    (4*THICKSP_XSIZE*THICKSP_YSIZE) /* 524288    512 kB */
#define  GESP_SIZE       (4*GESP_XSIZE   *GESP_YSIZE)    /* 98304      96 kB */
#define  TGESP_SIZE      (4*TGESP_XSIZE  *TGESP_YSIZE)   /* 12288      12 kB */
#define  NASP_SIZE       (4*NASP_XSIZE   *NASP_YSIZE)    /* 262144    256 kB */
#define  TNASP_SIZE      (4*TNASP_XSIZE  *TNASP_YSIZE)   /* 65536      64 kB */
#define  ALFNA_SIZE      (4*ALFNA_XSIZE  *ALFNA_YSIZE)   /* 4194304     4 MB */
#define  ALFGE_SIZE      (4*ALFGE_XSIZE  *ALFGE_YSIZE)   /* 4194304     4 MB */
#define  MAT_SIZE        (4*MAT_XSIZE    *MAT_YSIZE)     /* 524288    512 kB */
