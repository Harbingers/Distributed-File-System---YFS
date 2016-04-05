// RPC interface

#ifndef rpc_h
#define rpc_h

#include <sys/socket.h>
#include <netinet/in.h>
#include <list>
#include <map>
#include <stdio.h>

#include "thr_pool.h"
#include "marshall.h"
#include "connection.h"

#ifdef DMALLOC // check for memory leak
#include "dmalloc.h"
#endif

class rpc_const {
	public:
		static const unsigned int bind = 1;   // handler number reserved for bind
		static const int timeout_failure = -1;
		static const int unmarshal_args_failure = -2;
		static const int unmarshal_reply_failure = -3;
		static const int atmostonce_failure = -4;
		static const int oldsrv_failure = -5;
		static const int bind_failure = -6;
		static const int cancel_failure = -7;
};
