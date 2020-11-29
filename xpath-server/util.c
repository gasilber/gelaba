/*
*/
#include "grab.h"

/**
 * glbNormalizeSpace:
 * @source: text to normalize
 */
xmlChar *
glbNormalizeSpace(const xmlChar* source)
{
  xmlBufferPtr target;
  xmlChar blank;
  xmlChar *result = NULL;
  
  target = xmlBufferCreate();
  if (target && source) 
    {
      while (IS_BLANK_CH(*source))
	source++;
    
      blank = 0;
      while (*source) 
	{
	  if (IS_BLANK_CH(*source)) 
	    {
	      blank = 0x20;
	    } 
	  else 
	    {
	      if (blank) 
		{
		  xmlBufferAdd(target, &blank, 1);
		  blank = 0;
		}
	      xmlBufferAdd(target, source, 1);
	    }
	  source++;
	}
      result = xmlStrdup (xmlBufferContent(target));
      xmlBufferFree(target);
    }
  return result;	
}

xmlNodePtr
glbNextElement (xmlNodePtr element)
{
  xmlNodePtr cur;

  if (element == NULL) 
    return NULL;
  assert (element->type == XML_ELEMENT_NODE);
  cur = element->next;
  while (cur && cur->type != XML_ELEMENT_NODE)
    cur = cur->next;
  return cur;
}

xmlNodePtr
glbFirstChild (xmlNodePtr element)
{
  xmlNodePtr cur;
  if (element == NULL) return NULL;
  assert (element->type == XML_ELEMENT_NODE);
  cur = element->children;
  while (cur && cur->type != XML_ELEMENT_NODE)
    cur = cur->next;
  return cur;
}

int
glbCountChildren (xmlNodePtr def)
{
  xmlNodePtr cur;
  int count;
  
  if (def == NULL)
    return 0;
  
  assert(def->type == XML_ELEMENT_NODE);
  
  cur = glbFirstChild (def);
  count = 0;
  while (cur != NULL) 
    {
      count++;
      cur = glbNextElement (cur);
    }	
  
  return count;
}

xmlNodePtr
glbGetChild (xmlNodePtr def, int i)
{
  xmlNodePtr cur;
  xmlNodePtr node;
  int count;
  
  if (def == NULL) return NULL;
  if (i < 1) return NULL;
  
  assert(def->type == XML_ELEMENT_NODE);
  
  cur = glbFirstChild(def);
  count = 0;
  node = NULL;
  while (cur != NULL) 
    {
      count++;
      if (count == i) 
	{
	  node = cur;
	  break;
	}
      cur = glbNextElement(cur);
    }	
  
  return node;
}

/* 0 = continue, 1 = stop */
int
glbWalkNode (xmlNodePtr n, 
	     glbWalkerNode prewalker, 
	     glbWalkerNode inwalker,
	     glbWalkerNode postwalker, 
	     const void *user, 
	     int depth)
{
  xmlNodePtr cur;
  int ret;

  if (n == NULL)
    return 0;

  if (prewalker)
    if (prewalker (n, user, depth))
      return 0;

  cur = n->children;
  while (cur)
    {
      if (glbWalkNode (cur, prewalker, inwalker, postwalker, user, depth + 1))
	return 1;
      cur = cur->next;
      if (cur && inwalker)
	if (inwalker (n, user, depth))
	  break;
    }
  
  if (postwalker)
    if (postwalker (n, user, depth))
      return 1;

  return 0;
}

int
glbRemoveBlankNodes (xmlNodePtr n)
{
  xmlNodePtr cur;
  xmlNodePtr next;

  if (n == NULL)
    return 0;

  cur = n->children;
  while (cur)
    {
      next = cur->next;      
      if (xmlIsBlankNode (cur))
	{
	  xmlUnlinkNode (cur);
	  xmlFreeNode (cur);
	}
      else
	glbRemoveBlankNodes (cur);
      cur = next;
    }

  return 0;
}

int
glbRemoveEmptyNodes (xmlNodePtr n)
{
  xmlNodePtr cur;
  xmlNodePtr next;

  if (n == NULL)
    return 0;

  cur = n->children;
  if (n->type == XML_ELEMENT_NODE && cur == NULL)
    {
      xmlUnlinkNode (n);
      xmlFreeNode (n);
    }
  else
    while (cur)
      {
	next = cur->next;      
	glbRemoveEmptyNodes (cur);
	cur = next;
      }

  return 0;
}
