#pragma once

typedef int socklen_t;
typedef unsigned short int sa_family_t;

struct sockaddr {
  sa_family_t sa_family;
  char sa_data[];
};

struct sockaddr_storage {
  sa_family_t ss_family;
};

typedef struct fd_set {
  unsigned int fd_count;               /* how many are SET? */
  int fd_array[10];   /* an array of SOCKETs */
} fd_set;
