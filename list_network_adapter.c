#include <ifaddrs.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>

int main() {

  struct ifaddrs *addresses;

  if (getifaddrs(&addresses) == -1) { //allocates memory and fills in a linked list of addresses.
   printf("getifaddrs call failed\n");
    return -1;
  }

  struct ifaddrs *address = addresses; //to walk through the linked list of addresses

  while (address) {
    int family = address->ifa_addr->sa_family;
    if (family == AF_INET || family == AF_INET6) {
      printf("%s\t", address->ifa_name);
      printf("%s\t", family == AF_INET ? "IPv4" : "IPv6");
      char ap[100]; //define a buffer to store the textual address.
      const int family_size = family == AF_INET ? sizeof(struct sockaddr_in)
                                                : sizeof(struct sockaddr_in6);
      getnameinfo(address->ifa_addr, family_size, ap, sizeof(ap), 0, 0,
                  NI_NUMERICHOST);
      printf("\t%s\n", ap);
    }
    address = address->ifa_next;
  }
  freeifaddrs(addresses);
  return 0;
}
