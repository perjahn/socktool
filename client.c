#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int sendfile(char *serverip, int port, char *filename);

int main(int argc, char *argv[])
{
  if (argc != 4)
  {
    printf("client <serverip> <port> <filename>\n");
    return 1;
  }

  char *serverip = argv[1];
  int port = atoi(argv[2]);
  char *filename = argv[3];

  int result = sendfile(serverip, port, filename);
  if (result != 0)
  {
    return result;
  }

  return 0;
}

int sendfile(char *serverip, int port, char *filename)
{
  size_t bufsize = 10;
  unsigned char *buffer = malloc(bufsize);
  if (buffer == NULL)
  {
    printf("Out of memory.");
    return 1;
  }

  int fd = open(filename, O_RDONLY);
  if (fd < 0)
  {
    printf("Couldn't open file: '%s'\n", filename);
    return 1;
  }

  struct stat st;
  stat(filename, &st);
  size_t filelength = st.st_size;

  int sd = socket(AF_INET, SOCK_STREAM, 0);
  if (sd == -1)
  {
    printf("Couldn't create socket.");
    return 1;
  }

  struct sockaddr_in server;
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(serverip);
  server.sin_port = htons(port);

  if (connect(sd, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    printf("Connect error.\n");
    return 1;
  }

  printf("Connected.\n");

  int size = bufsize;

  for (long offset = 0; offset < filelength; offset += size)
  {
    if (offset + bufsize > filelength)
    {
      size = filelength - offset;
    }

    printf("Reading: %d\n", size);
    read(fd, buffer, size);

    printf("Sending: %d\n", size);
    if (send(sd, buffer, size, 0) < 0)
    {
      printf("Send failed.\n");
      return 1;
    }
    printf("Data sent.\n");
  }

  printf("Deallocating buffer.\n");
  free(buffer);
  printf("Closing file.\n");
  close(fd);
  printf("Closing socket.\n");
  close(sd);

  return 0;
}
