/* This file is derived from source code for the Nachos
 instructional operating system.  The Nachos copyright notice
 is reproduced in full below. */

/* Copyright (c) 1992-1996 The Regents of the University of California.
 All rights reserved.
 
 Permission to use, copy, modify, and distribute this software
 and its documentation for any purpose, without fee, and
 without written agreement is hereby granted, provided that the
 above copyright notice and the following two paragraphs appear
 in all copies of this software.
 
 IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO
 ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE
 AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA
 HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
 BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
 MODIFICATIONS.
 */
#define NOT_IMPLEMENTED 0
#define WORKS 0
static int assert_count = 2;


#include "threads/synch.h"
#include <stdio.h>
#include <string.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

/* Initializes semaphore SEMA to VALUE.  A semaphore is a
 nonnegative integer along with two atomic operators for
 manipulating it:
 
 - down or "P": wait for the value to become positive, then
 decrement it.
 
 - up or "V": increment the value (and wake up one waiting
 thread, if any). */
void
sema_init (struct semaphore *sema, unsigned value) 
{
  ASSERT (sema != NULL);
	
  sema->value = value;
  list_init (&sema->waiters);
	//sema->holding = NULL;
	list_init (&sema->holders);
}

/* Down or "P" operation on a semaphore.  Waits for SEMA's value
 to become positive and then atomically decrements it.
 
 This function may sleep, so it must not be called within an
 interrupt handler.  This function may be called with
 interrupts disabled, but if it sleeps then the next scheduled
 thread will probably turn interrupts back on. */
void
sema_down (struct semaphore *sema) 
{
  enum intr_level old_level;
	
  ASSERT (sema != NULL);
  ASSERT (!intr_context ());
	
  old_level = intr_disable ();
  if (sema->value == 0) 
	{
			list_insert_ordered(&sema->waiters,&thread_current()->elem,&thread_priority_compare,NULL);
		struct thread* t = NULL;//list_entry(list_begin(&sema->holders),struct holder,elem)->t;//list_entry(list_begin(&sema->holders),struct thread,holder);
		
		int old_priority = 64;//if (t != NULL) old_priority = t->priority;
		int cur_priority = thread_current()->priority;
		if (old_priority < cur_priority) bump_priority(t,old_priority,cur_priority);
		/*{
			t->priority = cur_priority;
			
			if (t->status == THREAD_BLOCKED)
			{
				ASSERT(0);
			}
			else// if (&t->elem != NULL && t->elem.prev != NULL && t->elem.next != NULL)
			{
				adjust_order(&t->elem,&thread_priority_compare);
			}
			
			thread_block ();
			
			t->priority = old_priority;
		}*/
		else
		{
			thread_block ();
		}
		
	}
	
	//if (sema->holding == NULL) sema->holding = thread_current();
	
	holding_insert(&sema->holders,thread_current());//NEEDS CHANGED
	
	//ASSERT(assert_count--);
	
  sema->value--;
  intr_set_level (old_level);
}

void bump_priority(struct thread* t,int old_priority,int new_priority)
{
	ASSERT(NOT_IMPLEMENTED);
	t->priority = new_priority;
	adjust_order(&t->elem,&thread_priority_compare);
	thread_block ();
	t->priority = old_priority;
}


/*void nestedDonation(struct thread* t)
 {
 
 }*/
/* Down or "P" operation on a semaphore, but only if the
 semaphore is not already 0.  Returns true if the semaphore is
 decremented, false otherwise.
 
 This function may be called from an interrupt handler. */
bool
sema_try_down (struct semaphore *sema) 
{
  enum intr_level old_level;
  bool success;
	
  ASSERT (sema != NULL);
	
  old_level = intr_disable ();
  if (sema->value > 0) 
	{
		sema->value--;
		success = true; 
	}
  else
    success = false;
  intr_set_level (old_level);
	
  return success;
}

/* Up or "V" operation on a semaphore.  Increments SEMA's value
 and wakes up one thread of those waiting for SEMA, if any.
 
 This function may be called from an interrupt handler. */
void
sema_up (struct semaphore *sema) 
{
  enum intr_level old_level;
	
  ASSERT (sema != NULL);
	
  old_level = intr_disable ();
	
	holding_remove(&sema->holders,thread_current());
	
	//if (sema->holding == thread_current()) sema->holding = NULL;
	
	if (!list_empty (&sema->waiters)) 
		thread_unblock (list_entry (list_pop_front (&sema->waiters),
																struct thread, elem));
	sema->value++;
	intr_set_level (old_level);
}

static void sema_test_helper (void *sema_);

/* Self-test for semaphores that makes control "ping-pong"
 between a pair of threads.  Insert calls to printf() to see
 what's going on. */
void
sema_self_test (void) 
{
	struct semaphore sema[2];
	int i;
	
	printf ("Testing semaphores...");
	sema_init (&sema[0], 0);
	sema_init (&sema[1], 0);
	thread_create ("sema-test", PRI_DEFAULT, sema_test_helper, &sema);
	for (i = 0; i < 10; i++) 
	{
		sema_up (&sema[0]);
		sema_down (&sema[1]);
	}
	printf ("done.\n");
}

/* Thread function used by sema_self_test(). */
static void
sema_test_helper (void *sema_) 
{
	struct semaphore *sema = sema_;
	int i;
	
	for (i = 0; i < 10; i++) 
	{
		sema_down (&sema[0]);
		sema_up (&sema[1]);
	}
}

/* Initializes LOCK.  A lock can be held by at most a single
 thread at any given time.  Our locks are not "recursive", that
 is, it is an error for the thread currently holding a lock to
 try to acquire that lock.
 
 A lock is a specialization of a semaphore with an initial
 value of 1.  The difference between a lock and such a
 semaphore is twofold.  First, a semaphore can have a value
 greater than 1, but a lock can only be owned by a single
 thread at a time.  Second, a semaphore does not have an owner,
 meaning that one thread can "down" the semaphore and then
 another one "up" it, but with a lock the same thread must both
 acquire and release it.  When these restrictions prove
 onerous, it's a good sign that a semaphore should be used,
 instead of a lock. */
void
lock_init (struct lock *lock)
{
	ASSERT (lock != NULL);
	
	lock->holder = NULL;
	sema_init (&lock->semaphore, 1);
}

/* Acquires LOCK, sleeping until it becomes available if
 necessary.  The lock must not already be held by the current
 thread.
 
 This function may sleep, so it must not be called within an
 interrupt handler.  This function may be called with
 interrupts disabled, but interrupts will be turned back on if
 we need to sleep. */
void
lock_acquire (struct lock *lock)
{
	ASSERT (lock != NULL);
	ASSERT (!intr_context ());
	ASSERT (!lock_held_by_current_thread (lock));
	
	sema_down (&lock->semaphore);
	lock->holder = thread_current ();
}

/* Tries to acquires LOCK and returns true if successful or false
 on failure.  The lock must not already be held by the current
 thread.
 
 This function will not sleep, so it may be called within an
 interrupt handler. */
bool
lock_try_acquire (struct lock *lock)
{
	bool success;
	
	ASSERT (lock != NULL);
	ASSERT (!lock_held_by_current_thread (lock));
	
	success = sema_try_down (&lock->semaphore);
	if (success)
		lock->holder = thread_current ();
	return success;
}

/* Releases LOCK, which must be owned by the current thread.
 
 An interrupt handler cannot acquire a lock, so it does not
 make sense to try to release a lock within an interrupt
 handler. */
void
lock_release (struct lock *lock) 
{
	ASSERT (lock != NULL);
	ASSERT (lock_held_by_current_thread (lock));
	
	lock->holder = NULL;
	sema_up (&lock->semaphore);
}

/* Returns true if the current thread holds LOCK, false
 otherwise.  (Note that testing whether some other thread holds
 a lock would be racy.) */
bool
lock_held_by_current_thread (const struct lock *lock) 
{
	ASSERT (lock != NULL);
	
	return lock->holder == thread_current ();
}

/* One semaphore in a list. */
struct semaphore_elem 
{
	struct list_elem elem;              /* List element. */
	struct semaphore semaphore;         /* This semaphore. */
	int priority;
};

/* Initializes condition variable COND.  A condition variable
 allows one piece of code to signal a condition and cooperating
 code to receive the signal and act upon it. */
void
cond_init (struct condition *cond)
{
	ASSERT (cond != NULL);
	
	list_init (&cond->waiters);
}

/* Atomically releases LOCK and waits for COND to be signaled by
 some other piece of code.  After COND is signaled, LOCK is
 reacquired before returning.  LOCK must be held before calling
 this function.
 
 The monitor implemented by this function is "Mesa" style, not
 "Hoare" style, that is, sending and receiving a signal are not
 an atomic operation.  Thus, typically the caller must recheck
 the condition after the wait completes and, if necessary, wait
 again.
 
 A given condition variable is associated with only a single
 lock, but one lock may be associated with any number of
 condition variables.  That is, there is a one-to-many mapping
 from locks to condition variables.
 
 This function may sleep, so it must not be called within an
 interrupt handler.  This function may be called with
 interrupts disabled, but interrupts will be turned back on if
 we need to sleep. */
void
cond_wait (struct condition *cond, struct lock *lock) 
{
	struct semaphore_elem waiter;
	
	ASSERT (cond != NULL);
	ASSERT (lock != NULL);
	ASSERT (!intr_context ());
	ASSERT (lock_held_by_current_thread (lock));
	
	sema_init (&waiter.semaphore, 0);
	waiter.priority = thread_current()->priority;
	list_insert_ordered(&cond->waiters,&waiter.elem,&cond_priority_compare,NULL);
	lock_release (lock);
	sema_down (&waiter.semaphore);
	lock_acquire (lock);
}

/* If any threads are waiting on COND (protected by LOCK), then
 this function signals one of them to wake up from its wait.
 LOCK must be held before calling this function.
 
 An interrupt handler cannot acquire a lock, so it does not
 make sense to try to signal a condition variable within an
 interrupt handler. */
void
cond_signal (struct condition *cond, struct lock *lock UNUSED) 
{
	ASSERT (cond != NULL);
	ASSERT (lock != NULL);
	ASSERT (!intr_context ());
	ASSERT (lock_held_by_current_thread (lock));
	
	if (!list_empty (&cond->waiters)) 
		sema_up (&list_entry (list_pop_front (&cond->waiters),
													struct semaphore_elem, elem)->semaphore);
}

/* Wakes up all threads, if any, waiting on COND (protected by
 LOCK).  LOCK must be held before calling this function.
 
 An interrupt handler cannot acquire a lock, so it does not
 make sense to try to signal a condition variable within an
 interrupt handler. */
void
cond_broadcast (struct condition *cond, struct lock *lock) 
{
	ASSERT (cond != NULL);
	ASSERT (lock != NULL);
	
	while (!list_empty (&cond->waiters))
		cond_signal (cond, lock);
}

bool cond_priority_compare (const struct list_elem* one, const struct list_elem* two, void* aux UNUSED)
{
	struct semaphore_elem *c_one = list_entry (one, struct semaphore_elem, elem);
	struct semaphore_elem *c_two = list_entry (two, struct semaphore_elem, elem);
	return c_one->priority > c_two->priority;
}

bool holder_priority_compare (const struct list_elem* one, const struct list_elem* two, void* aux UNUSED)
{
	struct holder *h_one = list_entry (one, struct holder, elem);
	struct holder *h_two = list_entry (two, struct holder, elem);
	return h_one->t->priority > h_two->t->priority;
}
void holding_insert(struct list* list,struct thread* t)
{
	ASSERT(t!=NULL);
	static struct holder h;
	h.t = t;
	ASSERT(assert_count);
	list_insert_ordered(list,&h.elem,&holder_priority_compare,NULL);
	verifyList(list);
}
void holding_remove(struct list* list,struct thread* t)
{
	struct list_elem* elem = list_begin(list);
	
	while (elem->next != NULL) {
		if (list_entry(elem,struct holder,elem)->t == t)
		{
			list_remove(elem);
			break;
		}
		else
			elem = elem->next;
	}
	verifyList(list);
	//ASSERT(elem!=elem->next && ((elem->prev==NULL && elem->next==NULL) || (elem!=elem->prev && elem->prev!=elem->next)));
}

void verifyList(struct list* list)
{
	struct list_elem* elem = list_begin(list);
	while (elem->next != NULL) {
		ASSERT(elem != elem->next);
		ASSERT(elem != elem->prev);
		ASSERT(elem->prev != elem->next);
		ASSERT(elem->prev != NULL);
		ASSERT(elem->next != NULL);
		elem = elem->next;
	}
}

void adjust_order(struct list_elem* elem,list_less_func* order_by)
{
	if (elem->prev != NULL)
	{
		struct list_elem *e;
		
		for (e = elem->prev; e->prev != NULL; e = e->prev)
			if (!order_by (elem, e, NULL))
				break;
			else
			{
				e->next = elem->next;
				elem->next = e;
				elem->prev = e->prev;
				e->prev = elem;
			}
	}
}