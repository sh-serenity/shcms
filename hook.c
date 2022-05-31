#include "fcgi_config.h"
#include "fcgiapp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cJSON.c"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#define SOCKET_PATH "0.0.0.0:9997"
static int socketId;

FCGX_Request r;


char *readfile(char *filename) {
  FILE *f;
  f = fopen(filename, "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *string = (char *)calloc(fsize + 1, sizeof(char));
  fread(string, fsize, 1, f);
  fclose(f);

  string[fsize] = 0;
  return string;
}

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
      len = FCGX_GetParam("CONTENT_LENGTH", r.envp);
      int ilen = atoi(len);
      len = FCGX_GetParam("CONTENT_LENGTH", r.envp);
   
      if ((ilen > 0))
      {
        //FILE *fe = fopen("hook.txt", "a");
        char *rawbufp = malloc(ilen);
        FCGX_GetStr(rawbufp, ilen, r.in);
        char *bufp = url_decode(rawbufp);
        bufp[ilen] = 0;
        cJSON *bundle = cJSON_Parse(bufp);
        if(bundle) {
        cJSON *repo = cJSON_GetObjectItemCaseSensitive(bundle, "repository");
        cJSON *full_name = cJSON_GetObjectItemCaseSensitive(repo,"full_name");
        if(!strcmp(full_name->valuestring,"sh-serenity/shcms"))
        {
            FILE *fe = fopen("/build/buildnumber", "r");
            int buildnumber;
            fscanf(fe, "%d", &buildnumber);
            fclose(fe);
            buildnumber++;
            fe = fopen("/buildnumebr", "w");
            char *s = (char *)malloc(32);
            sprintf(s,"%d",buildnumber);
            fputs(s,fe);
            fclose(fe);
            char *path = malloc(64);
            sprintf(path,"/build/%s",s);
            mkdir(path,755);
            chdir(path);
            char *start = readfile("/build/deploy.sh");
            FILE *fp = fopen("deploy.sh", "wb");
            fwrite(start, sizeof(char), strlen(start), fp);
            fclose(fp);           
            system("bash deploy.sh");
            free(s);
            free(path);
            free(start);
        }
        }
        free(bufp);
        free(rawbufp);
        cJSON_Delete(bundle);
      }
    }
    FCGX_Finish_r(&r);
  }
  free(m);
  free(len);
  free(page);
}