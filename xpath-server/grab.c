#include "grab.h"

#define DEFAULT_MAX_RESPONSES 100

static void
xmatch (xmlXPathParserContextPtr ctxt, int nargs) 
{
  xmlXPathObjectPtr hay, needle;
  xmlRegexpPtr cregexp;
  
  CHECK_ARITY(2);
  CAST_TO_STRING;
  CHECK_TYPE(XPATH_STRING);
  needle = valuePop(ctxt);
  CAST_TO_STRING;
  hay = valuePop(ctxt);

  if ((hay == NULL) || (hay->type != XPATH_STRING)) 
    {
      xmlXPathFreeObject(hay);
      xmlXPathFreeObject(needle);
      XP_ERROR(XPATH_INVALID_TYPE);
    }

  cregexp = xmlRegexpCompile (needle->stringval);
  if (cregexp == NULL) 
    {
      xmlXPathFreeObject(hay);
      xmlXPathFreeObject(needle);
      XP_ERROR(XPATH_INVALID_TYPE);    
    }

  if (xmlRegexpExec (cregexp, hay->stringval))
    valuePush(ctxt, xmlXPathNewBoolean(1));
  else
    valuePush(ctxt, xmlXPathNewBoolean(0));
  
  xmlXPathFreeObject (hay);
  xmlXPathFreeObject (needle);
  xmlRegFreeRegexp (cregexp);
}

glbGrabContextPtr
glbGrabNewContext (xmlDocPtr doc)
{
  static const char *fname = "glbGrabNewContext";
  glbGrabContextPtr c;

  assert (doc);

  c = (glbGrabContextPtr) malloc (sizeof (glbGrabContext));
  if (c == NULL)
    {
      glbSysError (fname, "cannot allocate new grab context.\n");
      return NULL;
    }
  memset (c, 0, sizeof (glbGrabContext));  
  c->doc = doc;
  
  c->ctxt = xmlXPathNewContext (doc);
  if (c->ctxt == NULL)
    {
      glbError (fname, "cannot allocate new XPath context.\n");
      glbGrabFreeContext (c);
      return NULL;
    }

  if (xmlXPathRegisterFunc (c->ctxt, BAD_CAST "matches", xmatch) == -1)
    {
      glbError (fname, "cannot allocate new XPath function 'matches'.\n");
      glbGrabFreeContext (c);
      return NULL;
    }
  
  c->output = xmlBufferCreate ();
  if (c->output == NULL)
    {
      glbError (fname, "cannot create buffer.\n");
      glbGrabFreeContext (c);
      return NULL;
    }
  
  return c;
}

void
glbGrabFreeContext (glbGrabContextPtr c)
{
  if (c)
    {
      if (c->ctxt)
	xmlXPathFreeContext (c->ctxt);
      if (c->xpath_req)
	free (c->xpath_req);
      if (c->response)
	xmlXPathFreeObject (c->response);
      if (c->output)
	xmlBufferFree (c->output);
      /*if (c->query)
	glbGrabFreeQuery (c->query);
	This is just a convenience. Does not free that.
      */
      free (c);
    }
}

int
glbGrabRegisterNS (glbGrabContextPtr c, const xmlChar *prefix, const xmlChar *href)
{
  static const char *fname = "glbGrabRegisterNS";
  
  assert (c);
  assert (href);

  if (xmlXPathRegisterNs (c->ctxt, prefix, href) == -1)
    {
      glbError (fname, "unable to register NS %s=%s\n", prefix, href);
      return -1;
    }
  return 0;
}

int
glbGrabEval (glbGrabContextPtr c, const xmlChar* req)
{
  static const char *fname = "glbGrabEval";

  assert (c);
  assert (req);

  if (c->xpath_req)
    xmlFree (c->xpath_req);

  c->xpath_req = xmlStrdup (req);
  if (c->xpath_req == NULL)
    {
      glbSysError (fname, "cannot allocate Xpath request.\n");
      return -1;
    }

  if (c->response)
    {
      xmlXPathFreeObject (c->response);
      c->response = NULL;
    }

  c->response = xmlXPathEvalExpression (c->xpath_req, c->ctxt);
  if (c->response == NULL)
    {
      glbError (fname, "unable to evaluate expression: %s\n", c->xpath_req);
      return -1;
    }

  return 0;
}

static int
write_data (int fd, char *buf, int len)
{
  static const char *fname = "write_data";
  if (write (fd, buf, len) == -1)
    {
      glbSysError (fname, "fatal write error.\n");
      return 1;
    }
  return 0;
}

#define GWRITEBUF(f,b,l) if (write_data (f, b, l) == -1) return 1;
#define GWRITE(f,s) if (write_data (f, s, strlen(s)) == -1) return 1;

int
glbGrabWriteNumber (glbGrabContextPtr c, int fd)
{
  static const char *fname = "glbGrabWriteNumber";
  char *buf;
  xmlBufferPtr vbuf;

  buf = c->buffer;
  vbuf = c->output;

  GWRITE (fd, GLB_GRAB_PREAMBLE);

  GWRITE (fd, GLB_GRAB_START_DOC);
  GWRITE (fd, ">");

  GWRITE (fd, GLB_GRAB_START_NUMBER);
  if (snprintf (buf, GLB_BUFSIZ, 
		"%f", c->response->floatval) >= GLB_BUFSIZ)
    {
      glbSysError (fname, "buffer to small to write value.\n");
      return 1;
    }
  GWRITE (fd, buf);
  glbLog (fname, "Result XPATH_NUMBER (%s).\n", buf);
  GWRITE (fd, GLB_GRAB_STOP_NUMBER);

  GWRITE (fd, GLB_GRAB_STOP_DOC);
  GWRITE (fd, "\n");
  
  return 0;
}

int
glbGrabWriteString (glbGrabContextPtr c, int fd)
{
  static const char *fname = "glbGrabWriteString";
  char *buf;
  xmlBufferPtr vbuf;

  buf = c->buffer;
  vbuf = c->output;

  assert (c->response->stringval);

  GWRITE (fd, GLB_GRAB_PREAMBLE);

  GWRITE (fd, GLB_GRAB_START_DOC);
  GWRITE (fd, ">");

  GWRITE (fd, GLB_GRAB_START_STRING);
  GWRITE (fd, (char*)c->response->stringval);
  glbLog (fname, "Result XPATH_STRING (%s).\n", (char*)c->response->stringval);
  GWRITE (fd, GLB_GRAB_STOP_STRING);

  GWRITE (fd, GLB_GRAB_STOP_DOC);
  GWRITE (fd, "\n");

  return 0;
}

int
glbGrabWriteBoolean (glbGrabContextPtr c, int fd)
{
  static const char *fname = "glbGrabWriteBoolean";
  char *buf;
  xmlBufferPtr vbuf;

  buf = c->buffer;
  vbuf = c->output;

  GWRITE (fd, GLB_GRAB_PREAMBLE);

  GWRITE (fd, GLB_GRAB_START_DOC);
  GWRITE (fd, ">");

  GWRITE (fd, GLB_GRAB_START_BOOL);
  if (snprintf (buf, GLB_BUFSIZ, 
		"%d", c->response->boolval) >= GLB_BUFSIZ)
    {
      glbSysError (fname, "buffer to small to write value.\n");
      return 1;
    }
  glbLog (fname, "Result XPATH_BOOLEAN (%s).\n", buf);
  GWRITE (fd, buf);
  GWRITE (fd, GLB_GRAB_STOP_BOOL);

  GWRITE (fd, GLB_GRAB_STOP_DOC);
  GWRITE (fd, "\n");

  return 0;
}

int
glbGrabWriteNodeset (glbGrabContextPtr c, int fd, int first, int last)
{
  static const char *fname = "glbGrabWriteNodeset";
  char *buf;
  xmlBufferPtr vbuf;
  int min;
  int max;
  int n;
  int i;
  int encapsulate_p;

  buf = c->buffer;
  vbuf = c->output;

  assert (c->response->nodesetval);

  n = c->response->nodesetval->nodeNr;

  glbLog (fname, "Result XPATH_NODESET (%d nodes).\n", n);

  if (first == 0)
    first = 1;
  if (last == 0)
    last = n;
    /*last = DEFAULT_MAX_RESPONSES;*/

  if (n == 0 || first > n || last < 1)
    {
      min = max = 0;
    }
  else
    {
      assert (n >= 1);
      assert (first <= n);
      
      if (first <= 1)
	min = 1;
      else
	min = first;

      if (last > n)
	/*max = (n>DEFAULT_MAX_RESPONSES?DEFAULT_MAX_RESPONSES:n);*/
	max = n;
      else
	max = last;
    }

  encapsulate_p = (strcmp ((char*)c->xpath_req, "/") != 0);

  if (encapsulate_p)
    {
      GWRITE (fd, GLB_GRAB_PREAMBLE);      
      GWRITE (fd, GLB_GRAB_START_DOC);
      GWRITE (fd, ">");
      
      GWRITE (fd, GLB_GRAB_START_NODESET);
      if (snprintf (buf, GLB_BUFSIZ, 
		    " nb=\"%d\"", n) >= GLB_BUFSIZ)
	{
	  glbSysError (fname, "buffer to small to write value.\n");
	  return 1;
	}
      GWRITE (fd, buf);
      if (snprintf (buf, GLB_BUFSIZ, 
		    " first=\"%d\"", min) >= GLB_BUFSIZ)
	{
	  glbSysError (fname, "buffer to small to write value.\n");
	  return 1;
	}
      GWRITE (fd, buf);
      if (snprintf (buf, GLB_BUFSIZ, 
		    " last=\"%d\"", max) >= GLB_BUFSIZ)
	{
	  glbSysError (fname, "buffer to small to write value.\n");
	  return 1;
	}
      GWRITE (fd, buf);      
      GWRITE (fd, ">");      
    }

  if (min > 0)
    {
      for (i = min; i <= max; i++)
	{
	  xmlBufferEmpty (vbuf);
	  if (xmlNodeDump (vbuf, c->doc, 
			   c->response->nodesetval->nodeTab[i-1],
			   0, 0) == -1)
	    {
	      GWRITE (fd, GLB_GRAB_INTERNAL_ERROR);
	      return 1;
	    }

	  if (encapsulate_p)
	    {
	      GWRITE (fd, GLB_GRAB_START_NODE);
	      if (snprintf (buf, GLB_BUFSIZ, 
			    " n=\"%d\"", i) >= GLB_BUFSIZ)
		{
		  glbSysError (fname, "buffer to small to write value.\n");
		  return 1;
		}
	      GWRITE (fd, buf);
	      GWRITE (fd, ">");
	    }

	  GWRITE (fd, (char*)vbuf->content);

	  if (encapsulate_p)
	    {	      
	      GWRITE (fd, GLB_GRAB_STOP_NODE);
	    }
	}
    }

  if (encapsulate_p)
    {	            
      GWRITE (fd, GLB_GRAB_STOP_NODESET);      
      GWRITE (fd, GLB_GRAB_STOP_DOC);
      GWRITE (fd, "\n");
    }

  return 0;
}

int
glbGrabWriteError (glbGrabContextPtr c, int fd, int xpath_p)
{
  static const char *fname = "glbGrabWriteError";
  char *buf;
  xmlBufferPtr vbuf;
  int code = 100;
  char *message = "internal error.";

  buf = c->buffer;
  vbuf = c->output;

  GWRITE (fd, GLB_GRAB_PREAMBLE);

  GWRITE (fd, GLB_GRAB_START_DOC);
  GWRITE (fd, ">");

  GWRITE (fd, GLB_GRAB_START_ERROR);
  GWRITE (fd, GLB_GRAB_START_ERROR_CODE);
  if (xpath_p)
    {
      code = c->ctxt->lastError.code;
    }
  if (snprintf (buf, GLB_BUFSIZ, "%d", code) >= GLB_BUFSIZ)
    {
      glbSysError (fname, "buffer to small to write value.\n");
      return 1;
    }
  GWRITE (fd, buf);
  GWRITE (fd, GLB_GRAB_STOP_ERROR_CODE);
  GWRITE (fd, GLB_GRAB_START_ERROR_MESSAGE);
  if (xpath_p)
    {
      message = c->ctxt->lastError.message;
    }
  if (message) 
    GWRITE (fd, message);
  GWRITE (fd, GLB_GRAB_STOP_ERROR_MESSAGE);
  GWRITE (fd, GLB_GRAB_STOP_ERROR);

  GWRITE (fd, GLB_GRAB_STOP_DOC);
  GWRITE (fd, "\n");

  return 0;
}

int
glbGrabWrite (glbGrabContextPtr c, int fd, int first, int last)
{
  static const char *fname = "glbGrabWrite";
  char *buf;
  xmlBufferPtr vbuf;
  int i;

  assert (c);
  assert (c->ctxt);
  buf = c->buffer;
  vbuf = c->output;
  assert (buf);
  assert (vbuf);

  if (c->xpath_req)
    {
      /* Log request.  */
      glbLog (fname, "XPath request '%s'\n", c->xpath_req);
      
      if (c->response)
	{
	  xmlXPathObjectType t = c->response->type;
	  switch (t)
	    {
	      /* Things we deal with. */
	    case XPATH_NODESET:
	      if (glbGrabWriteNodeset (c, fd, first, last))
		return 1;
	      break;
	    case XPATH_BOOLEAN:
	      if (glbGrabWriteBoolean (c, fd))
		return 1;
	      break;	  
	    case XPATH_NUMBER:
	      if (glbGrabWriteNumber (c, fd))
		return 1;
	      break;	  
	    case XPATH_STRING:
	      if (glbGrabWriteString (c, fd))
		return 1;
	      break;	  
	      /* Things we don't deal with.  */
	    case XPATH_POINT:
	    case XPATH_RANGE:
	    case XPATH_LOCATIONSET:
	    case XPATH_USERS:
	    case XPATH_XSLT_TREE:
	      glbLog (fname, "NOT IMPLEMENTED %d.\n", t);
	      glbGrabWriteError (c, fd, 0);
	      return 1;
	      break;
	    case XPATH_UNDEFINED:
	      glbLog (fname, "XPATH_UNDEFINED\n");
	      glbGrabWriteError (c, fd, 0);
	      return 1;
	      break;
	    default:
	      glbLog (fname, "UNKNOWN %d.\n", t);
	      glbGrabWriteError (c, fd, 0);
	      return 1;
	      break;
	    }
	}
      else
	{
	  glbLog (fname, "Xpath error %d.\n", c->ctxt->lastError.code);
	  if (glbGrabWriteError (c, fd, 1))
	    return 1;
	}
    }

  return 0;
}

static char*
get_parameter_content (char *p, char *name)
{
  static const char *fname = "get_parameter_content";
  int ln = strlen (name);

  if (strncmp (p, name, ln) == 0)
    {
      char *content;
      content = strdup (p + ln);
      if (content == NULL)
	{
	  glbSysError (fname, "not enough memory for content.\n");
	  return NULL;
	}
      return content;
    }
  return NULL;
}

static void
free_char_in_list (xmlLinkPtr lk)
{
  char **c = xmlLinkGetData (lk);
  free (c[0]);
  free (c);
}


glbGrabQueryPtr
glbGrabNewQuery (char *q_string)
{
  /* TODO: strtok_r... Portabilite Windows?  */
  /* TODO: report an error message to the client.  */
  static const char *fname = "glbGrabNewQuery";
  glbGrabQueryPtr q;

  q = (glbGrabQueryPtr) malloc (sizeof (glbGrabQuery));
  if (q == NULL)
    {
      glbSysError (fname, "cannot allocate query.\n");
      return NULL;
    }
  memset (q, 0, sizeof (glbGrabQuery));

  if (q_string)
    {
      char *parameter;
      char *qstring;
      char *tofree;

      q->qstring = xmlURIUnescapeString (q_string, 0, NULL);
      if (q->qstring == NULL)
	{
	  glbError (fname, "cannot allocate unescaped query string.\n");
	  glbGrabFreeQuery (q);
	  return NULL;
	}
      qstring = tofree = strdup (q->qstring);
      if (qstring == NULL)
	{
	  glbSysError (fname, "cannot allocate copy of unescaped query string.\n");
	  glbGrabFreeQuery (q);
	  return NULL;
	}

      parameter = strtok (qstring, "&");
      
      while (parameter)
	{
	  char *content;
	  
	  if (content = get_parameter_content (parameter, GLB_GRAB_PARAM_XPATH))
	    {
	      q->xpath = content;
	    }
	  else if (content = get_parameter_content (parameter, GLB_GRAB_PARAM_FILENAME))
	    {
	      q->filename = content;
	    }
	  else if (content = get_parameter_content (parameter, GLB_GRAB_PARAM_MIN))
	    {	      
	      q->min = strtol (content, NULL, 10);
	      free (content);
	    }
	  else if (content = get_parameter_content (parameter, GLB_GRAB_PARAM_MAX))
	    {
	      q->max = strtol (content, NULL, 10);
	      free (content);
	    }
	  else if (content = get_parameter_content (parameter, GLB_GRAB_PARAM_NAMESPACES))
	    {
	      char **ns;
	      char *split;
	      char *prefix;
	      char *uri;
	      if (q->namespaces == NULL)
		{
		  q->namespaces = xmlListCreate (free_char_in_list, NULL);
		}
	      ns = (char**) malloc (sizeof (char*) * 2);
	      if (ns == NULL)
		{
		  glbSysError (fname, "cannot allocate namespace.\n");
		  glbGrabFreeQuery (q);
		  return NULL;
		}
	      split = strchr (content, '=');
	      if (split == NULL)
		{
		  glbError (fname, "bad namespace format: %s\n", content);
		  free (ns);
		  glbGrabFreeQuery (q);
		  return NULL;
		}
	      prefix = content;
	      *split = '\0';
	      uri = split + 1;
	      if (strlen (prefix) == 0 || strlen (uri) == 0)
		{
		  glbError (fname, "bad namespace format: '%s','%s'\n", prefix, uri);
		  free (ns);
		  glbGrabFreeQuery (q);
		  return NULL;		  
		}
	      ns[0] = prefix;
	      ns[1] = uri;
	      xmlListAppend (q->namespaces, ns);
	    }
	  parameter = strtok (NULL, "&");
	}      

      if (tofree)
	free (tofree);
    }
  
  if (q->xpath == NULL)
    {
      glbError (fname, "XPath request empty.\n");
      glbGrabFreeQuery (q);
      return NULL;		  
    }

  return q;
}

void
glbGrabFreeQuery (glbGrabQueryPtr q)
{
  if (q)
    {
      if (q->qstring)
	free (q->qstring);
      if (q->xpath)
	free (q->xpath);
      if (q->namespaces)
	xmlListDelete (q->namespaces);
      if (q->filename)
	free (q->filename);
    }
}

static int
namespace_printer (const void * data, const void * user)
{
  const char **ns = (const char**)data;
  fprintf ((FILE*)user, "   xmlns:%s=\"%s\"\n", ns[0], ns[1]);
  return 1;
}

void
glbGrabPrintQuery (glbGrabQueryPtr q, FILE *f)
{
  if (q)
    {
      if (q->qstring)
	fprintf (f, "QUERY String: %s\n", q->qstring);
      if (q->xpath)
	fprintf (f, "XPath String: %s\n", q->xpath);
      if (q->namespaces)
	{
	  fprintf (f, "Namespaces  :\n");
	  xmlListWalk (q->namespaces, namespace_printer, f);
	}
      if (q->filename)
	fprintf (f, "Filename    : %s\n", q->filename);
      fprintf (f, "Min         : %ld\n", q->min);
      fprintf (f, "Max         : %ld\n", q->max);
    }
}

static int
namespace_logger (const void * data, const void * user)
{
  static const char *fname = "glbGrabLogQuery";  
  const char **ns = (const char**)data;
  glbLog (fname, "Namespace    :  xmlns:%s=\"%s\"\n", ns[0], ns[1]);
  return 1;
}

void
glbGrabLogQuery (glbGrabQueryPtr q)
{
  static const char *fname = "glbGrabLogQuery";  
  if (q)
    {
      if (q->qstring)
	glbLog (fname, "QUERY String: %s\n", q->qstring);
      if (q->xpath)
	glbLog (fname, "XPath String: %s\n", q->xpath);
      if (q->namespaces)
	{
	  xmlListWalk (q->namespaces, namespace_logger, NULL);
	}
      if (q->filename)
	glbLog (fname, "Filename    : %s\n", q->filename);
      glbLog (fname, "Min         : %ld\n", q->min);
      glbLog (fname, "Max         : %ld\n", q->max);
    }
}

struct ns_reg {
  glbGrabContextPtr c;
  int retval;
};

static int
namespace_register (const void * data, const void * user)
{
  static const char *fname = "namespace_register";  
  struct ns_reg *s = (struct ns_reg *)user;
  const char **ns = (const char**)data;
  
  if (glbGrabRegisterNS (s->c, BAD_CAST ns[0], BAD_CAST ns[1]))
    {
      s->retval = -1;
      return 0;
    }

  glbLog (fname, "Register namespace xmlns:%s='%s'.\n", ns[0], ns[1]);

  return 1;
}

glbGrabContextPtr
glbGrabExecuteQuery (xmlDocPtr doc, glbGrabQueryPtr q)
{
  static const char *fname = "glbGrabExecuteQuery";
  glbGrabContextPtr c;
  struct ns_reg reg_ns;

  assert (doc);
  assert (q);
  
  c = glbGrabNewContext (doc);
  if (c == NULL)
    {
      glbError (fname, "cannot create new context.\n");
      return NULL;
    }
  
  if (q->namespaces)
    {
      reg_ns.c = c;
      reg_ns.retval = 0;
      xmlListWalk (q->namespaces, namespace_register, &reg_ns);
      if (reg_ns.retval == -1)
	{
	  glbError (fname, "error in namespace registration.\n");
	  glbGrabFreeContext (c);
	  return NULL;
	}
    }

  if (glbGrabEval (c, BAD_CAST q->xpath) == -1)
    {
      glbError (fname, "cannot evaluate query.\n");
      glbGrabFreeContext (c);
      return NULL;      
    }


  return c;
}

int
glbGrabError (int fd, char *message, char *code)
{
  GWRITE (fd, GLB_GRAB_PREAMBLE);
  GWRITE (fd, GLB_GRAB_START_DOC);
  GWRITE (fd, ">");
  GWRITE (fd, GLB_GRAB_START_ERROR);
  GWRITE (fd, GLB_GRAB_START_ERROR_CODE);
  GWRITE (fd, code);
  GWRITE (fd, GLB_GRAB_STOP_ERROR_CODE);
  GWRITE (fd, GLB_GRAB_START_ERROR_MESSAGE);
  GWRITE (fd, message);
  GWRITE (fd, GLB_GRAB_STOP_ERROR_MESSAGE);
  GWRITE (fd, GLB_GRAB_STOP_ERROR);
  GWRITE (fd, GLB_GRAB_STOP_DOC);  
  return 0;
}
