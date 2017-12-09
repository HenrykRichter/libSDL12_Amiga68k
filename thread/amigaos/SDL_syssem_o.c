/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002  Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id: SDL_syssem.c,v 1.2 2002/11/20 08:52:35 gabry Exp $";
#endif

/* An implementation of semaphores using mutexes and condition variables */

#include "SDL_error.h"
#include "SDL_thread.h"
#include "SDL_systhread_c.h"
#include <devices/timer.h>

#ifdef _AROS
#include "SDL_timer.h"
#include <stdlib.h>
#endif

struct SDL_semaphore
{
	struct SignalSemaphore Sem;
	Uint32 count;
	Uint32 waiters_count;
	SDL_mutex *count_lock;
	SDL_cond *count_nonzero;
};

#undef D

#define D(x) kprintf

SDL_sem *SDL_CreateSemaphore(Uint32 initial_value)
{
	SDL_sem *sem;

	sem = (SDL_sem *)malloc(sizeof(*sem));

	if ( ! sem ) {
		SDL_OutOfMemory();
		return(0);
	}

	D(bug("Creating semaphore %lx...\n",sem));

	memset(sem,0,sizeof(*sem));

	InitSemaphore(&sem->Sem);
	if (initial_value)ObtainSemaphore(&sem->Sem);
	return(sem);
}

void SDL_DestroySemaphore(SDL_sem *sem)
{
	D(bug("Destroying semaphore %lx...\n",sem));

	if ( sem ) {
// Condizioni per liberare i task in attesa?
		free(sem);
	}
}

int SDL_SemTryWait(SDL_sem *sem)
{
	if ( ! sem ) {
		SDL_SetError("Passed a NULL semaphore");
		return -1;
	}

	D(bug("TryWait semaphore...%lx\n",sem));

	ObtainSemaphore(&sem->Sem);
//	ReleaseSemaphore(&sem->Sem);

	return 1;
}

int SDL_SemWaitTimeout(SDL_sem *sem, Uint32 timeout)
{
	int retval;
    struct timerequest *tr;
    struct timeval tv;

	if ( ! sem ) {
		SDL_SetError("Passed a NULL semaphore");
		return -1;
	}

	D(bug("WaitTimeout (%ld) semaphore...%lx\n",timeout,sem));

	/* A timeout of 0 is an easy case */
	if ( timeout == 0 ) {
		return SDL_SemTryWait(sem);
	}
/*
	SDL_LockMutex(sem->count_lock);
	++sem->waiters_count;
	retval = 0;
	while ( (sem->count == 0) && (retval != SDL_MUTEX_TIMEDOUT) ) {
		retval = SDL_CondWaitTimeout(sem->count_nonzero,
		                             sem->count_lock, timeout);
	}
	--sem->waiters_count;
	--sem->count;
	SDL_UnlockMutex(sem->count_lock);
*/
   
/* get a pointer to an initialized timer request block */
    tr = create_timer( UNIT_MICROHZ );
    tv.tv_secs	= 0;
	tv.tv_micro	= 1000; // 1 ms
	if(!(retval=AttemptSemaphore(&sem->Sem)))
	{
		while ((timeout > 0) && (retval == FALSE))
		{
		wait_for_timer( tr, tv );
		retval=AttemptSemaphore(&sem->Sem);
		timeout--;
	    }
	}

	if(retval==TRUE)
	{
//		ReleaseSemaphore(&sem->Sem);
		retval=1;
	}
    delete_timer( tr );
	return retval;
}

int SDL_SemWait(SDL_sem *sem)
{
	ObtainSemaphore(&sem->Sem);
	return 0;
//	return SDL_SemWaitTimeout(sem, SDL_MUTEX_MAXWAIT);
}

Uint32 SDL_SemValue(SDL_sem *sem)
{
	Uint32 value;

	value = 0;
	if ( sem ) {
		#ifdef WARPOS 
		value = sem->Sem.ssppc_SS.ss_NestCount;
		#else
		value = sem->Sem.ss_NestCount;
		#endif
//		SDL_UnlockMutex(sem->count_lock);
	}
	return value;
}
 
int SDL_SemPost(SDL_sem *sem)
{
	if ( ! sem ) {
		SDL_SetError("Passed a NULL semaphore");
		return -1;
	} 
	D(bug("SemPost semaphore...%lx\n",sem));
    if (!sem->Sem.ss_NestCount)
    {
    D(bug("No other tasks wait for semaphore %lx",sem));
	return 0;
	}
	ReleaseSemaphore(&sem->Sem);
#if 0
	SDL_LockMutex(sem->count_lock);
	if ( sem->waiters_count > 0 ) {
		SDL_CondSignal(sem->count_nonzero);
	}
	++sem->count;
	SDL_UnlockMutex(sem->count_lock);
#endif
	return 0;
}

