/*
**********************************************************
* file: vlib.c
* 951110 MAWE adapted from 'vmelib.c'
* 951201 MAWE RTPC 8067, vme driver
* 951208 MAWE <ces/vmecmd.h> used
* 960122 MAWE <ces/vmelib.h> instead of vmecmd.h
* 960224 MAWE use 'autoswap'
* 960224 MAWE last update
*
* adapt OS9 'VME calls' to LYNX 'find_controller..'
**********************************************************
*/
#include <sys/types.h>
#include <ces/vmelib.h>

typedef unsigned long uint;

/*
*========================================================
* globals
*--------------------------------------------------------
*/
static struct pdparam_master param = {
	1,	/* no vme iack 			*/
	0,	/* No VME read prefetch 	*/
	0,	/* No VME write posting 	*/
	1,	/* autoswap 			*/
	0,	/* page descriptor number 	*/
	0,	/* dummy			*/
	0,	/* dummy			*/
	0	/* dummy			*/
};
 
/*
*========================================================
* map vme window
*--------------------------------------------------------
*/
uint vme_map(vaddr,wsiz,am)
uint vaddr,wsiz,am;
{
  uint ad;

  ad = find_controller(vaddr,wsiz,am,0,0,&param);
  if (ad == -1){
    return(0);
  }else{
    return(ad);
  }
}
/*
*========================================================
* return page descriptor
*--------------------------------------------------------
*/
uint vme_rel(laddr,wsiz)
uint laddr,wsiz;
{
  return(return_controller(laddr,wsiz));
}
