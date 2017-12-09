/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga

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
//#define NOIXEMUL


#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id: SDL_systhread.c,v 1.2 2002/11/20 08:52:36 gabry Exp $";
#endif


#include "../../include/SDL/SDL.h"
#include <exec/nodes.h>
#include "SDL_error.h"
#include "SDL_mutex.h"
#include "SDL_thread.h"
#include "../SDL_thread_c.h"
#include "../SDL_systhread.h"
#include "../../mydebug.h"

#ifdef APOLLO_BLIT
#include "apolloammxenable.h"
#endif

#ifdef AROS
#include <stdlib.h>
#endif

typedef struct {
	int (*func)(void *);
	void *data;
	SDL_Thread *info;
	
	struct Task *wait;
#ifdef SHARED_LIB
	APTR LibBase;
#endif
} thread_args;

#ifndef MORPHOS

#if defined(__SASC) && !defined(__PPC__)
__saveds __asm Uint32 RunThread(register __a0 char *args )
#elif defined(__PPC__) || defined(AROS)
Uint32 RunThread(char *)
#elif !defined(SHARED_LIB) 
Uint32 __saveds RunThread(char * p1)
#else
Uint32 RunThread(char * p1)
#endif
{ 
    register char *args __asm("a0");	
	struct Task *Father;
   
	long *pid;
	 APTR ixemulbase = 0;
#ifndef SHARED_LIB
#ifdef WARPOS
	thread_args *data=(thread_args *)args;
#else
	thread_args *data=(thread_args *)atol(args);
#endif

#else /* SHARED_LIB */
	thread_args *data;
	
	register APTR base __asm("a6");
	{
		ULONG temp=0;

		while(*args >= '0' && *args <= '9')
			temp = (temp * 10) + (*args++ - '0');

		data=(thread_args *)temp;
	}
     
	base=data->LibBase;
	mygeta4();
	D(bug("Library base: %lx\n",base));
#endif

	D(bug("Received data: %lx, father:%lx\n", data, data->wait));
	Father=data->wait;
#ifdef	APOLLO_BLIT
	Apollo_EnableAMMX(); /* AMMX is per task, enable for all threads (if applicable) */
#endif
	
    #ifndef NOIXEMUL
	{
		
		ixemulbase = OpenLibrary("ixemul.library",51);
		if (ixemulbase)
		{
			    ix_CreateChildData(Father,0);
			    SDL_RunThread(data);
				CloseLibrary(ixemulbase);
				Signal(Father,SIGBREAKF_CTRL_F );
				
				
	    }
	}
	#else
	{
	SDL_RunThread(data);
	D(bug("Thread exited, signaling father (%lx)...\n",Father));
	Signal(Father,SIGBREAKF_CTRL_F);
	D(bug("Thread with data %lx ended\n",data));
	}
    #endif
	
     //kprintf(" Task %lx end \n",FindTask(0));
	return(0);
}

#else /* Morphos code */

#include <emul/emulinterface.h>

Uint32 RunTheThread(void)
{
	thread_args *data=(thread_args *)atol((char *)REG_A0);
	struct Task *Father;

	D(bug("Received data: %lx\n",data));
	Father=data->wait;

	SDL_RunThread(data);

	Signal(Father,SIGBREAKF_CTRL_F);
	D(bug("Thread with data %lx ended\n",data));
	return(0);
}

struct EmulLibEntry RunThreadStruct=
{
	TRAP_LIB,
	0,
	(ULONG)RunTheThread
};

void *RunThread=&RunThreadStruct;
#endif


int SDL_SYS_CreateThread(SDL_Thread *thread, void *args)
{
	/* Create the thread and go! */
	char buffer[20];

#ifdef WARPOS
	struct TagItem tags[6];
#endif

#ifdef SHARED_LIB
	extern void *myLibPtr;

	((thread_args *)args)->LibBase= myLibPtr;
	D(bug("CreateThread: librarybase %lx\n",myLibPtr));
#endif

	D(bug("Sending %lx to the new thread...\n",args));

    sprintf(buffer,"%ld",args);


#ifdef WARPOS
	 tags[0].ti_Tag = TASKATTR_CODE;                       tags[0].ti_Data = (ULONG)RunThread;
    tags[1].ti_Tag = TASKATTR_NAME;                        tags[1].ti_Data = (ULONG)"SDL subtask";
    tags[2].ti_Tag = TASKATTR_STACKSIZE;                   tags[2].ti_Data = 100000;
    tags[3].ti_Tag = (args ? TASKATTR_R3 : TAG_IGNORE);    tags[3].ti_Data = (ULONG)args;
    tags[4].ti_Tag = TASKATTR_INHERITR2;                   tags[4].ti_Data = TRUE;
    tags[5].ti_Tag = TAG_DONE;                             tags[5].ti_Data = 0;

    thread->handle=CreateTaskPPC(tags);
#else
	{
	unsigned int stack;
	struct Task *t;
	long *pid;
	
	thread->handle=(struct Task *)CreateNewProcTags(
                    /*NP_Output, Output(),
                    NP_Input, Input(),*/ 
					NP_Name, (ULONG)"SDL subtask",
					/*NP_CloseOutput, FALSE,
                    NP_CloseInput, FALSE,*/ 
					NP_StackSize, 100000,
					NP_Entry, (ULONG)RunThread,
					NP_Cli, TRUE,
					NP_Arguments, (ULONG)buffer,
					TAG_DONE);
	}

	/*Delay(1); // its a testhack dont activate
	Disable();
	struct Task *t;
	struct Task *t2;
	APTR *data;
    t = FindTask(0);
    data = t->tc_UserData;
    t2 = thread->handle;
	t2->tc_UserData = data;
	Enable();*/
#endif

	if(!thread->handle)
	{
		SDL_SetError("Not enough resources to create thread");
		return(-1);
	}

	return(0);
}

void SDL_SYS_SetupThread(void)
{
}

Uint32 SDL_ThreadID(void)
{
	return((Uint32)FindTask(NULL));
}

void SDL_SYS_WaitThread(SDL_Thread *thread)
{
	
	
	struct Node *n;
	int found;
	SetSignal(0L,SIGBREAKF_CTRL_F);
for (;;)
{
	struct List *t;
	found = 0;
	Disable();
	t = &SysBase->TaskWait;
    for ( n = t->lh_Head;
		n;
		n = n->ln_Succ)
		if (thread->handle == n)found = 1;
	t = &SysBase->TaskReady;
	
	for ( n = t->lh_Head;
		n;
		n = n->ln_Succ)
		if (thread->handle == n)found = 1;
	    if (FindTask(0) == n)found =1;
	Enable();

	//kprintf("wait task %lx %ld \n", thread->handle,found);
	if (!found) //check if the task is here
	{
    //kprintf("task is quit%lx %lx \n", thread->handle,n);
    return;
	//TRAP	
	//SetSignal(0L,SIGBREAKF_CTRL_F);
	//Wait(SIGBREAKF_CTRL_F|SIGBREAKF_CTRL_C);
    
	}
	Delay(2);
}	 
}

void SDL_SYS_KillThread(SDL_Thread *thread)
{
	Signal((struct Task *)thread->handle,SIGBREAKF_CTRL_C);
}
