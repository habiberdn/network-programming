#include "chap03.h"
#include <ctype.h>
#include <stdio.h>

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
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  struct addrinfo *bind_address;
  getaddrinfo(0, "8080", &hints, &bind_address);
  printf("Creating socket...\n");
  SOCKET socket_listen;

  socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype,
                         bind_address->ai_protocol);
  if (!ISVALIDSOCKET(socket_listen)) {
    fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
    return 1;
  }
  printf("Binding socket to local address...\n");
  if (bind(socket_listen, // bind() our socket to the local address and have it
                          // enter a listening state
           bind_address->ai_addr, bind_address->ai_addrlen)) {
    fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
    return 1;
  }
  freeaddrinfo(bind_address);
  printf("Listening...\n");
  if (listen(socket_listen, 10) < 0) {
    fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
    return 1;
  }

  fd_set master; // file descriptor
  FD_ZERO(&master);
  FD_SET(socket_listen, &master);
  SOCKET max_socket = socket_listen; // holds the largest socket descriptor.
  printf("Waiting for connections...\n");

  while (1) {
    fd_set reads;
    reads = master; // copy data from master to add new connections, otherwise
                    // the data will lose
    if (select(max_socket + 1, &reads, 0, 0, 0) <
        0) { // manage multiple connections within a single thread to      see
             // if any of them are ready for I/O operations.
      fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
      return 1;
    }
    SOCKET i;
    for (i = 1; i <= max_socket;
         ++i) { // try every possible socket descriptor up to max_socket.
      if (FD_ISSET(i, &reads)) { // detect a new connections
        if (i == socket_listen) {
          printf("%d", i);
          struct sockaddr_storage client_address;
          socklen_t client_len = sizeof(client_address);
          SOCKET socket_client = accept( // accept new connections
              socket_listen, (struct sockaddr *)&client_address, &client_len);
          if (!ISVALIDSOCKET(socket_client)) {
            fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
            return 1;
          }
          FD_SET(socket_client, &master); // add new socket to master
          if (socket_client > max_socket)
            max_socket = socket_client;
          char address_buffer[100];
          getnameinfo((struct sockaddr *)&client_address, client_len,
                      address_buffer, sizeof(address_buffer), 0, 0,
                      NI_NUMERICHOST);
          printf("New connection from %s\n", address_buffer);
        } else {
          char read[1024]; 
          int bytes_received = recv(i, read, 1024, 0);
          if (bytes_received < 1) {
            FD_CLR(i, &master);
            CLOSESOCKET(i);
            continue;
          }
          int j;
          for (j = 0; j < bytes_received; ++j)
            read[j] = toupper(read[j]);
          send(i, read, bytes_received, 0);
        } 
      } // if FD_ISSET
    } // for i to max_socket
  } // while(1)
  printf("Closing listening socket...\n");
  CLOSESOCKET(socket_listen);
#if defined(_WIN32)
  WSACleanup();
#endif
  printf("Finished.\n");
  return 0;
}
