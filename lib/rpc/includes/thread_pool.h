/**
 thread pool interface
**/

#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include <pthread.h>
#include <vector>

#include "fifo.h"

class ThreadPool {
    private:
		pthread_attr_t attr_;
		int nthreads_;
		bool blockadd_;

		fifo<job_t> jobq_;
		std::vector<pthread_t> th_;

		bool addJob(void *(*f)(void *), void *a);

	public:
		struct job_t {
			void *(*f)(void *); //function point
			void *a; //function arguments
		};

		ThreadPool(int size, bool blocking=true);
		~ThreadPool();
		template<class C, class A> bool addObjJob(C *o, void (C::*m)(A), A a);
		void waitDone();
		bool takeJob(job_t *j);
};

template <class C, class A>
bool
ThreadPool::addObjJob(C *o, void (C::*m)(A), A a) {
	class objfunc_wrapper {
		public:
			C *o;
			void (C::*m)(A a);
			A a;
			static void *func(void *vvv) {
				objfunc_wrapper *x = (objfunc_wrapper*)vvv;
				C *o = x->o;
				void (C::*m)(A ) = x->m;
				A a = x->a;
				(o->*m)(a);
				delete x;
				return 0;
			}
	};

	objfunc_wrapper *x = new objfunc_wrapper;
	x->o = o;
	x->m = m;
	x->a = a;
	return addJob(&objfunc_wrapper::func, (void *)x);
}

#endif
