#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int receivefile(char *listenip, int port, char *filename);

int main(int argc, char *argv[])
{
  if (argc != 4)
  {
    printf("client <listenip> <port> <filename>\n");
    return 1;
  }

  char *listenip = argv[1];
  int port = atoi(argv[2]);
  char *filename = argv[3];

  int result = receivefile(listenip, port, filename);
  if (result != 0)
  {
    return result;
  }

  return 0;
}

int receivefile(char *listenip, int port, char *filename)
{
  size_t bufsize = 3;
  unsigned char *buffer = malloc(bufsize);
  if (buffer == NULL)
  {
    printf("Out of memory.");
    return 1;
  }

  int sd = socket(AF_INET, SOCK_STREAM, 0);
  if (sd == -1)
  {
    printf("Couldn't create socket.");
    return 1;
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(listenip);
  server.sin_port = htons(port);

  if (bind(sd, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    printf("Bind failed.\n");
    return 1;
  }
  printf("Bind done.\n");

  listen(sd, 3);

  printf("Waiting for incoming connections...\n");
  int c = sizeof(struct sockaddr_in);
  struct sockaddr_in client;
  int new_socket = accept(sd, (struct sockaddr *)&client, (socklen_t *)&c);
  if (new_socket < 0)
  {
    printf("Accept failed.\n");
    return 1;
  }

  printf("Connection accepted.\n");

  int fd = -1;

  for (long offset = 0;; offset += bufsize)
  {
    ssize_t size = recv(new_socket, buffer, bufsize, 0);
    if (size < 0)
    {
      printf("Recv failed.\n");
      return 1;
    }
    if (size == 0 && fd >= 0)
    {
      printf("End.\n");
      break;
    }
    printf("Data received: %ld\n", size);

    if (fd < 0)
    {
      printf("Opening file: '%s'\n", filename);
      fd = open(filename, O_CREAT | O_WRONLY);
      if (fd < 0)
      {
        printf("Couldn't open file: '%s'\n", filename);
        return 1;
      }
    }

    printf("Writing: %ld\n", size);
    write(fd, buffer, size);
  }

  printf("Deallocating buffer.\n");
  free(buffer);
  printf("Closing file.\n");
  close(fd);
  printf("Closing socket.\n");
  close(sd);
  printf("Closing socket.\n");
  close(new_socket);

  return 0;
}
