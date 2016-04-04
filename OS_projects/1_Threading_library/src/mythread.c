/******************************************************************************
 *
 *  File Name........: mythread.c
 *
 *
 *****************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<ucontext.h>
#include "mythread.h"

#define STACKSIZE 8*1024

//liinked list of child threads, with which parent thread has joined
struct join_list{
	int ch_tid;
	struct join_list *link;
};

struct node{
	ucontext_t cnxt;
	int tid;
	int pid;
	struct join_list *join_th;
	struct join_list *join_th_rear; 
	struct node *next;
};

struct node *front=NULL;
struct node *rear=NULL;
struct node *bfront=NULL;
struct node *brear=NULL;
struct node *curr_th=NULL;

ucontext_t init_cnxt;
ucontext_t dmy_cnxt;
//MyThread mt=NULL;
int thc=0;
//int cr_th=0;
//int pr_th=-1;

struct node * insert_q(ucontext_t nw_cnxt,int th_id,int pr_id,struct join_list *jfront,struct join_list *jrear)
{
	//printf("Insert Queue enter...\n");
	struct node *tmp=(struct node*)malloc(sizeof(struct node));
	if(tmp==NULL)
	{
		//printf("No memory allocated... Maximum thread limit reached...returning gracefully...\n");
		swapcontext(&dmy_cnxt,&init_cnxt);
	}
	tmp->cnxt=nw_cnxt;
	tmp->tid=th_id;
	tmp->pid=pr_id;
	tmp->join_th=jfront;
	tmp->join_th_rear=jrear;
	tmp->next=NULL;

	if(front==NULL && rear==NULL)
	{
		front=tmp;
		rear=front;
	} else {
		rear->next=tmp;
		rear=tmp;
	}
	
	return rear;
}

struct node * pop_q(void)
{
	/*if(front==NULL && rear==NULL)
	{
		//printf("Queue empty, returning NULL");
		return NULL;
	}*/
	
	//printf("Remove Queue enter...\n");
	struct node *tmp;
	
	tmp=front;
		
	if(front==rear)
	{
		//printf("Only one context left, returning it...\n");
		front=NULL;
		rear=NULL;
	} else {
		front=front->next;
	}
	
	return tmp;
}

void removeNode(int t_id)
{
	if(t_id==-1)
	  return;

	if(front==NULL)
	 return;
	   
	struct node *tmp=NULL;
	struct node *prev=NULL;
	
	if(front==rear && front->tid==t_id)
	{
		tmp=front;
		front=NULL;
		rear=NULL;
		free(tmp);
	} else if(front->tid==t_id)
	{
		tmp=front;
		front=front->next;
		free(tmp);
	} else 
	{
		tmp=front;
		
		while(tmp!=NULL && tmp->tid!=t_id)
		{
			prev=tmp;
			tmp=tmp->next;
		}
		
		if(tmp!=NULL && tmp->tid==t_id)
		{
			prev->next=tmp->next;
			free(tmp);
		} else if(tmp==NULL)
		{
			//printf("thread %d , not in blocked queue",t_id);
		}
	}

}

struct node * binsert_q(ucontext_t nw_cnxt,int th_id,int pr_id,struct join_list *jfront,struct join_list *jrear)
{
	//printf("Insert Queue enter...\n");
	struct node *tmp=(struct node*)malloc(sizeof(struct node));
	if(tmp==NULL)
	{
		//printf("No memory allocated... Maximum thread limit reached...returning gracefully...\n");
		swapcontext(&dmy_cnxt,&init_cnxt);
	}
	tmp->cnxt=nw_cnxt;
	tmp->tid=th_id;
	tmp->pid=pr_id;
	tmp->join_th=jfront;
	tmp->join_th_rear=jrear;
	tmp->next=NULL;
	
	if(bfront==NULL && brear==NULL)
	{
		bfront=tmp;
		brear=bfront;
	} else {
		brear->next=tmp;
		brear=tmp;
	}
	
	return brear;
}

struct node * bpop_q(void)
{
	/*if(front==NULL && rear==NULL)
	{
		//printf("Queue empty, returning NULL");
		return NULL;
	}*/
	
	//printf("Remove Queue enter...\n");
	struct node *tmp;
	
	tmp=bfront;
		
	if(bfront==brear)
	{
		//printf("Only one context left, returning it...\n");
		bfront=NULL;
		brear=NULL;
	} else {
		bfront=bfront->next;
	}
	
	return tmp;
}

struct node * bgetNode(int t_id)
{
	if(t_id==-1)
	  return NULL;
	  
	if(bfront==NULL)
		return NULL;
	  
	struct node *tmp=bfront;
	 
	while(tmp!=NULL && (tmp->tid != t_id))
	{
		tmp=tmp->next;
	}
	 
	if(tmp!=NULL && tmp->tid==t_id)
	{
		return tmp;
	}
	
	if(tmp==NULL)
	{
		//printf("thread %d , not in blocked queue",t_id);
	}
	
	return NULL;
}

void bremoveNode(int t_id)
{
	if(t_id==-1)
	  return;

	if(bfront==NULL)
	 return;
	   
	struct node *btmp=NULL;
	struct node *bprev=NULL;
	
	if(bfront==brear && bfront->tid==t_id)
	{
		btmp=bfront;
		bfront=NULL;
		brear=NULL;
		free(btmp);
	} else if(bfront->tid==t_id)
	{
		btmp=bfront;
		bfront=bfront->next;
		free(btmp);
	} else 
	{
		btmp=bfront;
		
		while(btmp!=NULL && btmp->tid!=t_id)
		{
			bprev=btmp;
			btmp=btmp->next;
		}
		
		if(btmp!=NULL && btmp->tid==t_id)
		{
			bprev->next=btmp->next;
			free(btmp);
		} else if(btmp==NULL)
		{
			//printf("thread %d , not in blocked queue",t_id);
		}
	}

}

MyThread MyThreadCreate(void(*start_funct)(void *), void *args)
{
	//printf("MyThreadCreate: START ...\n");
	ucontext_t curr_context;
	MyThread thr;
	
	if(0!=getcontext(&curr_context))
	{
	   //printf("MyThreadCreate : getcontext error...\n");
	}
	//printf(" MyThreadCreate : start _funct == %d \n", start_funct);
	if(start_funct==NULL)
	{
		//printf("MyThreadCreate : NULL function is passed...no thread will be creared...\n");
		return NULL;
	}
	curr_context.uc_stack.ss_sp=(char *)malloc(STACKSIZE);
	if(curr_context.uc_stack.ss_sp==NULL)
	{
		//printf("No memory allocated... Maximum thread limit reached...returning gracefully...\n");
		swapcontext(&dmy_cnxt,&init_cnxt);
	}
	curr_context.uc_stack.ss_size=STACKSIZE;
	curr_context.uc_stack.ss_flags=0;
	curr_context.uc_link=NULL;
	
	//printf("MyThreadCreate: makecontext... \n");
	
	makecontext(&curr_context,(void (*)(void))start_funct,(int)1,(void *)args);
	//printf("Context is created, need to put in Queue \n");
	
	struct node * tmp=insert_q(curr_context,++thc,curr_th->tid,NULL,NULL);
	//printf("MyThreadCreate : Thread=%d , created context=%d is inserted in Queue \n",tmp->tid,tmp->cnxt);
	
	thr=(MyThread)tmp;
	
	//MyThread new_th=(MyThread)thr;
	
	return thr;
}

void MyThreadYield(void)
{
	//printf("MyThreadYield: START ... curr_th = %d , parent_th = %d\n", curr_th->tid, curr_th->pid);
		
	/*if(0!=getcontext(&curr_context))l
	{
		//printf("MyThreadYield : getcontext error... \n");
	}*/
	
	if(front==NULL)
	{
		//printf("No pending ready thread, invoking thread = %d , will continue to run \n", curr_th->tid);
		return;
	}
	
	ucontext_t new_cnxt;
	struct node * new_th=pop_q();
	
	if(new_th == NULL)
	{
		//printf("MyThreadYield: ready queue is empty...continue running this thread... \n");
		return;	
	}
	
	new_cnxt=new_th->cnxt;
	
	//printf("Inserting saved context to the queue : GetContext removed... \n");
	struct node * tmp=insert_q(curr_th->cnxt,curr_th->tid,curr_th->pid,curr_th->join_th,curr_th->join_th_rear);

	int old_th=curr_th->tid;
	int old_pr=curr_th->pid;
	ucontext_t old_cnxt=curr_th->cnxt;
	struct join_list *old_jfront=curr_th->join_th;
	struct join_list *old_jrear=curr_th->join_th_rear;
	curr_th->tid=new_th->tid;
	curr_th->pid=new_th->pid;
	curr_th->cnxt=new_th->cnxt;
	curr_th->join_th=new_th->join_th;
	curr_th->join_th_rear=new_th->join_th_rear;
	

	//printf("MyThreadYield: swap context from thread : %d ,to first ready thread : %d ... \n",tmp->tid,curr_th->tid);
	//printf("MyThreadYield: swap context from =%d , to  = %d  which is also same as new_cnxt=%d\n ",tmp->cnxt,curr_th->cnxt,new_cnxt);
	

	if(swapcontext(&(tmp->cnxt),&(new_th->cnxt))==-1)
	{
		curr_th->tid=old_th;
		curr_th->pid=old_pr;
		curr_th->cnxt=old_cnxt;
		curr_th->join_th=old_jfront;
		curr_th->join_th_rear=old_jrear;
		removeNode(curr_th->tid);
		//printf("MyThreadYield: swapcontext error...\n");	
	}

	//printf("MyThreadYield: EXIT... \n\n\n");
}

int MyThreadJoin(MyThread thread)
{
	//printf("MyThreadJoin: START ... \n");
	if(thread==NULL)
	{
		//printf("NULL thread, returning from MyThreadJoin");
		return 0;
	}
	struct node *child_th;
	child_th=(struct node *)thread;
	//printf("MyThreadJoin: child thread= %d , its parent thread= %d , current invoking thread= %d \n",child_th->tid,child_th->pid,curr_th->tid);

	
	if(curr_th->tid != child_th->pid)
	{
		//printf("MyThreadJoin: FAILURE : specified thread is not an immediate child of invoking thread...\n");
		return -1;
	}
	
	//printf("Search of specified child thread in ready queue...\n");

	// If the child has already terminated, do not block. Note: A child may have terminated without the parent having joined with it.
	
	int cid=child_th->tid;
	int child_exist=0;

	// check for child thread into Ready Queue
	struct node *tmp=front;	
	
	while(tmp!=NULL && tmp->tid != cid )
	{
		tmp=tmp->next;
	}
	
	if(tmp!=NULL && tmp->tid ==cid)
	{
		child_exist=1;
		//printf("Specified child thread exists in Ready queue... \n");
	}
	
	
	//If child not found in Ready Queue, check in Blocked queue
	if(child_exist==0)
	{
		//printf("Specified child thread not in ready queue...search in blocked queue\n");
		tmp=bfront;
		
		while(tmp!=NULL && tmp->tid != cid)
		{
			tmp=tmp->next;		
		}
		
		if(tmp!=NULL && tmp->tid == cid)
		{
			child_exist=1;
			//printf("Specified child thread exists in Blocked queue... \n");
		}
		
	} 	

	if(child_exist==0)
	{
		//printf("Specified child thread is already terminated. Not blocking the invoking thread...\n");
		return 0;
	} else if(child_exist == 1)
	{
		//printf("Specified child thread exists ... \n");
	}
	
	//todo: remove this 
	////printf("MyThreadJoin: RETURN ... \n");
	//return 0;


	//Child thread exists, need to block the invoking thread and move into Blocked queue
	//After blocking, run the first ready thread
		
	
	//saving join pointers for future failure recovery
	struct join_list *old_jfront=curr_th->join_th;
	struct join_list *old_jrear=curr_th->join_th_rear;

	// Before blocking, need to update join_list of this current thread...
	//printf("MyThreadJoin : need to update join_list of this current thread:%d \n",curr_th->tid);
	
	struct join_list *jtmp=(struct join_list *)malloc(sizeof(struct join_list));
	if(jtmp==NULL)
	{
		//printf("No memory allocated... Maximum thread limit reached...returning gracefully...\n");
		swapcontext(&dmy_cnxt,&init_cnxt);
	}
	jtmp->ch_tid=cid;
	jtmp->link=NULL;
	
	if((curr_th->join_th)==NULL)
	{
		curr_th->join_th=jtmp;
		curr_th->join_th_rear=curr_th->join_th;
	} else 
	{
		curr_th->join_th_rear->link=jtmp;
		curr_th->join_th_rear=jtmp;
	}
	
	struct node * blocked_th=binsert_q(curr_th->cnxt,curr_th->tid,curr_th->pid,curr_th->join_th,curr_th->join_th_rear);
	
	//printf("MyThreadJoin : current thread:%d entered in BQ , but actual current thread is= %d\n",blocked_th->tid, curr_th->tid);
	
	int old_th=curr_th->tid;
	int old_pr=curr_th->pid;
	ucontext_t old_cnxt=curr_th->cnxt;


	ucontext_t new_cnxt;
	struct node * new_th=pop_q();

	if(new_th == NULL)
	{
		//printf(" MyThreadJoin: ready queue empty ... \n");
		swapcontext(&dmy_cnxt,&init_cnxt);
	}
	
	new_cnxt=new_th->cnxt;
	
	curr_th->tid=new_th->tid;
	curr_th->pid=new_th->pid;
	curr_th->cnxt=new_th->cnxt;
	curr_th->join_th=new_th->join_th;
	curr_th->join_th_rear=new_th->join_th_rear;
	
	//printf("MyThreadJoin: swap context from thread : %d  to first ready thread : %d ... \n",old_th,curr_th->tid);
	
	if(swapcontext(&(blocked_th->cnxt),&new_cnxt)==-1)
	{
		curr_th->tid=old_th;
		curr_th->pid=old_pr;
		curr_th->cnxt=old_cnxt;
		curr_th->join_th=old_jfront;
		curr_th->join_th_rear=old_jrear;
		bremoveNode(curr_th->tid);
		//printf("MyThreadJoin: Blocking swapcontext error...");
	}
	
	//printf("MyThreadJoin: curr_thread= %d , parent_th = %d ,EXIT... \n",curr_th->tid,curr_th->pid);
}

/*
Waits until all children have terminated. Returns immediately if there are no active children. 
*/
void MyThreadJoinAll(void)
{
	//printf("MyThreadJoinAll: START : for current thread : %d \n", curr_th->tid);
	//printf("Need to traverse through RQ and BQ and add all children of this thread to its join_list...\n");
	
	//saving join pointers for future failure recovery
	struct join_list *old_jfront=curr_th->join_th;
	struct join_list *old_jrear=curr_th->join_th_rear;


	//Traverse through RQ
	
	struct node *tmp=front;
	
	while(tmp!=NULL)
	{
		if(tmp->pid == curr_th->tid)
		{ 
			//printf("Child found , thread=%d , is a child thread of current thread , need to insert into join_list\n",tmp->tid);
			struct join_list *jtmp=(struct join_list *)malloc(sizeof(struct join_list));
			if(jtmp==NULL)
			{
				//printf("No memory allocated... Maximum thread limit reached...returning gracefully...\n");
				swapcontext(&dmy_cnxt,&init_cnxt);
			}
			jtmp->ch_tid=tmp->tid;
			jtmp->link=NULL;
			
			if((curr_th->join_th)==NULL)
			{
				curr_th->join_th=jtmp;
				curr_th->join_th_rear=curr_th->join_th;
			} else 
			{
				curr_th->join_th_rear->link=jtmp;
				curr_th->join_th_rear=jtmp;
			}
		}
		
		tmp=tmp->next;
	}
	
	//Need to Traverse through BQ
	
	//printf("MyThreadJoinAll: now Traversing BQ... \n");
	tmp=bfront;
	
	while(tmp!=NULL)
	{
		if(tmp->pid == curr_th->tid)
		{ 
			//printf("Child found , thread=%d , is child thread of current thread , need to insert into join_list \n",tmp->tid);
			struct join_list *jtmp=(struct join_list *)malloc(sizeof(struct join_list));
			if(jtmp==NULL)
			{
				//printf("No memory allocated... Maximum thread limit reached...returning gracefully...\n");
				swapcontext(&dmy_cnxt,&init_cnxt);
			}
			jtmp->ch_tid=tmp->tid;
			jtmp->link=NULL;
			
			if((curr_th->join_th)==NULL)
			{
				curr_th->join_th=jtmp;
				curr_th->join_th_rear=curr_th->join_th;
			} else 
			{
				curr_th->join_th_rear->link=jtmp;
				curr_th->join_th_rear=jtmp;
			}
		}
		
		tmp=tmp->next;
	}
	
	//printf("MyThreadJoinAll : Traversing queues finished... join_list updated... now move thread:%d to blocked queue and run the first ready thread  \n",curr_th->tid);
	
	//Need to move this thread into blocked Queue and run the first ready state
	//Fetching first ready state

	
	struct node * blocked_th=binsert_q(curr_th->cnxt,curr_th->tid,curr_th->pid,curr_th->join_th,curr_th->join_th_rear);
	
	//printf("MyThreadJoinAll : current thread:%d entered in BQ \n",curr_th->tid);
	
	int old_th=curr_th->tid;
	int old_pr=curr_th->pid;
	ucontext_t old_cnxt=curr_th->cnxt;


	ucontext_t new_cnxt;
	struct node * new_th=pop_q();

	if(new_th == NULL)
	{
		//printf(" MyThreadJoinAll: ready queue empty ... \n");
		swapcontext(&dmy_cnxt,&init_cnxt);
	}
	
	new_cnxt=new_th->cnxt;
	
	curr_th->tid=new_th->tid;
	curr_th->pid=new_th->pid;
	curr_th->cnxt=new_th->cnxt;
	curr_th->join_th=new_th->join_th;
	curr_th->join_th_rear=new_th->join_th_rear;
	
	//printf("MyThreadJoinAll: swap context from thread : %d to first ready thread : %d... \n",old_th,curr_th->tid);	
	
	if(swapcontext(&(blocked_th->cnxt),&new_cnxt)==-1)
	{
		curr_th->tid=old_th;
		curr_th->pid=old_pr;
		curr_th->cnxt=old_cnxt;
		curr_th->join_th=old_jfront;
		curr_th->join_th_rear=old_jrear;
		bremoveNode(curr_th->tid);
		//printf("MyThreadJoinAll: Blocking swapcontext error... \n");
	}
	
}
/*
Terminates the invoking thread. Note: all MyThreads are required to invoke this function. Do not allow functions to “fall out” of the start function. 
*/
void MyThreadExit(void)
{
	//printf("MyThreadExit: START ... for thread =%d \n",curr_th->tid);
	ucontext_t next_context;

	//Need to check, if terminating thread's parent is in blocked queue or not ...
	struct node *parent_th=bgetNode(curr_th->pid);
	
	if(parent_th!=NULL)
	{
		//printf("Parent thread = %d is in blocked queue, need to check whether it is blocked by this child thread = %d \n" , parent_th->tid,curr_th->tid);
		struct join_list *jtmp;		
		
		if(parent_th->join_th !=NULL)
		{
			if( (parent_th->join_th == parent_th->join_th_rear) && (parent_th->join_th->ch_tid == curr_th->tid))
			{ // Only one element in queue, and that's the one we are looking for
				//printf("Only One element in join_list, and it's a MATCH, deleting it, Queue is empty now ...\n");
				jtmp=parent_th->join_th;
				parent_th->join_th=NULL;
				parent_th->join_th_rear=NULL;
				free(jtmp);
			} else if (parent_th->join_th->ch_tid == curr_th->tid)
			{ // First element needs to be deleted, and Queue have more than one elements...
				//printf("First element of queue is a MATCH, queue have more than one element, shifting Front pointer of join_list ...\n");
				jtmp=parent_th->join_th;
				parent_th->join_th=parent_th->join_th->link;
				free(jtmp);
			} else 
			{// Need to traverse through queue, to check for the element
				//printf("Need to traverse through,  join_list of parent thread = %d , to look for child thread = %d \n",parent_th->tid,curr_th->tid);
				jtmp=parent_th->join_th;
				struct join_list *jprev=NULL;

				while(jtmp!=NULL && (jtmp->ch_tid != curr_th->tid))
				{
					jprev=jtmp;
					jtmp=jtmp->link;
				}	

				if(jtmp!=NULL && (jtmp->ch_tid == curr_th->tid))
				{	
					//printf("Child thread =%d in join_list of parent thread=%d, removing it from list ...\n",curr_th->tid,parent_th->tid);
					jprev->link=jtmp->link;
					free(jtmp);
				} else if(jtmp==NULL)
				{
					//printf("This terminating thread = %d is not blocking its parent thread= %d...\n",curr_th->tid,parent_th->tid);
				}
		
			}	

			//Now, if this parent thread's waiting child thread join_list becomes empty, it is ready to execute and therefore need to insert into ready queue...
			if(parent_th->join_th ==NULL)
			{
				//printf(" Parent thread: %d , join list is empty, It is ready to execute...inserting into ready queue \n",parent_th->tid);
				struct node *blocked_to_ready=insert_q(parent_th->cnxt,parent_th->tid,parent_th->pid,parent_th->join_th,parent_th->join_th_rear);
				//printf(" Thread = %d moved to ready Queue \n ",blocked_to_ready->tid);
				bremoveNode(parent_th->tid);
				//printf(" Thread = %d removed from blocked Queue \n",blocked_to_ready->tid);
			}

		}		

	}
	
	if(front==NULL)
	{
		//printf("No ready threads pending...\n");
		swapcontext(&dmy_cnxt,&init_cnxt);
	}
	
	//printf("MyThreadExit: fetching first ready state ... \n");
	struct node * new_th=pop_q();

	if(new_th == NULL)
	{
		//printf(" MyThreadExit: ready queue empty ... \n");
		swapcontext(&dmy_cnxt,&init_cnxt);
	}
	
	next_context=new_th->cnxt;
	
	int old_th=curr_th->tid;
	int old_pr=curr_th->pid;
	ucontext_t old_cnxt=curr_th->cnxt;
	struct join_list *old_jfront=curr_th->join_th;
	struct join_list *old_jrear=curr_th->join_th_rear;
	
	
	curr_th->tid=new_th->tid;
	curr_th->pid=new_th->pid;
	curr_th->cnxt=new_th->cnxt;
	curr_th->join_th=new_th->join_th;
	curr_th->join_th_rear=new_th->join_th_rear;

	//printf("MyThreadExit : Terminating invoking thread : %d ,  and running next thread : %d .... \n",old_th,new_th->tid);
	//printf("MyThreadExit: Thread=%d set terminated... , New thread=%d , context = %d \n",old_th,new_th->tid, new_th->cnxt);
	//printf("MyThreadExit: swap context from =%d , to  = %d  which is also same as next_cnxt=%d\n ",old_cnxt,new_th->cnxt,next_context);
		
	if(-1==swapcontext(&(old_cnxt),&(new_th->cnxt)))
	{
		//printf("MyThreadExit: setcontext error...\n");
		curr_th->tid=old_th;
		curr_th->pid=old_pr;
		curr_th->cnxt=old_cnxt;
		curr_th->join_th=old_jfront;
		curr_th->join_th_rear=old_jrear;
		//printf("MyThreadExit: setcontext error RECOVERY done...\n");	
	}

	//printf("MyThreadExit: exit for thread=",curr_th->tid);
}

void MyThreadInit(void(*start_funct)(void *), void *args)
{
	ucontext_t curr_context;
	//printf("MyThreadInit: START ... \n ");
	
	if(0!=getcontext(&curr_context))
	{
		//printf("MyThreadInit : getcontext error...\n ");
	}
	
	curr_context.uc_stack.ss_sp=(char *)malloc(STACKSIZE);
	if(curr_context.uc_stack.ss_sp==NULL)
	{
		//printf("No memory allocated... Maximum thread limit reached...returning gracefully...\n");
		return;
	}
	curr_context.uc_stack.ss_size=STACKSIZE;
	curr_context.uc_stack.ss_flags=0;
	curr_context.uc_link=NULL;
	//printf("MyThreadInit: makecontext... \n");
	
	makecontext(&curr_context,(void (*)(void))start_funct,1,args);
	//printf("MyThreadInit: makecontext  : DONE... \n");
	
	curr_th=(struct node *)malloc(sizeof(struct node));
	if(curr_th==NULL)
	{
		//printf("No memory allocated... Maximum thread limit reached...returning gracefully...\n");
		return;
	}
	curr_th->cnxt=curr_context;
	curr_th->tid=0;
	curr_th->pid=-1;
	curr_th->join_th=NULL;
	curr_th->join_th_rear=NULL;
	curr_th->next=NULL;
	
	//printf("MyThreadInit: MAIN THREAD = 0, context =%d ... \n",curr_th->cnxt); 
	if(swapcontext(&init_cnxt,&curr_context)==-1)
	{
		//printf("MythreadInit: setcontext error...\n");	
	}
	
	//printf("MyThreadInit: EXIT : back to init context... \n");
}

//semaphore routines...

struct sema
{
	int sid;
	int val;
	struct sb_thread *sb_front;
	struct sb_thread *sb_rear;
	struct sema* other;
};

struct sb_thread
{
	int sb_id;
	struct sb_thread *sb_next;
};

int smc=0;
struct sema *sfront;
struct sema *srear;

struct sema * get_sem(struct sema * sem)
{
	if(sem==NULL)
	{
		//printf("semaphore lookup for NULL value... \n");
		return NULL;
	}
	struct sema *tmp=sfront;
	
	while(tmp!=NULL && tmp->sid != sem->sid)
	{
		tmp=tmp->other;
	}
	
	if(tmp!=NULL && tmp->sid==sem->sid)
	{
		return tmp;
	} else if (tmp==NULL)
	{
		//printf("Semaphore not found in lookup... \n");
	}

	return NULL;
}

//Create a semaphore. Set the initial value to initialValue, which must be non-negative. 
//A positive initial value has the same effect as invoking MySemaphoreSignal the same number of times. On error it returns NULL.

MySemaphore MySemaphoreInit(int initialValue)
{
	//printf("MySemaphoreInit : Enter... initialValue= %d \n",initialValue);
	if(initialValue < 0)
	{
		//printf(" please set positive value to semaphore... \n");
		return NULL;
	}

	MySemaphore MySem;

	struct sema *tmp=(struct sema *)malloc(sizeof(struct sema));
	if(tmp==NULL)
	{
		//printf(" MySemaphoreInit : Memory not allocated...max limit reached for threads....returning gracefully...\n");
		swapcontext(&dmy_cnxt,&init_cnxt);
	}
	
	tmp->sid=++smc;
	tmp->val=initialValue;
	tmp->sb_front=NULL;
	tmp->sb_rear=NULL;
	tmp->other=NULL;

	if(sfront==NULL)
	{
		sfront=tmp;
		srear=sfront;
	}else
	{
		srear->other=tmp;
		srear=tmp;
	}

	MySem=(MySemaphore)srear;

	return MySem;
}

//Signal semaphore sem. The invoking thread is not pre-empted.

void MySemaphoreSignal(MySemaphore sem)
{
   struct sema *passed_sem=(struct sema *)sem;
	struct sema *c_sem=get_sem(passed_sem);

	if(c_sem==NULL)
	{
		//printf("MySemaphoreSignal : sem is NULL.... returning.... \n");
		return;
	}
	
	//printf("MySemaphoreSignal : Enter... for sem=%d \n",c_sem->sid);
	
	if(c_sem->val>0)
	{
		//printf("MySemaphoreSignal : sem=%d is >0... simply incrementing...new val=%d \n",c_sem->sid,(c_sem->val+1));
		c_sem->val = (c_sem->val)+1;
	}
	else if(c_sem->val==0)
	{
		//if sb_thread queue empty, no thread is waiting...than increase semaphore...
		if(c_sem->sb_front==NULL)
		{
			c_sem->val=(c_sem->val)+1;
		}
		else
		{
			int sbth_id=-2;
		
			//pop the sb_thread queue, and get the first entered sb_thread id
			struct sb_thread *sb_tmp=c_sem->sb_front;
			
			if(c_sem->sb_front==c_sem->sb_rear)
			{
				c_sem->sb_front=NULL;
				c_sem->sb_rear=NULL;
			}
			else 
			{
				c_sem->sb_front=c_sem->sb_front->sb_next;
				
			}
			
			sbth_id=sb_tmp->sb_id;
		
			// No need to, make semaphore =1 now, untill first ready thread get access to it...
			//Time to shift first blocked thread into ready queue...look from MyThreadExit

			// First, get thread via thread id from blocked queue...
			struct node *sblocked_th=bgetNode(sbth_id);
			
			//second, insert thread into RQ...
			//printf(" MySemaphoreSignal : thread: %d , inserting into ready queue \n",sblocked_th->tid);
			struct node *sblocked_to_ready=insert_q(sblocked_th->cnxt,sblocked_th->tid,sblocked_th->pid,sblocked_th->join_th,sblocked_th->join_th_rear);
			//printf(" MySemaphoreSignal: Thread = %d moved to ready Queue \n ",sblocked_to_ready->tid);
			
			//Third, remove thread id from BQ...
			bremoveNode(sblocked_th->tid);
			//printf(" MySemaphoreSignal: Thread = %d removed from blocked Queue \n",sblocked_to_ready->tid);

			//printf(" MySemaphoreSignal: Moved thread= %d from blocked to ready queue...continuing current thread=%d \n",sbth_id,curr_th->tid);

		}	
	}
	
	//printf(" MySemaphoreSignal: EXIT ... \n");
}

//Wait on semaphore sem.

void MySemaphoreWait(MySemaphore sem)
{
   struct sema *passed_sem=(struct sema *)sem;
	struct sema *c_sem=get_sem(passed_sem);
	
	if(c_sem==NULL)
	{
		//printf("MySemaphoreWait : sem is NULL.... returning.... \n");
		return;
	}
	
	//printf("MySemaphoreWait : Enter... for sem=%d \n",c_sem->sid);
	
	if(c_sem->val>0)
	{
		//printf("MySemaphoreWait : resource available for c_sem=%d ...simply decrementing its value , now val=%d\n",c_sem->sid,(c_sem->val-1));
		c_sem->val = (c_sem->val)-1;
	}
	else if(c_sem->val==0)
	{
		//printf(" MySemaphoreWait : c_sem->val is already 0.... add this thread into sem's sblocked list...and then....Move current thread=%d to blocked queue... \n",curr_th->tid);
		// need to add curr_th into, this semaphore's FIFO blocked thread id list....

		struct sb_thread *tmp=(struct sb_thread *)malloc(sizeof(struct sb_thread));
		
		if(tmp==NULL)
		{
			//printf(" MySemaphoreWait : Memory not allocated...max limit reached for threads....returning gracefully...\n");
			swapcontext(&dmy_cnxt,&init_cnxt);
		}
		
		tmp->sb_id=curr_th->tid;
		tmp->sb_next=NULL;

		//printf("MySemaphoreWait : init tmp...\n");
		if(c_sem->sb_front == NULL)
		{
			c_sem->sb_front=tmp;
			c_sem->sb_rear=tmp;
		} else
		{
			c_sem->sb_rear->sb_next=tmp;
			c_sem->sb_rear=tmp;
		}

		// Move current thread to blocked queue, run first ready queue, lookup MyThreadJoin for reference....
		// POP first ready thread...

	
		// todo: code to move current thread to blocked queue...
	
		struct node * blocked_th=binsert_q(curr_th->cnxt,curr_th->tid,curr_th->pid,curr_th->join_th,curr_th->join_th_rear);
	
		//printf("MySemaphoreWait : current thread:%d entered in BQ , but actual current thread is= %d\n",blocked_th->tid, curr_th->tid);

		//saving current thread pointers for future failure recovery	
		int old_th=curr_th->tid;
		int old_pr=curr_th->pid;
		ucontext_t old_cnxt=curr_th->cnxt;
		struct join_list *old_jfront=curr_th->join_th;
		struct join_list *old_jrear=curr_th->join_th_rear;

		//printf("MySemaphoreWait : popping first RQ thread...\n");
		ucontext_t new_cnxt;
		struct node * new_th=pop_q();
		
		if(new_th == NULL)
		{
			//printf(" MySemaphoreWait: ready queue empty ... \n");
			swapcontext(&dmy_cnxt,&init_cnxt);
		}
	
		new_cnxt=new_th->cnxt;
		//updating current thread pointers
		curr_th->tid=new_th->tid;
		curr_th->pid=new_th->pid;
		curr_th->cnxt=new_th->cnxt;
		curr_th->join_th=new_th->join_th;
		curr_th->join_th_rear=new_th->join_th_rear;
	
		//printf("MySemaphoreWait: swap context from thread : %d  to first ready thread : %d ... \n",old_th,curr_th->tid);
	
		if(swapcontext(&(blocked_th->cnxt),&new_cnxt)==-1)
		{
			//printf("MySemaphoreWait: Blocking swapcontext error... \n");
			curr_th->tid=old_th;
			curr_th->pid=old_pr;
			curr_th->cnxt=old_cnxt;
			curr_th->join_th=old_jfront;
			curr_th->join_th_rear=old_jrear;
			bremoveNode(curr_th->tid);
			//printf("MySemaphoreWait: Blocking swapcontext error recovery done...\n");
		}
	
		// After resuming from ready - queue,
		//printf(" MySemaphoreWait: Semaphore wait over, for current thread= %d ....\n",curr_th->tid);
		
	}
	//printf(" MySemaphoreWait : EXIT .... \n");
}


//Destroy semaphore sem. Do not destroy semaphore if any threads are blocked on the queue. Return 0 on success, -1 on failure.

int MySemaphoreDestroy(MySemaphore sem)
{

	if(sem==NULL)
	{
		//printf(" MySemaphoreDestroy : sem is NULL....returning... \n");
		return 0;
	}

	struct sema *passed_sem=(struct sema *)sem;
	struct sema *c_sem=get_sem(passed_sem);
	
	//printf(" MySemaphoreDestroy : enter for sem=%d and fetched sem=%d.... \n",passed_sem->sid,c_sem->sid);
	
	//Do not destroy semaphore if any threads are blocked on the queue. Return 0 on success, -1 on failure.
	if(c_sem->sb_front!=NULL)
	{
		//printf("MySemaphoreDestroy: this sem's sblocked queue is not empty.... So not destroying... \n");
		return -1;
	}


	//Now delete the sem, from Semaphore list...
	
	struct sema *stmp=NULL;
	struct sema *sprev=NULL;

	//only one sem, and that's to be deleted...	
	if(sfront==srear && sfront->sid==c_sem->sid)
	{
		stmp=sfront;
		sfront=NULL;
		srear=NULL;
		free(stmp);
	} else if(sfront->sid==c_sem->sid)
	{ // more than one sem, but first one to be deleted...
		stmp=sfront;
		sfront=sfront->other;
		free(stmp);
	} else 
	{ // Not first sem match, need to traverse through list....
		stmp=sfront;
		
		while(stmp!=NULL && stmp->sid!=c_sem->sid)
		{
			sprev=stmp;
			stmp=stmp->other;
		}
		
		if(stmp!=NULL && stmp->sid==c_sem->sid)
		{
			sprev->other=stmp->other;
			free(stmp);
		} else if(stmp==NULL)
		{
			//printf("MySemaphoreDestroy: sem= %d does not exist... \n",c_sem->sid);
		}
	}
	
	//printf(" MySemaphoreDestroy : EXIT .... \n");
	
	return 0;
	
}
