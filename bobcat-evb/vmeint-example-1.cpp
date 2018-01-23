// from http://isscvs.cern.ch/cgi-bin/viewcvs-all.cgi/detectors/LAr/Vme/src/VME_RIO.cpp?root=atlastdaq&view=co
// after google search "/dev/vme VME_INT_LINK"

//  $Log: VME_RIO.cpp,v $
//  Revision 1.2  2005/02/28 15:01:12  fbellach
//  Add "Log" CVS keyword.
//

#include "VME_RIO.h"

extern "C" {
unsigned long	find_controller(unsigned long, unsigned long, int, int, int, void*);
int				return_controller(unsigned long, unsigned long);
}

VME::UINT32	VME::myPageSize	= VME::PAGE_SIZE_64K;
VME* 		VME::myInstance = 0;
int 		VME::myNbInstances = 0;
static		VmeImp::UINT32	theAm[] = {	VmeImp::A16_NP_DA, VmeImp::A24_NP_DA, VmeImp::A32_NP_DA, VmeImp::A24_CR_CSR };

VME* VME::Open()
{
	if(myInstance == 0)
	{	char * pEnv = getenv("ATL_ONLINE_VME");

		if(pEnv && (strcmp("VME_RIO8064", pEnv) == 0))
		{	myPageSize	= PAGE_SIZE_16M;
			SetDebug(GetMessageString(PAGE_SIZE_16M));
		}
    		else
			SetDebug(GetMessageString(PAGE_SIZE_64K));

		myInstance = new VME();
	}
	
	if(myInstance)
		myNbInstances++;
		
	return myInstance;
}

int VME::Close()
{
	if(myInstance)
	{	if(--myNbInstances == 0)
		{	delete myInstance;
		
			myInstance	= 0;
	}	}
	
	return 0;
}

void* VmeImp::Map(const UINT32 offset, const UINT32 blockSize)
{
	struct 	pdparam_master	theParams = { 1, 0, 0, 1 };

	UINT32	startAddr	= myBAddr + offset;
	UINT32	endAddr		= startAddr + blockSize;

	for(vector<MapAbs*>::iterator it = myMap.begin(); it != myMap.end(); it++)
	{	MyMap* mit = dynamic_cast<MyMap*>(*it);
			   
		if((startAddr >= mit->startAddr) && (endAddr <= mit->endAddr))
			return (void*) (mit->lAddr + (startAddr - mit->startAddr));
	}

	MyMap*	map 		= new MyMap();
	
	map->startAddr		= (startAddr/VME::myPageSize)*VME::myPageSize;
	map->size			= blockSize + (startAddr - map->startAddr);
	int	incr			= (map->size%VME::myPageSize) ? 1 : 0;
	map->size			= (((map->size/VME::myPageSize) + incr)*VME::myPageSize);
	map->endAddr			= map->startAddr + map->size;

	map->lAddr		= ::find_controller(map->startAddr, map->size, theAm[myAm], 0, 0, &theParams);
//	cout << "Mapping   >>>>>>>> find_controller = " << hex << map->startAddr << " " << hex << map->size << " " << int(map->lAddr) << endl;
	if(map->lAddr == (VME::UINT32) -1)
	{	SetError(find_controller_ERROR);
		return 0;
	}

	myMap.push_back(map);

	return (void*) (map->lAddr + (startAddr - map->startAddr));
}

void VmeImp::MyMap::Unmap()
{
	::return_controller(lAddr, size);
}

////////////////////////////////////////////////////////////////////////////////
// VMEbus interrupt
////////////////////////////////////////////////////////////////////////////////

/*----------------------------------------------------------------------------*/
VmeInterruptImp::VmeInterruptImp () : myMaxDescriptor (0), myFd (-1)
/*----------------------------------------------------------------------------*/
{
  return;
}

/*----------------------------------------------------------------------------*/
VmeInterruptImp::~VmeInterruptImp ()
/*----------------------------------------------------------------------------*/
{
  if (myFd != -1) { close (myFd); }
}

/*----------------------------------------------------------------------------*/
void VmeInterruptImp::Dump (void)
/*----------------------------------------------------------------------------*/
{
  printf ("\n\033[7mINDEX VECTOR   LEVEL TYPE %-16s\033[m\n", "NAME");

  for(size_t ii=0; ii<myItems.size (); ii++) {
    printf ("\033[1m%-5d\033[m %02x (%-3u) %-5d %-4s %-16s\n",
            ii, myItems[ii]->vector, myItems[ii]->vector, myItems[ii]->level,
            myItems[ii]->roak? "ROAK": "RORA",
            myItems[ii]->name.c_str ());
  }

  printf ("\n\n");

  return;
}

/*----------------------------------------------------------------------------*/
VmeInterruptAbs::status_type VmeInterruptImp::GetStatusType (unsigned int status)
/*----------------------------------------------------------------------------*/
{
  VmeInterruptAbs::status_type status_type;

  switch (status) {
    case VME_E_VctRouted:
      status_type = VME_INT_EINVAL;
      break;
    case VME_E_VctBusy:
      status_type = VME_INT_EAGAIN;
      break;
    case VME_E_NotLinked:
      status_type = VME_INT_UNKNOWN;
      break;
    case VME_E_WtAbort:
      status_type = VME_INT_EINTR;
      break;
    case EFAULT:
      status_type = VME_INT_EFAULT;
      break;
    default:
      status_type = VME_INT_SUCCESS;
  }

  return (status_type);
}

/*----------------------------------------------------------------------------*/
VmeInterruptAbs::status_type VmeInterruptImp::GenerateIrq (unsigned char vector, unsigned int level)
/*----------------------------------------------------------------------------*/
{
  return (VME_INT_SUCCESS);
}

/*----------------------------------------------------------------------------*/
VmeInterruptAbs::status_type VmeInterruptImp::Link (void)
/*----------------------------------------------------------------------------*/
{
  if (myItems.empty ()) { return (VME_INT_ENOVECT); }

  for(size_t ii=0; ii<myItems.size (); ii++) {
    vme_int_cmd_t int_cmd;
    struct vme_conf conf;

    if ((myFd = open ("/dev/vme", O_RDWR)) == -1) {
      perror (__PRETTY_FUNCTION__);
      return (VME_INT_NOTOPEN);
    }

    myVectorMap[myFd] = myItems[ii]->vector;

    if (ioctl (myFd, VME_GET_CONF, &conf) == -1) {
      perror (__PRETTY_FUNCTION__);
      return (VME_INT_EFAULT);
    }

    myConfigMap[myItems[ii]->vector] = new config_type (myFd, conf);
    int_cmd.vector                   = myItems[ii]->vector;

    if (ioctl (myFd, VME_INT_LINK, &int_cmd) == -1) {
      perror (__PRETTY_FUNCTION__);
      return (static_cast<status_type> (int_cmd.status));
    }

    conf.ilev   |= (1 << myItems[ii]->level);
    conf.ivec    = myItems[ii]->vector;
    conf.acfail  = 0;
    conf.sysfail = 0;

    if (ioctl (myFd, VME_SET_CONF, &conf) == -1) {
      perror (__PRETTY_FUNCTION__);
      return (VME_INT_EFAULT);
    }
  }

  return (VME_INT_SUCCESS);
}

/*----------------------------------------------------------------------------*/
VmeInterruptAbs::status_type VmeInterruptImp::RegisterSignal (int signo)
/*----------------------------------------------------------------------------*/
{
  return (VME_INT_SUCCESS);
}

/*----------------------------------------------------------------------------*/
VmeInterruptAbs::status_type VmeInterruptImp::Reenable (void)
/*----------------------------------------------------------------------------*/
{
  return (VME_INT_SUCCESS);
}

/*----------------------------------------------------------------------------*/
VmeInterruptAbs::status_type VmeInterruptImp::Unlink (void)
/*----------------------------------------------------------------------------*/
{
  if (myItems.empty ()) { return (VME_INT_ENOVECT); }

  for(size_t ii=0; ii<myItems.size (); ii++) {
    vme_int_cmd_t int_cmd;

    config_type* config = myConfigMap[myItems[ii]->vector];

    if (config) {
      int_cmd.vector = myItems[ii]->vector;

      if (ioctl (config->fd, VME_INT_ULNK, &int_cmd) == -1) {
        return (static_cast<status_type> (int_cmd.status));
      }

      if (ioctl (config->fd, VME_SET_CONF, &config->old) == -1) {
        perror (__PRETTY_FUNCTION__);
        return (VME_INT_EFAULT);
      }
    }
    else {
      return (VME_INT_UNKNOWN);
    }
  }

  return (VME_INT_SUCCESS);
}

/*----------------------------------------------------------------------------*/
VmeInterruptAbs::status_type VmeInterruptImp::Wait (int timeout)
/*----------------------------------------------------------------------------*/
{
  if (myItems.empty ()) { return (VME_INT_ENOVECT); }

  if (myInfoQ.empty ()) {
    fd_set fdset;
    int nfds = 0;
    struct timeval tmo;

    FD_ZERO (&fdset);

    for(size_t ii=0; ii<myItems.size (); ii++) {
      config_type* config = myConfigMap[myItems[ii]->vector];

      if (config) {
        FD_SET (config->fd, &fdset);
        nfds = (config->fd > nfds)? config->fd: nfds;
      }
    }

    tmo.tv_sec  = timeout / 1000;
    tmo.tv_usec = (timeout % 1000) * 1000;

    if (select (nfds + 1, &fdset, (fd_set *)NULL, (fd_set *)NULL, &tmo) != -1) {
      for(size_t ii=0; ii<myItems.size (); ii++) {
        config_type* config = myConfigMap[myItems[ii]->vector];

        if (config) {
          if (FD_ISSET (config->fd, &fdset)) {
            unsigned int count;

            if (read (config->fd, &count, sizeof (unsigned int)) == -1) {
              perror (__PRETTY_FUNCTION__);
              return (VME_INT_EFAULT);
            }

            info_type* info = new info_type (myItems[ii]->name,
                                             myItems[ii]->vector,
                                             myItems[ii]->level,
                                             myItems[ii]->roak,
                                             count);
            myInfoQ.push (info);
          }
        }
      }
    }
  }

  info_type* info = myInfoQ.front ();

  if (info) {
    myInfo = *info;
    delete info;
  }

  return (VME_INT_SUCCESS);
}
