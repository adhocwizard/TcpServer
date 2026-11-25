//**********************************************************************
// file name: udpServer.cc
//**********************************************************************

//*************************************************************************
// This program provides the functionality of a TCP server.  When a
// IP datagram arrives, the payload of the datagram will be sent to
// stdout.  This allows the output to be piped to another program.
//
// To run this program type,
// 
//     ./tcpServer -p <listenPort>
//
// where,
//
//    listenPort - the port number for which the server will listen.
//*************************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "TcpServer.h"
#include <pthread.h>

#define DEFAULT_LISTEN_PORT (40000)

// This structure is used to consolidate user parameters.
struct MyParameters
{
  int *listenPortPtr;
};

pthread_t connectionThread;
TcpServer *networkInterfacePtr;
int fd;
char buffer[200];

/**************************************************************************

  Name: connectionProcedure

  Purpose: The purpose of this function is to handle remote connection
  data from a TcpServer object.

  Calling Sequence: connectionProcedure(arg)

  Inputs:

    arg - A pointer to an argument.

  Outputs:

    None.

**************************************************************************/
void connectionProcedure(void *arg)
{
  int *socketDescriptorPtr;
  size_t count;
  bool done;

  socketDescriptorPtr = (int *)arg; 
  fd = *socketDescriptorPtr;

  // Set up for loop entry.
  done = false;

  while (!done)
  {
    count = read(fd,buffer,85);

    // Did link partner closed the link?
    if (count == 0)
    {
      // Notify server of link partner disconnect.
      networkInterfacePtr->disconnect();

      // Bail out.
      done = true;
    } // if
    else
    {
      // Echo to link partner.
      write(fd,buffer,count);
    } // else
  } // while

  pthread_exit(0);

} // connectionProcedure

/**************************************************************************

  Name: connectCallback

  Purpose: The purpose of this function is to handle remote connection
  events from a TcpServer object.

  Calling Sequence: connectCallback(socketDescripTor)

  Inputs:

    socketDescripTor - A file descriptor associated with the remote
    connection.

  Outputs:

    None.

**************************************************************************/
static void connectCallback(int socketDescriptor)
{

  pthread_create(&connectionThread,0,
                 (void *(*)(void *))connectionProcedure,&socketDescriptor);

  return;

} // receiveCallback

/*****************************************************************************

  Name: getUserArguments

  Purpose: The purpose of this function is to retrieve the user arguments
  that were passed to the program.  Any arguments that are specified are
  set to reasonable default values.

  Calling Sequence: exitProgram = getUserArguments(parameters)

  Inputs:

    parameters - A structure that contains pointers to the user parameters.

  Outputs:

    exitProgram - A flag that indicates whether or not the program should
    be exited.  A value of true indicates to exit the program, and a value
    of false indicates that the program should not be exited..

*****************************************************************************/
bool getUserArguments(int argc,char **argv,struct MyParameters parameters)
{
  bool exitProgram;
  bool done;
  int opt;

  // Default not to exit program.
  exitProgram = false;

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Default parameters.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Default a resonable port.
  *parameters.listenPortPtr = DEFAULT_LISTEN_PORT;
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  // Set up for loop entry.
  done = false;

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Retrieve the command line arguments.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  while (!done)
  {
    // Retrieve the next option.
    opt = getopt(argc,argv,"p:h");

    switch (opt)
    {
      case 'p':
      {
        *parameters.listenPortPtr = atoi(optarg);
        break;
      } // case

      case 'h':
      {
        // Display usage.
        fprintf(stderr,"./TdpServer -p <listenport>\n");

        // Indicate that program must be exited.
        exitProgram = true;
        break;
      } // case

      case -1:
      {
        // All options consumed, so bail out.
        done = true;
      } // case
    } // switch

  } // while
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  return (exitProgram);

} // getUserArguments

//*************************************************************************
// Mainline code.
//*************************************************************************
int main(int argc,char **argv)
{
  bool exitProgram;
  int listenPort;
  struct MyParameters parameters;

 // Set up for parameter transmission.
  parameters.listenPortPtr = &listenPort;

  // Retrieve the system parameters.
  exitProgram = getUserArguments(argc,argv,parameters);

  if (exitProgram)
  {
    // Bail out.
    return (0);
  } // if

  // Create tCP server object.
  networkInterfacePtr = new TcpServer(listenPort,connectCallback);

  while (1)
  {
  } // while

  // Clean up.
  if (networkInterfacePtr != NULL)
  {
    delete networkInterfacePtr;
  } // if

  return (0);

} // main
