#include <stdio.h>

void increment(int *num) {
    (*num)++;
}

int main() {
  const char *response =
    "HTTP/1.1 200 OK\r\n"
    "Connection: close\r\n"
    "Content-Type: text/plain\r\n\r\n"
    "Local time is: ";
  printf("%s\n", response);
  printf("%s\n", *response);
    return 0;
}
