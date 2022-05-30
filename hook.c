#include "fcgi_config.h"
#include "fcgiapp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define SOCKET_PATH "0.0.0.0:9997"
static int socketId;

FCGX_Request r;

static inline int av_tolower(int c)
{
  if (c >= 'A' && c <= 'Z')
    c ^= 0x20;
  return c;
}

char *url_decode(const char *url)
{
  int s = 0, d = 0, url_len = 0;
  char c;
  char *dest = NULL;

  //  if (!url)
  //      return NULL;

  url_len = strlen(url) + 1;
  dest = malloc(url_len);

  if (!dest)
    return NULL;

  while (s < url_len)
  {
    c = url[s++];

    if (c == '%' && s + 2 < url_len)
    {
      char c2 = url[s++];
      char c3 = url[s++];
      if (isxdigit(c2) && isxdigit(c3))
      {
        c2 = av_tolower(c2);
        c3 = av_tolower(c3);

        if (c2 <= '9')
          c2 = c2 - '0';
        else
          c2 = c2 - 'a' + 10;

        if (c3 <= '9')
          c3 = c3 - '0';
        else
          c3 = c3 - 'a' + 10;

        dest[d++] = 16 * c2 + c3;
      }
      else
      { /* %zz or something other invalid */
        dest[d++] = c;
        dest[d++] = c2;
        dest[d++] = c3;
      }
    }
    else if (c == '+')
    {
      dest[d++] = ' ';
    }
    else
    {
      dest[d++] = c;
    }
  }

  return dest;
}


int main()
{
  FCGX_Request r;
  FCGX_Init();
  socketId = FCGX_OpenSocket(SOCKET_PATH, 20);
  if (socketId < 0)
  {
    return 1;
  }
  printf("Socket is opened\n");

  if (FCGX_InitRequest(&r, socketId, 0) != 0)
  {
    printf("Can not init request\n");
  }
  printf("Request is inited\n");
  char *page = (char *)calloc(1024, sizeof(char));
  
  char *m = malloc(16);
  char *len = malloc(16);    
  while (FCGX_Accept_r(&r) >= 0)
  {
  //  FCGX_PutS("Content-type: text/html\r\n", r.out);
  //  FCGX_PutS("\r\n", r.out);
    FCGX_PutS("Accept-Post: application/json\r\n\r\n", r.out);
      
      
    page = FCGX_GetParam("REQUEST_URI", r.envp);
    m = FCGX_GetParam("REQUEST_METHOD", r.envp);
    if (!strcmp(m, "POST"))
    {
      FCGX_PutS("Accept-Post: application/json\r\n\r\n", r.out);
      FILE *fe = fopen("post.txt", "a");   
      len = FCGX_GetParam("CONTENT_LENGTH", r.envp);
      int ilen = atoi(len);
      len = FCGX_GetParam("CONTENT_LENGTH", r.envp);
   
      if ((ilen > 0))
      {
        char *rawbufp = malloc(ilen);
        FCGX_GetStr(rawbufp, ilen, r.in);
        char *bufp = url_decode(rawbufp);
        bufp[ilen] = 0;
     
        fputs(page, fe);
        fputs(bufp, fe);
        free(bufp);
        free(rawbufp);
      }
      fclose(fe);
    }
  
    FCGX_Finish_r(&r);
  }
  free(m);
  free(len);
  free(page);
}