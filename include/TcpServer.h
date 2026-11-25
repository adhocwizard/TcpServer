//**********************************************************************
// file name: TcpServer.h
//**********************************************************************

#ifndef _TCPSERVER_H_
#define _TCPSERVER_H_

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

class TcpServer
{
  public:

  TcpServer(int port,
            void (*handlerProcedurePtr)(int socketDescriptor));

  ~TcpServer(void);

  void peerDisconnected(void);

  private:

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Utility functions.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  void waitForCliConnection(int socketDescriptor);
  void serviceCliConnection(void);

  // This function performs the lidtening and peer connection launch.
  static void serverProcedure(void *arg);
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Attributes.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // This used to synchronize closing of hreads.
  bool timeToExit;

  // This indicates that a link psrtner has established a TCP connection.
  bool peerConnected;

  // The servere will be listening on this port.
  int listenerPort;

  // This is the socket descriptor for the listener.
  int fileDescriptor;

  // This descriptor is associated with the remote connection.
  int connectFileDescriptor;

  // Housekeeping stuff.
  struct sockaddr_in myAddress;
  socklen_t clientAddressLength;
  struct sockaddr_in addr;

  // Usd for tHread management.
  pthread_t serverThread;

  // Pointer to a callback function used when a connection is established.
  void (*handlerProcedurePtr)(int socketDescriptor);
};

#endif // _TCPSERVER_H_
