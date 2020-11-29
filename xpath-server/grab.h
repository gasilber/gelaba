#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <sys/socket.h>
#include <errno.h>
#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/uri.h>

#define GLB_BUFSIZ 1024

/*
 * Utils
 */
xmlNodePtr glbNextElement   (xmlNodePtr element);
xmlNodePtr glbFirstChild    (xmlNodePtr element);
int        glbCountChildren (xmlNodePtr element);
xmlNodePtr glbGetChild      (xmlNodePtr root, int i);


typedef int (*glbWalkerNode) (const xmlNodePtr data, 
			      const void *user, 
			      int depth);

int glbWalkNode (xmlNodePtr n, 
		  glbWalkerNode prewalker, 
		  glbWalkerNode inwalker,
		  glbWalkerNode postwalker, 
		  const void *user, 
		  int depth);

xmlChar*   glbNormalizeSpace   (const xmlChar* source);

/*
  ERRORS
*/
extern int glbDebug;
void glbSetLog (FILE *f);
void glbSetError (FILE *f);
void glbError    (const char *fname, const char *fmt, ...);
void glbSysError (const char *fname, const char *fmt, ...);
void glbWarning  (const char *fname, const char *fmt, ...);
void glbMessage  (const char *fname, const char *fmt, ...);
void glbDebugMsg (const char *fname, const char *fmt, ...);
void glbLog      (const char *fname, const char *fmt, ...);

/*
  GRAB
*/
#define GLB_GRAB_PREAMBLE "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
#define GLB_GRAB_START_DOC "<glb:grab xmlns:glb=\"http://www.gelaba.org/gelaba/1.0\""
#define GLB_GRAB_STOP_DOC "</glb:grab>"

#define GLB_GRAB_START_NODESET "<glb:nodeset"
#define GLB_GRAB_STOP_NODESET "</glb:nodeset>"
#define GLB_GRAB_START_NODE "<glb:n"
#define GLB_GRAB_STOP_NODE  "</glb:n>"

#define GLB_GRAB_START_NUMBER "<glb:number>"
#define GLB_GRAB_STOP_NUMBER  "</glb:number>"

#define GLB_GRAB_START_BOOL "<glb:boolean>"
#define GLB_GRAB_STOP_BOOL  "</glb:boolean>"

#define GLB_GRAB_START_STRING "<glb:string>"
#define GLB_GRAB_STOP_STRING  "</glb:string>"

#define GLB_GRAB_START_ERROR "<glb:error>"
#define GLB_GRAB_STOP_ERROR  "</glb:error>"
#define GLB_GRAB_START_ERROR_CODE "<glb:error_code>"
#define GLB_GRAB_STOP_ERROR_CODE  "</glb:error_code>"
#define GLB_GRAB_START_ERROR_MESSAGE "<glb:error_msg>"
#define GLB_GRAB_STOP_ERROR_MESSAGE  "</glb:error_msg>"

#define GLB_GRAB_INTERNAL_ERROR \
  "<glb:internal_error xmlns:glb=\"http://www.gelaba.org/gelaba/1.0\"/>"

#define GLB_GRAB_PARAM_XPATH "xpath="
#define GLB_GRAB_PARAM_FILENAME "filename="
#define GLB_GRAB_PARAM_MIN "min="
#define GLB_GRAB_PARAM_MAX "max="
#define GLB_GRAB_PARAM_NAMESPACES "namespace="

typedef struct glbGrabQuery {
  long min;
  long max;
  char *qstring;
  char *xpath;
  xmlListPtr namespaces;
  char *filename;
} glbGrabQuery, *glbGrabQueryPtr;

glbGrabQueryPtr glbGrabNewQuery (char *q_string);
void glbGrabFreeQuery (glbGrabQueryPtr q);
void glbGrabPrintQuery (glbGrabQueryPtr q, FILE *f);
void glbGrabLogQuery (glbGrabQueryPtr q);

typedef struct glbGrabNs {
  xmlChar* prefix;
  xmlChar* uri;
} glbGrabNs, *glbGrabNsPtr;

typedef struct glbGrabContext {
  char buffer[BUFSIZ+1];
  xmlBufferPtr output;
  xmlDocPtr doc;
  xmlXPathContextPtr ctxt;
  xmlChar* xpath_req;
  xmlXPathObjectPtr response;
} glbGrabContext, *glbGrabContextPtr;

glbGrabContextPtr glbGrabNewContext (xmlDocPtr doc);
void glbGrabFreeContext (glbGrabContextPtr c);
int glbGrabRegisterNS (glbGrabContextPtr c, const xmlChar *prefix, const xmlChar *href);
int glbGrabEval (glbGrabContextPtr c, const xmlChar* req);
int glbGrabWrite (glbGrabContextPtr c, int fd, int first, int last);

glbGrabContextPtr glbGrabExecuteQuery (xmlDocPtr doc, glbGrabQueryPtr q);


/*
 * SERVICE
 */
int glbCreateTCPService (int port, int max_clients);
ssize_t glbReadAll (int fd, const void *buf, size_t nbyte);
ssize_t glbWriteAll (int fd, const void *buf, size_t nbyte);

