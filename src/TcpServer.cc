//**********************************************************************
// file name: TcpServer.cc
//**********************************************************************

#include "TcpServer.h"
#include <stdio.h>
#include <errno.h>

/**************************************************************************

  Name: TcpServer

  Purpose: The purpose of this function is to serve as the constructor
  of a TcpServer object.  This function opens a socket, and if
  successful, populates the peer address structure with the appropriate
  information.

  Calling Sequence: TcpServer(port,receiveCallbackPtr)

  Inputs:

    port - The port number for this server to listen to.

    receiveCallbackPtr - A pointer to a callback function that will
    be invoked when a UDP datagram arrives.

  Outputs:

    None.

**************************************************************************/
TcpServer::TcpServer(
  int port,
  void (*handlerProcedurePtr)(int socketDescriptor))
{
  int status, option;

  if (port < 0)
  {
    // Keep it sane.
    port = 32000;
  } // if

  // This is the port for which the server will listen.
  this->listenerPort = port;

  // This is a pointer to the callbck handler procedure.
  this->handlerProcedurePtr = handlerProcedurePtr;

  // This allows the server to run.
  timeToExit = false;

  // Indicate that a peer is not connected.
  peerConnected = false;

  // Create socket.
  fileDescriptor = socket(PF_INET,SOCK_STREAM,0);

  //*******************************************************************
  // If the file descriptor is valid, a socket has been created.  Now
  // we set up the parameters that are needed for the bind() call.
  //*******************************************************************
  if (fileDescriptor != -1)
  {
    // This is used for setting socket options.
    option = 1;

    // Set up to reuse the port number.
    status = setsockopt(fileDescriptor,SOL_SOCKET,SO_REUSEADDR,
         (char *)&option,sizeof(option));

    if (status != -1)
    {
      // Set up the address structure.
      bzero((char *)&addr,sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = inet_addr("0.0.0.0");
      addr.sin_port = htons(listenerPort);

      // Associate the address with the socket.
      status = bind(fileDescriptor,(struct sockaddr *)&addr,sizeof(addr));
    } // if

    if (status != -1)
    {
      // Accept only one connection.
      status = listen(fileDescriptor,1);
    } // if

  // This is needed needed by the accept() call.
  clientAddressLength = sizeof(addr);

  // Start the server loop.
  pthread_create(&serverThread,0,
                 (void *(*)(void *))serverProcedure,this);

  } // if
  else
  {
    // clean up the mess
    close(fileDescriptor);
  } // else

} // TcpServer

/**************************************************************************

  Name: ~TcpServer

  Purpose: The purpose of this function is to serve as the destructor
  of a TcpServer object.

  Calling Sequence: ~TcpServer()

  Inputs:

    None.

  Outputs:

    None.

**************************************************************************/
TcpServer::~TcpServer(void)
{

  // Notify the server procedure to terminate.
  timeToExit = true;

  // Wait for the procedure to terminate.
  pthread_join(serverThread,0);

  if (fileDescriptor != 0)
  {
    close(fileDescriptor);
  } // if

  return;
 
} // ~TcpServer

/*****************************************************************************

  Name: waitForCliConnection

  Purpose: The purpose of this function is to wait for a client
  connection o complete. It provides synchronization for higher
  level code.

  Calling Sequence: waitForCliConnection(socketDescriptor)

  Inputs:

    socketDescriptor - A descriptor for the listener socket..

  Outputs:

    None.

*****************************************************************************/
void TcpServer::waitForCliConnection(int socketDescriptor)
{
  bool done;
  int status;
  fd_set connectSet;
  struct timeval timeout;

  // Set up for loop entry.
  done = false;

  while ((!timeToExit) && (!done))
  {
    // Ensure that we're clean.
    FD_ZERO(&connectSet);

    // Indicate the descriptor of interest.
    FD_SET(socketDescriptor,&connectSet);

    // Set timeout to 1 second.
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    // See which descriptors are available for reading.
    status = select(socketDescriptor+1,&connectSet,0,0,&timeout);

    if (status != -1)
    {
      if (FD_ISSET(socketDescriptor,&connectSet))
      {
        // Bail out.
        done = true;
      } // if
    } // if
  } // while

  return;

} // waitForCliConnection

/*****************************************************************************

  Name: serviceCliConnection

  Purpose: The purpose of this function is to service estabishment of a
  remote connection. An application-provided callback function is
  invoked for processing of the new connection.

  Calling Sequence: serviceCliConnection()

  Inputs:

    None.

  Outputs:

    None.

*****************************************************************************/
void TcpServer::serviceCliConnection(void)
{
  struct timeval timeout;

  // Indicate that a connection has been established.
  peerConnected = true;

  if (handlerProcedurePtr != NULL)
  {
    // Notify the client app of a new connection.
    handlerProcedurePtr(connectFileDescriptor);
  } // if

  // Wait until either it is time to exit or the connection is dropped by
  // the link partner.
  while ((peerConnected) && (!timeToExit))
  {
    // Set up to block for 100ms.
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

    // Wait for the designated time interval.
    select(0,0,0,0,&timeout);
  } // while

  return;

} // serviceCliConnection

/*****************************************************************************

  Name: peerDisconnected

  Purpose: The purpose of this function is is to notify the server
  processing routine that the link partner has terminated a connection.

  Calling Sequence: peerDisconnected()

   Inputs:

     None.

   Outputs:

     None.

*****************************************************************************/
void TcpServer::peerDisconnected(void)
 { 

  // Indicate that the link partner is gone.
  peerConnected = false;

  // Close our side of the connection.
  close(connectFileDescriptor);

  // Keep it safe.
  connectFileDescriptor = 0;

  return;

} // peerDisconnected

/*****************************************************************************

  Name: serverProcedure

  Purpose: The purpose of this function is to service estabishment of a
  remote connection. An application-provided callback function is
  invoked for processing of the new connection.

  Calling Sequence: serverProcedure(arg)

  Inputs:

    arg - A pointer to an argument that is supplied when the associated
   thread is launched.

  Outputs:

    None.

*****************************************************************************/
void TcpServer::serverProcedure(void *arg)
{
  TcpServer *mePtr;

  // Save  the pointer to the TcpServer object.
  mePtr = (TcpServer *)arg;

  //**********************************************************************
  // This is the main server loop.  The server will block on the
  // accept() call until a connection is made by a client.  Once a
  // connection is established, the connection will be serviced, and no
  // other processing will occur.  Note that this loop is traversed only
  // once per connect/disconnect sequence, since this is an interative
  // server that services only one client connection at a time.
  //**********************************************************************
  while (!mePtr->timeToExit)
  {
    // Wait until a client is attempting to connect.
    mePtr->waitForCliConnection(mePtr->fileDescriptor);

    //************************************************************
    // This block of code will accept the incoming connection.
    // If the system call is interrupted for some reason, another
    // attempt will be made to accept connections.
    //************************************************************
    do
    {
      // Accept the incoming connection.
      mePtr->connectFileDescriptor =
         accept(mePtr->fileDescriptor,
                (sockaddr *)&mePtr->addr,
                &mePtr->clientAddressLength);


      if (mePtr->connectFileDescriptor == -1)
      {
        if (errno == EINTR)
        {
          continue;
        } /* if */
      } // if
    } // do
    while (mePtr->connectFileDescriptor == -1);

    // Do the appropriate processing for the new connection.
    mePtr->serviceCliConnection();

    // We're done with this connection.
    close(mePtr->connectFileDescriptor);

  } // while

  pthread_exit(0);

} // serverProcedure
