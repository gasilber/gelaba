#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>

ssize_t
glbWriteAll (int fd, const void *buf, size_t nbyte)
{
  ssize_t nwritten = 0, n;

  if (buf == NULL || nbyte <= 0)
    return 0;

  do {
    n = write (fd, &((const char *)buf)[nwritten], nbyte - nwritten);
    if (n == -1)
      {
	if (errno == EINTR)
	  continue;
	else
	  return -1;
      }
    nwritten += n;
  } while (nwritten < nbyte);

  return nwritten;
}

ssize_t
glbReadAll (int fd, void *buf, size_t nbyte)
{
  ssize_t nread = 0, n;

  if (buf == NULL || nbyte <= 0)
    return 0;
  
  do {
    n = read (fd, &((char *)buf)[nread], nbyte - nread);
    if (n == -1)
      {
	if (errno == EINTR)
	  continue;
	else
	  return -1;
      }
    if (n == 0)
      return nread;
    nread += n;
  } while (nread < nbyte);

  return nread;
}

int
glbCreateTCPService (int port, int max_clients)
{
  static const char *fname = "glbCreateTCPService";
  int socket_id;
  int optval = 1;
  int ret;
  struct sockaddr_in sockname;

  socket_id = socket (AF_INET, SOCK_STREAM, 0);
  if (socket_id == -1)
    {
      glbSysError (fname, "cannot create TCP service socket.\n");
      return -1;
    }

  /*setsockopt (socket_id, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (int));*/
  memset ((char*) &sockname, 0, sizeof (struct sockaddr_in));

  sockname.sin_family = AF_INET;
  sockname.sin_port = htons (port);
  sockname.sin_addr.s_addr = htonl (INADDR_ANY);

  ret = bind (socket_id, (struct sockaddr *) &sockname, sizeof (struct sockaddr_in));
  if (ret == -1)
    {
      glbSysError (fname, "cannot bind TCP service socket.\n");
      return -1;      
    }

  ret = listen (socket_id, max_clients);
  if (ret == -1)
    {
      glbSysError (fname, "cannot listen in TCP service socket with %d max clients.\n", max_clients);
      return -1;      
    }

  return socket_id;
}
