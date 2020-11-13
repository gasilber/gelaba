#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "grab.h"

static FILE* logf = NULL;
static FILE* errf = NULL;

int glbDebug = 0;

void
glbSetLog (FILE *f)
{
  logf = f;
}

void
glbSetError (FILE *f)
{
  errf = f;
}

void
glbError (const char *fname, const char *fmt, ...)
{
  va_list v;
  if (errf == NULL)
    errf = stderr;
  fprintf (errf, "[GeLaBa] %d ERROR (%s): ", getpid(), fname);
  va_start (v, fmt);
  vfprintf (errf, fmt, v);
  va_end (v);
}

void
glbSysError (const char *fname, const char *fmt, ...)
{
  va_list v;
  if (errf == NULL)
    errf = stderr;
  fprintf (errf, "[GeLaBa] %d SYSERROR (%s): %s: ", getpid(), fname, strerror (errno));
  va_start (v, fmt);
  vfprintf (errf, fmt, v);
  va_end (v);
}

void
glbWarning (const char *fname, const char *fmt, ...)
{
  va_list v;
  if (errf == NULL)
    errf = stderr;
  fprintf (errf, "[GeLaBa] %d WARNING (%s): ", getpid(), fname);
  va_start (v, fmt);
  vfprintf (errf, fmt, v);
  va_end (v);
}

void
glbMessage (const char *fname, const char *fmt, ...)
{
  va_list v;
  if (errf == NULL)
    errf = stderr;
  fprintf (errf, "[GeLaBa] %d MESSAGE (%s): ", getpid(), fname);
  va_start (v, fmt);
  vfprintf (errf, fmt, v);
  va_end (v);
}

void
glbDebugMsg (const char *fname, const char *fmt, ...)
{
  if (glbDebug)
    {
      va_list v;
      if (errf == NULL)
	errf = stderr;
      fprintf (errf, "[GeLaBa] %d DEBUG (%s): ", getpid(), fname);
      va_start (v, fmt);
      vfprintf (errf, fmt, v);
      va_end (v);
    }
}

void
glbLog (const char *fname, const char *fmt, ...)
{
  va_list v;
  if (logf == NULL)
    logf = stderr;
  fprintf (logf, "[GeLaBa] %d OK (%s): ", getpid(), fname);
  va_start (v, fmt);
  vfprintf (logf, fmt, v);
  va_end (v);
}

void
glbLogError (const char *fname, const char *fmt, ...)
{
  va_list v;
  if (logf == NULL)
    logf = stderr;
  fprintf (logf, "[GeLaBa] %d ERROR (%s): ", getpid(), fname);
  va_start (v, fmt);
  vfprintf (logf, fmt, v);
  va_end (v);
}
