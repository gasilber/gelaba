#include "grab.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define MAXREQSIZE 4096
#define MAXCLIENTS 10

#define FAVICON "GET /favicon.ico"

/*
  TODO LIST:

  - Add a version number.

  - Status function ? With version, date, memory usage, etc...

  - filename=FILE support, with catalog reading at launchtime. Maybe
    it can just be a list of files, stored in a hashtable (just the
    basename). The doc is going to be stored in the hashtable...
    maybe it could be more something like: url/file.xml?xpath=

  - case of GML dictionaries.

  - Add a threshold where the service refuses to return data.

  - OpenSSL version ?

  - key=KEY ? Combined with OpenSSL ?

  - 
*/

static int
write_data (int fd, char *buf, int len)
{
  static const char *fname = "write_data";
  if (glbWriteAll (fd, buf, len) == -1)
    {
      glbSysError (fname, "fatal write error.\n");
      return 1;
    }
  return 0;
}

#define GWRITEBUF(f,b,l) if (write_data (f, b, l) == -1) return -1;
#define GWRITE(f,s) if (write_data (f, s, strlen(s)) == -1) return -1;

void
usage (int ac, char **av)
{
  printf ("xpathservice port XML_file\n");
}

char*
time_string ()
{
  char *tstr;
  time_t t;
  
  t = time (NULL);
  if (t == -1)
    return NULL;
  return ctime (&t);
}

int
write_header_response (int fd)
{
  char *ts = time_string ();
  GWRITE (fd, "HTTP/1.1 200 OK\n");
  GWRITE (fd, "Date: ");
  GWRITE (fd, (ts?ts:"\n"));
  GWRITE (fd, "Server: XPathService\n");
  GWRITE (fd, "Content-Type: text/xml\n");
  GWRITE (fd, "\n");
  return 0;
}

int
write_header_bad_request (int fd)
{
  char *ts = time_string ();
  GWRITE (fd, "HTTP/1.1 400 Bad Request\n");
  GWRITE (fd, "Date: ");
  GWRITE (fd, (ts?ts:"\n"));
  GWRITE (fd, "Server: XPathService\n");
  GWRITE (fd, "\n");
  GWRITE (fd, "400 Bad Request\n");
  GWRITE (fd, "\n");
  return 0;
}

int
write_header_internal_server_error (int fd)
{
  char *ts = time_string ();
  GWRITE (fd, "HTTP/1.1 500 Internal Server Error\n");
  GWRITE (fd, "Date: ");
  GWRITE (fd, (ts?ts:"\n"));
  GWRITE (fd, "Server: XPathService\n");
  GWRITE (fd, "\n");
  GWRITE (fd, "500 Internal Server Error\n");
  GWRITE (fd, "\n");
  return 0;
}

char*
get_request (int fd, int *error_code)
{
  static const char *fname = "get_request";
  char buf[MAXREQSIZE+1];
  char smallbuf[BUFSIZ];
  int ret;
  int readbytes;
  int done;
  int i;
  char *start;
  char *request;

  readbytes = 0;
  done = 0;

  *error_code = 0;

  while (!done)
    {      
      /*ret = glbReadAll (fd, smallbuf, BUFSIZ);      */
      ret = read (fd, smallbuf, BUFSIZ);
      if (ret == -1 && errno != EINTR)
	{
	  glbSysError (fname, "read problem from client.\n");
	  *error_code = 4;
	  return NULL;
	}

      if (ret > 0)
	{
	  /*write (1, smallbuf, ret);*/
	  for (i = 0; i < ret && !done; i++, readbytes++)
	    {
	      if (readbytes >= MAXREQSIZE)
		{
		  glbError (fname, "request too long.\n");
		  *error_code = 1;
		  return NULL;
		}
	      if (smallbuf[i] == '\r' || smallbuf[i] == '\n')
		{
		  done = 1;
		  buf[readbytes] = '\0';
		}
	      else
		{		  
		  buf[readbytes] = smallbuf[i];
		}
	    }
	}
    }

  char * t = time_string ();
  glbLog (fname, "REQUEST RECEIVED : %s\n", buf);
  glbLog (fname, "REQUEST DATE     : %s", (t?t:"\n"));

  if (strncmp (buf, FAVICON, strlen (FAVICON)) == 0)
    {
      glbError (fname, "favicon request ignored.\n");
      *error_code = 6;
      return NULL;
    }

  /* Clean up mess, i.e. remove front 'GET .*\?' and remove tail after
     the first space.
   */
  done = 0;
  for (i = 0; i < readbytes && !done; i++)
    {
      if (buf[i] == '?')
	done = 1;
    }

  if (!done)
    {
      glbError (fname, "malformed request.\n");
      *error_code = 2;
      return NULL;
    }

  start = &buf[i];
  for (i = i; i < readbytes; i++)
    {
      if (buf[i] == ' ')
	{
	  buf[i] = '\0';
	  break;
	}
    }

  if (strlen (start) == 0)
    {
      glbError (fname, "empty request.\n");
      *error_code = 3;
      return NULL;
    }

  request = strdup (start);
  if (request == NULL)
    {
      glbSysError (fname, "cannot allocate memory for request.\n");
      *error_code = 4;
      return NULL;
    }

  return request;
}

int
xpath_service (xmlDocPtr doc, int fd)
{
  static const char *fname = "xpath_service";
  glbGrabQueryPtr q;
  glbGrabContextPtr c;

  char *request;
  int errcode;

  request = get_request (fd, &errcode);
  if (request == NULL)
    {
      glbError (fname, "cannot get request (errcode=%d).\n", errcode);
      /* TODO: write to client.  */
      if (errcode == 3)
	{
	  write_header_internal_server_error (fd);
	  glbGrabError (fd, "Internal error", "3");
	}
      else 
	{
	  write_header_bad_request (fd);
	  glbGrabError (fd, "Bad request", "2");
	}
      return -1;
    }

  q = glbGrabNewQuery (request);
  if (q == NULL)
    {
      glbError (fname, "cannot build request.\n");
      write_header_response (fd);      
      glbGrabError (fd, "Bad query", "1");
      /* TODO: write to client.  */
      return -1;
    }
  
  glbGrabLogQuery (q);

  /* TODO: cas particulier quand XPath=/.  Renvoyer le 
     document intégral, sans <glb:..>.  */

  c = glbGrabExecuteQuery (doc, q);

  if (c == NULL)
    {
      glbError (fname, "cannot execute query.\n");
      write_header_response (fd);      
      glbGrabError (fd, "Bad query", "1");
      /* TODO: write to client.  */
      return -1;
    }
  
  write_header_response (fd);

  if (glbGrabWrite (c, fd, q->min, q->max) == -1)
    {
      glbError (fname, "cannot write request result.\n");
      glbGrabError (fd, "Cannot write.", "2");
      /* TODO: write to client.  */
      return -1;
    }

  free (request);

  return 0;
}


int
main (int ac, char **av)
{
  static const char *fname = "main";
  int sock;
  int client;
  int pid;
  char *xmlfile;
  xmlDocPtr doc;
  long port;
  struct sockaddr_in addr;
  socklen_t addr_len;

  if (ac != 3)
    {
      usage(ac, av);
      exit (1);
    }

  port = strtol (av[1], NULL, 10);
  if (port > INT_MAX || port <=0)
    {
      glbError (fname, "Bad port value %d.\n", port);
      return 1;
    }

  xmlfile = av[2];
  doc = xmlReadFile (xmlfile, NULL, 0);
  if (doc == NULL)
    {
      glbError (fname, "unable to read XML file '%s'.\n", xmlfile);
      return 1;
    }
  
  sock = glbCreateTCPService ((int)port, MAXCLIENTS);
  if (sock == -1)
    {
      return 1;
    }

  glbLog (fname, "Serving '%s' on port '%d'.\n", xmlfile, port);
  
  while (1)
    {
      /* TODO: get client address.  */
      addr_len = sizeof (struct sockaddr_in);
      client = accept (sock, (struct sockaddr*)&addr, &addr_len);
      if (client == -1)
	{
	  glbSysError (fname, "accept problem.\n");
	  return 1;
	}

      if (addr_len == sizeof (struct sockaddr_in))
	{
	  char buf [BUFSIZ];
	  char *a;
	  a = inet_ntoa (addr.sin_addr);
	  glbLog (fname, "Open connection from %s (%x)\n", a, a);
	}
      else
	{
	  glbLog (fname, "Open connection.\n");
	}
      fflush (stdout);

      pid = fork();
      if (pid == -1)
	{
	  glbSysError (fname, "cannot fork().\n");
	  return 1;
	}      
      else if (pid == 0)
	{
	  if (close (sock) == -1)
	    {
	      glbSysError (fname, "cannot close service socket.\n");
	      glbLog (fname, "Close connection.\n");
	      exit (EXIT_FAILURE);
	    }

	  if (xpath_service (doc, client) == -1)
	    {
	      glbError (fname, "cannot serve client.\n");
	      glbLog (fname, "Close connection.\n");	 
	      exit (EXIT_FAILURE);
	    }

	  if (shutdown (client, SHUT_RDWR) == -1)
	    {
	      glbSysError (fname, "cannot shutdown client socket.\n");
	      glbLog (fname, "Close connection.\n");	  
	      exit (EXIT_FAILURE);	      
	    }

	  if (close (client) == -1)
	    {
	      glbSysError (fname, "cannot close client socket.\n");
	      glbLog (fname, "Close connection.\n");
	      exit (EXIT_FAILURE);	      
	    }

	  glbLog (fname, "Close connection.\n");
	  exit (EXIT_SUCCESS);
	}
      else
	{
	  close (client);
	  while (waitpid (-1, NULL, WNOHANG) > 0);
	}     
    }
  
  return 0;
}
