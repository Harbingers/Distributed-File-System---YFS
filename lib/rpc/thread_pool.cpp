#include <stdlib.h>
#include <errno.h>

#include "lang/verify.h"

#include "scoped_lock.h"
#include "thread_pool.h"

static void* do_worker(void *arg) {
	ThreadPool *tp = (ThreadPool *)arg;
	while (1) {
		ThreadPool::job_t j;
		if(!tp->takeJob(&j)) break; //die
		(void)(j.f)(j.a);
	}
	pthread_exit(NULL);
}

/**
when queue is full:
  if blocking: addJob() blocks;
  else: addJob() returns false;
**/
ThreadPool::ThreadPool(int size, bool blocking): nthreads_(size), blockadd_(blocking), jobq_(100*size) {
	pthread_attr_init(&attr_);
	pthread_attr_setstacksize(&attr_, 128<<10);

	for (int i = 0; i < size; i++) {
		pthread_t t;
		VERIFY(pthread_create(&t, &attr_, do_worker, (void *)this) ==0);
		th_.push_back(t);
	}
}

/**
 destructor implementation: this function can be called only when no external
 thread will ever use this thread pool again or is currently blocking on it
**/
ThreadPool::~ThreadPool() {
	for (int i = 0; i < nthreads_; i++) {
		job_t j;
		j.f = (void *(*)(void *))NULL; //poison pill to tell worker threads to exit
		jobq_.enq(j);
	}

	for (int i = 0; i < nthreads_; i++) {
		VERIFY(pthread_join(th_[i], NULL)==0);
	}

	VERIFY(pthread_attr_destroy(&attr_)==0);
}

bool ThrPool::addJob(void *(*f)(void *), void *a) {
	job_t j;
	j.f = f;
	j.a = a;

	return jobq_.enq(j,blockadd_);
}

bool ThrPool::takeJob(job_t *j) {
	jobq_.deq(j);
	return (j->f!=NULL);
}
