/*
 1. rpcc: client-side RPC.
  1)  Each rpcc is bound to a single RPC server.
  2)  Jobs of rpcc
      - maintaining a connection to server
      - sending RPC requests and waiting for responses
      - retransmissions
      - at-most-once delivery

 2. rpcs: server side of RPC.
   1)  Each rpcs handles multiple connections from different rpcc objects.
   2)  Jobs of rpcs:
      - accepting connections
      - dispatching requests to registered RPC handlers
      - at-most-once delivery etc.

 3. connection & rpcc/rpcs
   1)  connection class is the underlying communication channel.
   2)  To send RPC request/reply, call connection::send(),this will block until data is sent or the connection has failed
   3)  When a request/reply is received, connection makes a callback into rpcc/rpcs (see rpcc::got_pdu() and rpcs::got_pdu()).

 4. Thread organization:
   1)  rpcc uses application threads to send RPC requests and blocks to receive the reply or error.
   2)  All connections use a single PollMgr object to perform async socket IO.
   3)  PollMgr creates a single thread to examine the readiness of socket file descriptors and informs
       the corresponding connection whenever a socket is ready to be read or written.
   4)  We use asynchronous socket IO to reduce the number of threads needed to manage these connections;
       without async IO, at least one thread is needed per connection to read data without blocking other activities.
   5)  Each rpcs object creates one thread for listening on the server port and a pool of threads for executing RPC requests.
   6)  The thread pool allows us to control the number of threads spawned at the server
   7)  spawning one thread per request will hurt when the server faces thousands of requests.
   8)  To delete a connection object, need a reference count.
       -- For rpcc: multiple client threads might hold multiple references to the underlying connection object.
       -- For rpcs: multiple dispatch threads might hold references to the same connection object.
       -- A connection object is deleted only when the underlying connection is dead and the reference count reaches zero.
   9)  The previous version of the RPC library uses pthread_cancel* routines to implement the deletion of rpcc and rpcs objects.
       The idea is to cancel all active threads that might be holding a reference to an object before deleting that object.
       However, pthread_cancel is not robust and there are always bugs where outstanding references to deleted objects persist.
       This version of the RPC library does not do pthread_cancel, but explicitly joins exited threads to make sure
       no outstanding references exist before deleting objects.
   10) To delete a rpcc: users must ensure that there are no outstanding calls on the rpcc object.
   11) To delete a rpcs:
       -- stop accepting new incoming connections.
       -- close existing active connections.
       -- delete the dispatch thread pool which involves waiting for current active RPC handlers to finish.
   12) It is interesting how a thread pool can be deleted without using thread cancellation.
       The trick is to inject x "poison pills" for a thread pool of x threads.
       Upon getting a poison pill instead of a normal task, a worker thread will exit
       and thread pool destructor waits to join all x exited worker threads.
 */
