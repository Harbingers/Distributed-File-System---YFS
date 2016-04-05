// FIFO interface

#ifndef fifo_h
#define fifo_h

// if queue is FULL, blocks enq()
// if queue is Empty, blocks deq()

#include <time.h>
#include <errno.h>

#include <list>

#include "lang/verify.h"
#include "scoped_lock.h"

template<class T>
class fifo {
    private:
		std::list<T> q_;
		pthread_mutex_t m_;
		pthread_cond_t non_empty_c_; // queue went non-empty
		pthread_cond_t has_space_c_; // queue is no longer overfull
		unsigned int size_limit_; //maximum capacity of the queue, block enq threads if exceeds this limit

	public:
		fifo(int m=0);
		~fifo();
		bool enq(T, bool blocking=true);
		void deq(T *);
		bool size();
};

// constructor with size limitation implementation
template<class T>
fifo<T>::fifo(int limit) : size_limit_(limit) {
	VERIFY(pthread_mutex_init(&m_, 0) == 0);
	VERIFY(pthread_cond_init(&non_empty_c_, 0) == 0);
	VERIFY(pthread_cond_init(&has_space_c_, 0) == 0);
}

// destructor implementation
template<class T>
fifo<T>::~fifo() {
	//fifo is to be deleted only when no threads are using it!
	VERIFY(pthread_mutex_destroy(&m_)==0);
	VERIFY(pthread_cond_destroy(&non_empty_c_) == 0);
	VERIFY(pthread_cond_destroy(&has_space_c_) == 0);
}

template<class T> bool
fifo<T>::size() {
	ScopedLock ml(&m_);
	return q_.size();
}

