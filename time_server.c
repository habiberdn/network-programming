/*time_server.c*/
#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#endif

#if defined(_WIN32)
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())
#else
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>

int main() {
#if defined(_WIN32)
  WSADATA d;
  if (WSAStartup(MAKEWORD(2, 2), &d)) {
    fprintf(stderr, "Failed to initialize.\n");
    return 1;
  }
#endif
  printf("Configuring local address...\n");
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM; //indicate using TCP, SOCK_DGRAM for UDP
  hints.ai_flags = AI_PASSIVE; //bind to wildcard address
  struct addrinfo *bind_address; //hold return info from getaddrinfo
  getaddrinfo(0, "8080", &hints, &bind_address); //this func makes very easy to convert from ipv4 to ipv6(binding)
  printf("Creating socket...\n");
  SOCKET socket_listen;

  socket_listen = socket(bind_address->ai_family, //initiate socket
                         bind_address->ai_socktype, bind_address->ai_protocol); 

  printf("socket_listen: %d\n", socket_listen);
  if (!ISVALIDSOCKET(socket_listen)) {
    fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
    return 1;
  }
  printf("Binding socket to local address...\n");
  if (bind(socket_listen, //call bind() to associate it with our address from getaddrinfo()
           bind_address->ai_addr, bind_address->ai_addrlen)) {
    fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
    return 1;
  }
  freeaddrinfo(bind_address); //release the address memory.
  printf("Listening...\n");
  if (listen(socket_listen, 10) < 0) { //10 in this case, tells listen() how many connections it is allowed 
    //to queue up.
    fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
    return 1;
  }
  printf("Waiting for connection...\n");

  struct sockaddr_storage client_address; //store the address info for the connecting client
  socklen_t client_len = sizeof(client_address);
  SOCKET socket_client = accept(socket_listen, //accept will block your program until a new connection is made
                                (struct sockaddr*) &client_address, &client_len);
  if (!ISVALIDSOCKET(socket_client)) {
    fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
    return 1;
  }

  printf("Client is connected... ");
  char address_buffer[100];
  getnameinfo((struct sockaddr*)&client_address, //takes the client's address and address length.
              client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
  //NI_NUMERICHOST is for specifies that we want to see the hostname as an IP address.
  printf("%s\n", address_buffer);

  printf("Reading request...\n");
  char request[1024];
  //read request from HTTP
  int bytes_received = recv(socket_client, request, 1024, 0); //your program's execution blocks until new data is actually available.
  printf("Received %d bytes.\n", bytes_received);
  printf("%.*s", bytes_received, request);//print a specific number of characters—bytes_received

  printf("Sending response...\n");
  const char *response = //for header cant be mutate
    "HTTP/1.1 200 OK\r\n"
    "Connection: close\r\n"
    "Content-Type: text/plain\r\n\r\n"
    "Local time is: ";
  int bytes_sent = send(socket_client, response, strlen(response), 0); // send data to browser
  printf("Sent %d of %d bytes.\n", bytes_sent, (int)strlen(response));

  time_t timer;
  time(&timer);
  char *time_msg = ctime(&timer);
  bytes_sent = send(socket_client, time_msg, strlen(time_msg), 0); //send actual time 
  printf("Sent %d of %d bytes.\n", bytes_sent, (int)strlen(time_msg));

  printf("Closing connection...\n");
  CLOSESOCKET(socket_client); //close client connection
  
#if defined(_WIN32)
  WSACleanup();
#endif
  printf("Finished.\n");
  return 0;
}
