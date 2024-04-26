#ifndef _TARGET_H_
#define _TARGET_H_

//#include <pthread.h>

#define TLSF_MLOCK_T            osMutexId
#define TLSF_CREATE_LOCK(l)     {(*l) = osMutexCreate  (osMutex (DYNMemMutex));}
#define TLSF_DESTROY_LOCK(l)    {osMutexDelete(*l);}
#define TLSF_ACQUIRE_LOCK(l)    { \
	if (__get_IPSR() != 0U) { \
	} \
	else { \
		osMutexWait((*l), osWaitForever); \
		__disable_irq(); \
	} \
}
	
#define TLSF_RELEASE_LOCK(l)    { \
	if (__get_IPSR() != 0U) { \
	} \
	else { \
		__enable_irq(); \
		osMutexRelease((*l)); \
	} \
}

#endif
