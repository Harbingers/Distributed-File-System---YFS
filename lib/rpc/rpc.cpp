/*
 1. rpcc: client-side RPC.
  1) Each rpcc is bound to a single RPC server.
  2) Jobs of rpcc
    - maintaining a connection to server
    - sending RPC requests and waiting for responses
    - retransmissions
    - at-most-once delivery

 2. rpcs: server side of RPC.
   1) Each rpcs handles multiple connections from different rpcc objects.
   2) Jobs of rpcs:
    - accepting connections
    - dispatching requests to registered RPC handlers
    - at-most-once delivery etc.

 3. connection & rpcc/rpcs
   1) connection class is the underlying communication channel.
   2) To send RPC request/reply, call connection::send(),this will block until data is sent or the connection has failed
   3) When a request/reply is received, connection makes a callback into rpcc/rpcs (see rpcc::got_pdu() and rpcs::got_pdu()).
