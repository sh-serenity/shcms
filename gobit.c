#define __USE_MISC
#include "fcgi_config.h"
#include "fcgiapp.h"
#include "sodium.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <mysql/mysql.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <regex.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <curl/curl.h>
#include <sys/stat.h>  
#include <stdbool.h>

typedef unsigned int uint;

typedef struct user
{
  int uid;
  char *name;
  int admin;
  
} user;

MYSQL *con;
MYSQL *siriinit();
FCGX_Request r;

char *left;
char *query;
char *len;
char *m;
char *nu;
char *mailto;
char *payload;

static inline int av_tolower(int c);
char *url_decode(const char *str);
int echofile(char *filename, FCGX_Request request);
char *foo();
char *parse_post(char *input, char *find, int mosk);
char *ssha(const char *buf1);
int reg(FCGX_Request r);
int signin(FCGX_Request r);
void send_headers(FCGX_Request r);
int getsuid(FCGX_Request r);
void art(FCGX_Request r);
void addmsg(FCGX_Request r);
void bin(FCGX_Request r);
void add(FCGX_Request r);
void image(FCGX_Request r);
void addart(FCGX_Request r);
void more(FCGX_Request r);
void quit(FCGX_Request r);
//void topic(FCGX_Request r);
void addtopic(FCGX_Request r);
void newtopic(FCGX_Request r);
void send_payload(char *mailto, char *hash);
int sendpay(char *mailto);
void valid(FCGX_Request r);
void noc(FCGX_Request r);
void justone(long int id, FCGX_Request r);  
void one(long int id, int cutflag, FCGX_Request r);


#define SOCKET_PATH "0.0.0.0:9998"
static int socketId;

int doit()
{
  con = siriinit();
  left = (char *)malloc(1024);
  query = (char *)malloc(1024);
  char *len = malloc(16);
  char *m = malloc(32);
  char *nu = malloc(1024);
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
     
  while (FCGX_Accept_r(&r) >= 0)
  {
    //  regex_t ex;
    //regex_t exm, ex, exb, exme;

    //int val, valm, valb, valme;

    page = FCGX_GetParam("REQUEST_URI", r.envp);
    if (strcmp(page, "/signin") == 0)
    {
      signin(r);
    }
    if ((strcmp(page, "/art") == 0) || (strcmp(page, "/") == 0) ||
        (strcmp(page, "") == 0))
    {
      art(r);
    }
    if (strstr(page,"addmsg"))
    {
      addmsg(r);
    }
    if (strcmp(page, "/image") == 0)
    {
      image(r);
    }
    if (strcmp(page, "/reg") == 0)
    {
      reg(r);
    }
    if (strcmp(page, "/quit") == 0)
    {
      quit(r);
    }
    if (strcmp(page, "/addart") == 0)
    {
      addart(r);
    }
//    if (strcmp(page, "/topic") == 0)
//    {
//      topic(r);
//    }
    if (strcmp(page, "/add") == 0)
    {
      add(r);
    }
      if (strcmp(page, "/addtopic") == 0)
    {
      addtopic(r);
    }
    if (strstr(page,"fbclid")) {
      art(r);
    }
    if (strcmp(page, "/newtopic") == 0)
    {
      newtopic(r);
    }
    if (strstr(page, "more"))
    {  
      more(r);
    } 
    if (strstr(page, "valid"))
    {  
      valid(r);
    } 
    if (strstr(page, "noc"))
    {  
      noc(r);
    } 

  /*  valme = regcomp(&exme, "[:/menu?1-90e:]", 0);
    int resme = regexec(&exme, page, 0, NULL, 0);
    if (!resme)
    {
      menu(r);
    } */

  //  valb = regcomp(&exb, "[:/bin?1-90e:]", 0)
    
    if (strstr(page,"bin"))
    {
      bin(r);
    }

   
    FCGX_Finish_r(&r);
  }
  free(nu);
  free(m);
  free(left);
  free(query);
  free(len);
  free(page);
  mysql_close(con);
}

int main()
{
  doit();
  return 0;
}

MYSQL *siriinit()
{
  con = mysql_init(NULL);
  //  char *uid = (char *)malloc(16);
  if (con == NULL)
  {
    fprintf(stderr, "%s\n", mysql_error(con));
    exit(1);
  }

  mysql_real_connect(con, "mysql", "root", "azwsdcrf321", "gobit", 0,
                     NULL, CLIENT_INTERACTIVE);
  mysql_query(con, "SET NAMES utf8 COLLATE utf8_unicode_ci");
  return con;
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


bool file_exists (char *filename) {
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
}

/*
char from_hex(char ch) {
  return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}


char to_hex(char code) {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}

char *url_encode(char *str) {
  char *pstr = str, *buf = malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' ||
        *pstr == '~')
      *pbuf++ = *pstr;
    else if (*pstr == ' ')
      *pbuf++ = '+';
    else
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}

char *url_decode(char *str) {
  char *pstr = str, *buf = malloc(strlen(str) + 1), *pbuf = buf;
  while (*pstr) {
    if (*pstr == '%') {
      if (pstr[1] && pstr[2]) {
        *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
        pstr += 2;
      }
    } else if (*pstr == '+') {
      *pbuf++ = ' ';
    } else {
      *pbuf++ = *pstr;
    }
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}
*/
/*
char *url_decode(const char *str)
{
  int d = 0;

  char *dStr = malloc(strlen(str) + 1);
  char eStr[] = "00";

  strcpy(dStr, str);

  while (!d)
  {
    d = 1;
    int i;

    for (i = 0; i < strlen(dStr); ++i)
    {

      if (dStr[i] == '%')
      {
        if (dStr[i + 1] == 0)
          return dStr;

        if (isxdigit(dStr[i + 1]) && isxdigit(dStr[i + 2]))
        {

          d = 0;


          eStr[0] = dStr[i + 1];
          eStr[1] = dStr[i + 2];


          long int x = strtol(eStr, NULL, 16);


          memmove(&dStr[i + 1], &dStr[i + 3], strlen(&dStr[i + 3]) + 1);

          dStr[i] = x;
        }
      }
      else if (dStr[i] == '+')
      {
        dStr[i] = ' ';
      }
    }
  }

  return dStr;
}
*/
int echofile(char *filename, FCGX_Request request)
{
  sprintf(left,"/opt/shcms/%s",filename);
  if (file_exists(left))
  { 
    FILE *f = fopen(left, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET); /* same as rewind(f); */

    char *string = (char *)calloc(fsize + 1, sizeof(char));
    fread(string, fsize, 1, f);
    fclose(f);

    string[fsize] = 0;
    FCGX_PutS(string, request.out);
    free(string);
  }
  return 0;
}

int isNumber(char *s)
{
    for (int i = 0; s[i]!= '\0'; i++)
    {
        if (isdigit(s[i]) == 0)
              return 0;
    }
    return 1;
}

char *foo()
{
  char *myString = (char *)malloc(33);
  uint32_t myInt;

  if (sodium_init() < 0)
  {
    /* panic! the library couldn't be initialized, it is not safe to use */
    printf("error 1");
  }

  /* myString will be an array of 32 random bytes, not null-terminated */
  randombytes_buf(myString, 32);

  /* myInt will be a random number between 0 and 9 */
  myInt = randombytes_uniform(10);
  return myString;
}

char *parse_post(char *input, char *find, int mosk)
{
  char *subj = strstr(input, find);
  char *start = subj + strlen(find);
  if (start == NULL)
    return NULL;

  //   while (end = strstr(end,"&nbsp;"))
 //  end = strchr(end+3,'&');
 //  int nb = 3;
  char *end;
  char *next;
  next = start;
  char *fuck;
  while (1)
  {
    end = strchr(next, '&');
    //  if (end == strstr(input,"&nbsp;")) end = strchr(strstr(end+1,
    //  "&nbsp;")+1,'&'); if (end == strstr(input,"&quot;")) end =
    //  strchr(strstr(end+1, "&quot;")+1,'&');
    if ((end != (fuck = strstr(next, "&nbsp;"))) &&
        (end != (fuck = strstr(next, "&quot;"))))
      break;
    else
    {
      if (fuck)
        next = fuck + 1;
      if (!end)
      {
        // end = next;
        end = strchr(next, 0);
        break;
      }
    }
  }

  int slen = end - start;
  if (mosk == 0)
  {
    char *sdata = (char *)malloc(slen + 1);
    memcpy(sdata, start, slen);
    sdata[slen] = 0;
    return sdata;
  }
  else
  {
    if (slen < mosk)
    {
      char *sdata = (char *)malloc(slen + 1);
      memcpy(sdata, start, slen);
      sdata[slen] = 0;
      return sdata;
    }
    else
    {
      return NULL;
    }
  }
}
char *ssha(const char *buf1)
{
  char *res = (char *)malloc(64); 
  char *laende = (char *)malloc(64);
  unsigned char *d = SHA256(buf1, strlen(buf1), 0);
  int i;
  for (i = 0; i < 65; i++)
  {
      res[i] = 0;
      laende[i] = 0;
  }
  for (i = 0; i < SHA256_DIGEST_LENGTH; i++)
  {
    sprintf(res, "%02x", d[i]);
    strcat(laende, res);
  }
  return laende;
}

int reg(FCGX_Request r)
{
  payload = malloc(65);
  FCGX_PutS("Content-type: text/html\r\n", r.out);
  FCGX_PutS("\r\n", r.out);
  regex_t ex;
  int val;
  if (!strcmp(FCGX_GetParam("REQUEST_METHOD", r.envp), "POST"))
  {
    echofile("header.tpl", r);
    FCGX_PutS("<div class=sh>", r.out);
    int err;
    err = 0;

    
    len = FCGX_GetParam("CONTENT_LENGTH", r.envp);
    int ilen = atoi(len);
    char *rawbufp = (char *)calloc(ilen, sizeof(char));
    FCGX_GetStr(rawbufp, ilen, r.in);

    char *bufp = url_decode(rawbufp);
    bufp[ilen] = 0;

    free(rawbufp);
    char *pogin = parse_post(bufp, "name=", 40);
    char *email = parse_post(bufp, "email=", 40);
    char *pass = parse_post(bufp, "pass=", 40);
    char *pmore = parse_post(bufp, "pmore=", 40);
    char *fname = parse_post(bufp, "fname=", 40);
    char *sname = parse_post(bufp, "sname=", 40);
    char *invite = parse_post(bufp, "invite=", 40);
    
    if (!pogin)
    {
      FCGX_PutS("Something is wrong, go back to <a "
                "href=\"http://192.168.1.50/register.html\">Registration</a> and "
                "try again.",
                r.out);
      return 0;
    }
    if (!email)
    {
      FCGX_PutS("Something is wrong, go back to <a "
                "href=\"http://192.168.1.50/register.html\">Registration</a> and "
                "try again.",
                r.out);
      return 0;
    }
    if (!pass)
    {
      FCGX_PutS("Something is wrong, go back to <a "
                "href=\"http://192.168.1.50/register.html\">Registration</a> and "
                "try again.",
                r.out);
      return 0;
    }    
    if (!pmore)
    {
      FCGX_PutS("Something is wrong, go back to <a "
                "href=\"http://192.168.1.50/register.html\">Registration</a> and "
                "try again.",
                r.out);
      return 0;
    }
    if (!fname)
    {
      FCGX_PutS("Something is wrong, go back to <a "
                "href=\"http://192.168.1.50/register.html\">Registration</a> and "
                "try again.",
                r.out);
      return 0;
    }
    if (!sname)
    {
      FCGX_PutS("Something is wrong, go back to <a "
                "href=\"http://192.168.1.50/register.html\">Registration</a> and "
                "try again.",
                r.out);
      return 0;
    }
    if (!invite)
    {
      FCGX_PutS("Something is wrong, go back to <a "
                "href=\"http://192.168.1.50/register.html\">Registration</a> and "
                "try again.",
                r.out);
      return 0;
    }
    
    
    val = regcomp(&ex, "[A-Za-z0-9;()-_;:.@А-Яа-я]", 0);
  
    // FCGX_PutS(query,r.out);
    int res;
    
    char s[10];
  
    sprintf(query, "select * from users where name='%s'", pogin);
      mysql_query(con, query);
      MYSQL_RES *confres = mysql_store_result(con);
      int totalrows = mysql_num_rows(confres);
      if (totalrows != 0 ) {err=1; FCGX_PutS("This name is taken<br>", r.out); }  
    {
      
      sprintf(query, "select * from users where email='%s'", email);
      mysql_query(con, query);
      MYSQL_RES *confres = mysql_store_result(con);
      totalrows = mysql_num_rows(confres);
      char s[10];
      if (totalrows != 0 ) {err=3;}
        {
        res = regexec(&ex, pogin, 0, NULL, 0);
        if (!res)
        {
          FCGX_PutS("nicname is ok<br>", r.out);
        }
        else
        {
          FCGX_PutS("nicname contibs not usable characters <br>", r.out);
          err = 1;
        }
        res = regexec(&ex, pass, 0, NULL, 0);
        if (!res)
        {
          FCGX_PutS("pass ok <br>", r.out);
        }
        else
        {
          FCGX_PutS("Error in pass<br>", r.out);
          err = 1;
        }
        if (!strcmp(pass, pmore))
        {
          FCGX_PutS("pass and once more pass the same <br>", r.out);
        }
        else
        {
          FCGX_PutS("pass and once more again password not the same <br>", r.out);
          err = 1;
        }
        
        //val = regcomp(&ex, "^([a-z0-9])(([-a-z0-9._])*([a-z0-9]))*@([a-z0-9])(([a-z0-9-])*([a-z0-9]))+(.([a-z0-9])([-a-z0-9_-])?", 0);
        res = regexec(&ex, email, 0, NULL, 0);
        if (!res)
        {
          FCGX_PutS("email is ok <br>", r.out);
        }
        else
        {
          FCGX_PutS("this is not email<br>", r.out);
          err = 1;
        } 
        if (strcmp(invite,"bbs"))
        {
          err = 1;
          FCGX_PutS("invite is wrong<br>", r.out);
        }
        if (strlen(fname) < 1)
        {
          FCGX_PutS("First name is wrong<br>", r.out);
          err = 1;
        }
        if (strlen(sname) < 1)
        {
          FCGX_PutS("Second name is wrong<br>", r.out);
          err = 1;
        }
        if (strlen(pass) < 4)
        {
          FCGX_PutS("Pass is wrong<br>", r.out);
          err = 1; 
        }
        //if (err==2) FCGX_PutS("This name is taken<br>", r.out);
        if (err==3) FCGX_PutS("This email is taken<br>", r.out);
      }
      if (err == 0)
      {
       // char *payload = malloc(65);
        payload = ssha(foo());
//        send_payload(email, payload);
        sprintf(query,
                "insert into users (name, pass, email,fname,sname,payload,checked) values('%s',md5('%s'),'%s','%s','%s','%s',0)",pogin, pass, email, fname, sname, payload);
        mysql_query(con, query);
        valid(r);
        FCGX_PutS(mysql_error(con), r.out);
        FCGX_PutS("User added. <a href=\"http://192.168.1.50/login.html\">Go ahead!</a>:",r.out);
        free(pogin);
        free(email);
        free(pass);
      }
      else
      {
        //		send_headers(r);
        FCGX_PutS("Something is wrong, go back to <a "
                  "href=\"http://192.168.1.50/register.html\">Registration</a> "
                  "and try again.",
                  r.out);
        free(pogin);
        free(email);
        free(pass);
        free(pmore);
        free(bufp);
        regfree(&ex);
      }
    } 
    
    }
    FCGX_PutS("</div>", r.out);
  }



void valid(FCGX_Request r)
{
  nu = FCGX_GetParam("REQUEST_URI", r.envp);
  char *n = strstr(nu, "/valid=");
  if (n)
  {
    char *e = strchr(nu, 0);
    char *mmm = malloc(65);
    e = strchr(nu, 0);
    int plen = e - (nu + 7);
    memcpy(mmm, nu + 7, plen);
    sprintf(query,
            "select id, kilo from users where payload='%s'",
            mmm);
    mysql_query(con, query);
    MYSQL_RES *confres = mysql_store_result(con);
    int totalrows = mysql_num_rows(confres);
    MYSQL_ROW row;
    row = mysql_fetch_row(confres);
    if (totalrows > 0)
    {
    sprintf(query,
            "update users set checked=1 where payload='%s'",
            mmm);
    mysql_query(con, query);
    FCGX_PutS("Content-type: text/html\r\n", r.out);
    FCGX_PutS("\r\n", r.out);
      FCGX_PutS("<script language=\"javascript\" type=\"text/javascript\"> "
                "window.location.href = "
                "\"http://192.168.1.50/login.html\";</script>",
                r.out);
      
    }
    else
        FCGX_PutS("Somthing wrong with you code",r.out);

  }

  
}
int signin(FCGX_Request r)
{
  // char met[5];

  if (!strcmp(FCGX_GetParam("REQUEST_METHOD", r.envp), "POST"))
  {

  //  char *len = malloc(16);
    len = FCGX_GetParam("CONTENT_LENGTH", r.envp);
    int ilen = atoi(len);
    char *rawbufp = (char *)malloc(ilen);

    FCGX_GetStr(rawbufp, ilen, r.in);
    char *bufp = url_decode(rawbufp);
    bufp[ilen] = 0;
    free(rawbufp);
    //	char *query = malloc(4096);

    char *pogin = parse_post(bufp, "name=", 32);
    char *pass = parse_post(bufp, "pass=", 32);
    
    sprintf(query,
            "select id, kilo from users where name='%s' and pass=md5('%s')",
            pogin, pass);
    mysql_query(con, query);

    MYSQL_RES *confres = mysql_store_result(con);
    int totalrows = mysql_num_rows(confres);
    MYSQL_ROW row;
    row = mysql_fetch_row(confres);
    if (totalrows > 0)
    {
      if (strcmp(row[1], "0") == 0)
      {
        FCGX_PutS("Content-type: text/html\r\n", r.out);
        FCGX_PutS("Set-Cookie: kilo=", r.out);
        char *hash = malloc(65);
        hash = ssha(foo());
        sprintf(query, "update users set kilo='%s' where id='%s'", hash,
                row[0]);
        mysql_query(con, query);
        FCGX_PutS(hash, r.out);
        // FCGX_PutS("\r\n\r\n;", r.out);
        free(hash);
      }
      else
      {
        FCGX_PutS("Content-type: text/html\r\n", r.out);
        FCGX_PutS("Set-Cookie: kilo=", r.out);
        FCGX_PutS(row[1], r.out);
      }
      //  FCGX_PutS("Location: https:/seti/art/",r.out);
      FCGX_PutS(" SameSite=Strict", r.out);
      FCGX_PutS("Location: http://192.168.1.50/art", r.out);
      FCGX_PutS("\r\n", r.out);
      FCGX_PutS("\r\n", r.out);
      //    FCGX_PutS("Location: http://192.168.1.50/art",r.out);
      //         FCGX_PutS("<head><meta https-equiv=\"refresh\"
      //         content=\"0;URL=\http://192.168.1.50/art\"/></head>
      //         ",r.out);
      FCGX_PutS("<script language=\"javascript\" type=\"text/javascript\"> "
                "window.location.href = "
                "\"http://192.168.1.50/art\";</script>",
                r.out);
    } // else {
      //  FCGX_PutS("Content-type: text/html\r\n", r.out);
    FCGX_PutS("\r\n", r.out);
    FCGX_PutS("\r\n", r.out);
   // echofile("index.html",r.out)

     FCGX_PutS("Theris not such user.", r.out);
    //  }
//  free(len);
  free(bufp);
  free(pogin);
  free(pass);
  mysql_free_result(confres);  
  }
}

void send_headers(FCGX_Request r)
{

  char *cookie = (char *)malloc(72);
  cookie = FCGX_GetParam("HTTP_COOKIE", r.envp);
  if (cookie)
  {
    char *str = (char *)malloc(150);
    sprintf(
        str,
        "Content-type: text/html\r\n Set-Cookie: %s; SameSite=Strict\r\n\r\n",
        cookie);
    FCGX_PutS(str, r.out);
    free(str);
  }
  free(cookie);
}

int getsuid(FCGX_Request r)
{
  // char *str = (char *)malloc(110);
  char *kilo = (char *)calloc(200, sizeof(char));
  char *cookie = (char *)malloc(69);
  cookie = FCGX_GetParam("HTTP_COOKIE", r.envp);
  if (FCGX_GetParam("HTTP_COOKIE", r.envp) != NULL)
  {
    cookie = FCGX_GetParam("HTTP_COOKIE", r.envp);
//    FCGX_PutS(cookie,r.out);
  }
  if (cookie)
  {
    // char *query = (char *)malloc(200);

    //    send_headers(r);
    //    sprintf(query, "Content-type: text/html\r\n Set-Cookie: %s
    //    SameSite=Strict\r\n\r\n", cookie);
    //  FCGX_PutS(query, r.out);
    //  FCGX_PutS(cookie, r.out);
    char *kilo = (char *)malloc(70);
    char *query = (char *)malloc(300);
    memcpy(kilo, cookie + 5, 64);
    kilo[64] = 0;
    //   FCGX_PutS(kilo, r.out);
    sprintf(query, "select id from users where kilo='%s'", kilo);
    mysql_query(con, query);
    //   FCGX_PutS(mysql_error(con), r.out);
    MYSQL_RES *confres = mysql_store_result(con);
    if (confres != NULL)
    {
      int totalrows = mysql_num_rows(confres);
      if (totalrows > 0)
      {
        MYSQL_ROW row;
        row = mysql_fetch_row(confres);
        //    mysql_free_result(confres);
        //	FCGX_PutS(row[0],r.out);
        int uid = atoi(row[0]);
        return uid;
      }
    }
    //    FCGX_PutS("Database error", r.out);
    //     free(query);
    //     mysql_free_result(confres);
  } // else {
  // FCGX_PutS("Content-type: text/html\r\n", r.out);
  // FCGX_PutS("\r\n", r.out);
  //  FCGX_PutS("No cookies.", r.out);
  //  }
    free(cookie);
    free(kilo);
}

user getuser(FCGX_Request r)
{
  // char *str = (char *)malloc(110);
  user luser;
  char *kilo = (char *)calloc(200, sizeof(char));
  char *cookie = (char *)malloc(69);
  cookie = FCGX_GetParam("HTTP_COOKIE", r.envp);
  if (FCGX_GetParam("HTTP_COOKIE", r.envp) != NULL)
  {
    cookie = FCGX_GetParam("HTTP_COOKIE", r.envp);
  }
  if (cookie)
  {
    // char *query = (char *)malloc(200);

    //    send_headers(r);
    //    sprintf(query, "Content-type: text/html\r\n Set-Cookie: %s
    //    SameSite=Strict\r\n\r\n", cookie);
    //  FCGX_PutS(query, r.out);
    //  FCGX_PutS(cookie, r.out);
    char *kilo = (char *)malloc(70);
    char *query = (char *)malloc(300);
    memcpy(kilo, cookie + 5, 64);
    kilo[64] = 0;
    //   FCGX_PutS(kilo, r.out);
    sprintf(query, "select id, name, admin, kilo from users where kilo='%s'", kilo);
    mysql_query(con, query);
    //   FCGX_PutS(mysql_error(con), r.out);
    MYSQL_RES *confres = mysql_store_result(con);
    if (confres != NULL)
    {
      int totalrows = mysql_num_rows(confres);
      if (totalrows > 0)
      {
        MYSQL_ROW row;
        row = mysql_fetch_row(confres);
        //    mysql_free_result(confres);
        //	FCGX_PutS(row[0],r.out);
     //   if (row[0])
     //   {
     //     if(!strcmp(row[3],"0")) {
     //       luser.uid = 0;
     //       return luser;
     //     }
          luser.uid = atoi(row[0]);
          luser.name = (char *)malloc(strlen(row[1]));
          luser.name = row[1];
          if(row[2])
          luser.admin = 1;
          return luser;
        }
      //  else
      //    luser.uid = 0;
      //  luser.name = "anonimus";
      //  return luser;
      }
    }
    //    FCGX_PutS("Database error", r.out);
    //     free(query);
    //     mysql_free_result(confres);
  }
  
user header(FCGX_Request r)
{
  user luser = getuser(r);


  FCGX_PutS("\r\n", r.out);
  FCGX_PutS("\r\n", r.out);
  echofile("header.tpl", r);
  if (luser.uid)
  {
    sprintf(left, "<div class=hello><p>Welcome!</p></div><div class=menu><a class=sh href=\"/add\">Add</a><a class=sh href=\"/quit\">Quit</a></div>");
    FCGX_PutS(left, r.out);
  }
  else
    echofile("nologinmenu.tpl", r);
 
  return luser;
}

void quit(FCGX_Request r)
{
  user luser = getuser(r);
  sprintf(query, "update users set kilo='0' where id='%d'",luser.uid);
  mysql_query(con,query);
 // char *cookie = (char *)malloc(72);
  //cookie = FCGX_GetParam("HTTP_COOKIE", r.envp);
 // if (cookie)
 // {
  //  char *str = (char *)malloc(150);
  //  sprintf(
  //      str,
  //      "Content-type: text/html\r\n Set-Cookie: kilo=\"\"; expires = Thu, 01 Jan 1970 00:00:00 GMT \r\n\r\n");
  //  FCGX_PutS("Content-type: text/html\r\n Set-Cookie: kilo=\"\" \r\n\r\n", r.out);
 // FCGX_PutS("Content-type: text/html\r\n", r.out);
 // FCGX_PutS("Set-Cookie: kilo=\"0\" \r\n\r\n", r.out);
 // cookie = FCGX_GetParam("HTTP_COOKIE", r.envp);
   FCGX_FPrintF(r.out,
    "Content-type: text/html\r\n"
    "Set-Cookie: kilo=0\r\n"
    "\r\n"); 
 // }
  FCGX_PutS("<script language=\"javascript\" type=\"text/javascript\"> "
            "window.location.href = "
            "\"http://192.168.1.50/art\";</script>",
            r.out);
}


void addtopic(FCGX_Request r)
{
  user luser = getuser(r);
  m = FCGX_GetParam("REQUEST_METHOD", r.envp);
  if (!strcmp(m, "POST"))
  {
    if (luser.uid)
    {
     // char *len = malloc(16);
      len = FCGX_GetParam("CONTENT_LENGTH", r.envp);
      int ilen = atoi(len);
      if ((ilen > 0) && (ilen < 140))
      {
        char *rawbufp = malloc(ilen);
      //  FCGX_GetStr(rawbufp, ilen, r.in);
        int i = 0;
        while (1 == 1)
        {
          while (i < ilen)
          {
            rawbufp[i] = FCGX_GetChar(r.in);
            i++;
          }
          if (i >= ilen)
          break;
        }
        char *bufp = url_decode(rawbufp);
        bufp[ilen] = 0;
        free(rawbufp);

        regex_t ex;
        int val;
//        val = regcomp(&ex, "[A-Za-z0-9;()-_;:.@А-Яа-я]", 0);
        char* newtopic = parse_post(bufp, "name=", 128);
        char *parent = (char*)malloc(16);
       // if(strstr(bufp,"parent="))
          parent = parse_post(bufp,"topic=",16);
    //    else 
    ///       parent="0";  
        if(!isNumber(parent)) parent = 0;  
        int ntlen = strlen(newtopic);
        if (ntlen > 0)
        {
  //        int res = regexec(&ex, newtopic, 0, NULL, 0);
          int ntlen = strlen(newtopic);
    //      if (!res)
          {
            char *end = malloc(ntlen * 2 + 1);
            mysql_real_escape_string(con, end, newtopic, ntlen);
            sprintf(query, "insert into topic (name, parent) values('%s', '%s');", newtopic, parent);
            mysql_query(con, query);
            FILE *fe = fopen("err", "a");
            fputs(bufp, fe);
            FCGX_PutS(mysql_error(con), r.out);
            fputs(mysql_error(con), fe);
            fclose(fe);
            FCGX_PutS("Added.", r.out);
            free(end);
        }
      // else
      //    FCGX_PutS("Only letter and numbers allowed.", r.out);
        }
        free(parent);
        free(newtopic);
        free(bufp);
      }
      else
        FCGX_PutS("Too long name.", r.out);
//        free(len);
  } }
    else
      FCGX_PutS("Forbidden.", r.out);
//  free(m);
}
/*
void topic(FCGX_Request r)
{

  user luser = header(r);
  char *m = malloc(16);
  m = FCGX_GetParam("REQUEST_METHOD", r.envp);
  if (!strcmp(m, "POST"))
  {
    if (luser.uid)
    {
      char *len = malloc(16);
      len = FCGX_GetParam("CONTENT_LENGTH", r.envp);
      int ilen = atoi(len);
      if ((ilen > 0) && (ilen < 130))
      {
        char *query = malloc(256);
        char *rawbufp = malloc(ilen);
       // FCGX_GetStr(rawbufp, ilen, r.in);
       int i = 0;;
       while (1 == 1)
       {
          while (i < ilen)
          {
            rawbufp[i] = FCGX_GetChar(r.in);
            i++;
          }
          if (i >= ilen)
          break;
        }
        char *bufp = url_decode(rawbufp);
        bufp[ilen] = 
        free(rawbufp);
        char *idnt = parse_post(bufp, "ident=", 8);
        if (strcmp(idnt, "addtopic") == 0)
        {
          regex_t ex;
          int val;
          val = regcomp(&ex, "[A-Za-z0-9;()-_;:.@A-Яа-я]", 0);
          char *newtopic = parse_post(bufp, "topic=", 128);
          int ntlen = strlen(newtopic);
          if (ntlen > 0)
          {
            int res = regexec(&ex, newtopic, 0, NULL, 0);
            int ntlen = strlen(newtopic);
            if (!res)
            {
              char *end = malloc(ntlen * 2 + 1);
              mysql_real_escape_string(con, end, newtopic, ntlen);
              sprintf(query, "insert into topic (name) values('%s');", newtopic);
              mysql_query(con, query);
              FILE *fe = fopen("err", "a");
              fputs(query, fe);
              FCGX_PutS(mysql_error(con), r.out);
              fputs(mysql_error(con), fe);
              fclose(fe);
            }
          }
          else
            FCGX_PutS("Only letter and numbers allowed.", r.out);
        }
        else
        {
          char *tid = parse_post(bufp, "topic=", 8);
          sprintf(query, "delete from topic where id=%s", tid);
        }
      }
      else
        FCGX_PutS("Too long name.", r.out);
    }
    else
      FCGX_PutS("Forbidden.", r.out);
  }

  char *left = malloc(256);
  mysql_query(con, "SELECT id, name from topic");
  MYSQL_RES *confres = mysql_store_result(con);
  MYSQL_ROW row;
  FCGX_PutS("<div class=sh><form action=\"/topic\" method=\"POST\" id=\"addtopic\" name=\"addtopic\"  >", r.out);
  FCGX_PutS("<p>New: <input type=\"text\" name=\"newtopic\"></p>", r.out);
  FCGX_PutS("<p><input type=\"hidden\" name=\"ident\" value=\"addtopic\">", r.out);
  // FCGX_PutS("<script>$(\"#addtopic\").submit(function(e) { e.preventDefault(); $(this).submit();});</script>",r.out);
  FCGX_PutS("<input type=\"submit\" value=\"+\"  >", r.out);

  FCGX_PutS("<form action=\"/topic\" method=\"POST\" id=\"deltopic\" name=\"deltopic\">", r.out);
  FCGX_PutS("<input type=\"submit\" value=\"-\" form=\"deltopic\" ></p>", r.out);
  FCGX_PutS("<input type=\"hidden\" name=\"ident\" value=\"deltopic\">", r.out);
  FCGX_PutS("<select name=\"topic\" size=\"35\" form=\"deltopic\">", r.out);

  if (confres)
    while (row = mysql_fetch_row(confres))
    {
      sprintf(left, "<option value=\"%s\">%s</option>", row[0], row[1]);
      FCGX_PutS(left, r.out);
    }
  FCGX_PutS("</select></form></div>", r.out);

  /*  char *len = malloc(16);
    len = FCGX_GetParam("CONTENT_LENGTH", r.envp);
    int ilen = atoi(len);
    if ((ilen > 0) && (ilen < 140))
    {
      char *query = malloc(ilen + 40);
      char *rawbufp = malloc(ilen);
      FCGX_GetStr(rawbufp, ilen, r.in);
      char *bufp = url_decode(rawbufp);
      bufp[ilen] = 0;
      free(rawbufp); */

  // topic = parse_post(bufp,"topic=",128);
  //     FILE *fe = fopen("err", "a");
  //     fputs(query, fe);
  //     FCGX_PutS(mysql_error(con), r.out);
  //     fputs(topic, fe);
  //   sprintf("<p>%d</>",topic);
  //    fclose(fe);
  // }
/*
  echofile("footer.tpl", r);
}
*/
//#define HUGEQUERY(id) sprintf(query1,"WITH RECURSIVE msg_path (id,date, data, parent, lvl, path, name, subj) AS ( SELECT id, date, SUBSTRING(data,1,600), parent, 0 lvl, data as path, (select name from users where users.id = msg.owner),subj FROM msg WHERE id = '%s'  UNION ALL SELECT msg.id, msg.date, SUBSTRING(msg.data,1,600), msg.parent, msgp.lvl + 1, concat(msgp.path, \">\", msg.data),(select users.name from users where users.id = msg.owner), msg.subj FROM msg_path AS msgp  JOIN msg AS msg ON msgp.id = msg.parent ) SELECT * FROM msg_path order by path desc, date asc",cid);


void noc(FCGX_Request r)
{
  user luser = getuser(r);
  if (luser.admin)
  {

      echofile("header.tpl", r);
      FCGX_PutS("<p><div class=left><a href=\"/noc=srv\">Servers</a><br>"  , r.out);
      FCGX_PutS("<a href=\"/noc=dns\">DNS</a></br></p></div>", r.out);
      nu = FCGX_GetParam("REQUEST_URI", r.envp);
        char *n = strstr(nu, "/noc");
      if (n)
      {
        char *e = strchr(nu, 0);
        char *mmm = malloc(65);
        e = strchr(nu, 0);
        int plen = e - (nu + 5);
        if (plen > 0)
        memcpy(mmm, nu + 5, plen);
        mmm[plen] = 0;
        if (strcmp(mmm,"dns") == 0){
          FCGX_PutS("<div class=com>", r.out);
          
          sprintf(query,"select name, s1.hostname,s2.hostname from domains d inner join nodes s1 on d.master=s1.id inner join nodes s2 ON d.slave=s2.id where d.owner=%d;",luser.uid);
          mysql_query(con, query);
          FCGX_PutS(mysql_error(con), r.out);
          FCGX_PutS("<table><tr><td>domain</td><td>master</td><td>slave</td></tr>",r.out);
          int totalrows = 0;
          MYSQL_RES *confres = mysql_store_result(con);
          if (confres)
          {

            totalrows = mysql_num_rows(confres);
            if (totalrows > 0)
            {
              MYSQL_ROW row;
              while (row = mysql_fetch_row(confres))
              { 
                sprintf(left,"<tr><td><a href=\"/noc=dns&domain=%s>%s</a></td><td>%s</td><td>%s</td></tr>",row[0],row[0],row[1],row[2]);
                FCGX_PutS(left,r.out);
              }
            }
          }
          FCGX_PutS("</div></table>",r.out);
        } 
        if(strstr(nu,"/noc=dns&domain=")) {
          e = strchr(nu, 0);
          int plen = e - (nu + 16);
          if (plen > 0)
          memcpy(mmm, nu + 16, plen);
          mmm[plen] = 0;
          
        }
        //else justone(30,r);
      echofile("footer.tpl",r);
    }
  }
}


void one(long int id, int cutflag, FCGX_Request r)
{
  mysql_query(con, "SELECT max(id) from msg;");
  FCGX_PutS(mysql_error(con), r.out);
  MYSQL_RES *confres2 = mysql_store_result(con);
  char *cut = malloc(600);
  //char *more = malloc(64);
  //char *cid = (char *)malloc(16);
  //  sprinf(cid,"%d",id);
  if (confres2)
  {
    MYSQL_ROW row2 = mysql_fetch_row(confres2);
    long int max = atoi(row2[0]);
    long int min = max - 50;
    if (min < 0)
    {
      min = 0;
    }
    if (cutflag)
      // query1 = HUGEQUERY(cid);
      sprintf(
          query,
          "WITH RECURSIVE msg_path (id,date, data, parent, lvl, path, name, subj) AS "
          "( SELECT id, date, SUBSTRING(data,1,600), parent, 0 lvl, data as path, (select name "
          "from users"
          " where users.id = msg.owner),subj FROM msg WHERE id = '%d' "
          " UNION ALL SELECT msg.id, msg.date, SUBSTRING(msg.data,1,600), msg.parent, msgp.lvl + 1,"
          " concat(msgp.path, \">\", msg.data),(select users.name from users "
          "where users.id = msg.owner), msg.subj FROM msg_path AS msgp"
          " JOIN msg AS msg ON msgp.id = msg.parent ) SELECT * FROM msg_path order by date asc ",
          id);

    else
      sprintf(
          query,
          "WITH RECURSIVE msg_path (id,date, data, parent, lvl, path, name, subj) AS "
          "( SELECT id, date, data, parent, 0 lvl, data as path, (select name "
          "from users"
          " where users.id = msg.owner),subj FROM msg WHERE id = '%d' "
          " UNION ALL SELECT msg.id, msg.date, msg.data, msg.parent, msgp.lvl + 1,"
          " concat(msgp.path, \">\", msg.data),(select users.name from users "
          "where users.id = msg.owner), msg.subj FROM msg_path AS msgp"
          " JOIN msg AS msg ON msgp.id = msg.parent ) SELECT * FROM msg_path order by date asc ",
          id);

    //"order by CASE when parent = 0 then date end desc, path asc, CASE when
    // parent<>0 then path end desc, date asc", 0 , max, min );
    // sprintf(left,"%d",strlen(query1));
    // FCGX_PutS(left, r.out);

    mysql_real_query(con, query, strlen(query));
    FCGX_PutS(mysql_error(con), r.out);
    int totalrows1 = 0;
    MYSQL_RES *confres1 = mysql_store_result(con);
    if (confres1)
    {

      totalrows1 = mysql_num_rows(confres1);
      if (totalrows1 > 0)
      {
        MYSQL_ROW row1;
        while (row1 = mysql_fetch_row(confres1))
        {

          int lvl = atoi(row1[4]);
   //       int n = 30 + 2 * lvl;
   //       int m = 70;
          //FCGX_PutS("<div class=com>", r.out);
          sprintf(left, "<div class=com>");
          FCGX_PutS(left, r.out);
          if (row1[7])
          {
            FCGX_PutS("<h1>", r.out);
            FCGX_PutS(row1[7], r.out);
            FCGX_PutS("</h1>", r.out);
          }
          FCGX_PutS(row1[2], r.out);
          if (cutflag)
          {
            sprintf(left, "<p><a href=\"http://192.168.1.50/more?%s\">more...</a></p>", row1[0]);
            FCGX_PutS(left, r.out);
          }
          sprintf(left,
                  "<br><p style=\"font-size: 12px;\", color: red;\">Posted by "
                  "%s on %s",
                  row1[6], row1[1]);
          FCGX_PutS(left, r.out);
          // free(left);
          sprintf(left,
                  "<div id=\"blogcom\" style=\"width: 550px;\" ><form "
                  "action=\"/addmsg?%s\" method=\"POST\" id=\"blogco\">"
                  "<textarea name=\"test1\">Сomment</textarea>"
                  "<input type=\"submit\" value=\"post\"></form></div>",
                  row1[0]);
          FCGX_PutS(left, r.out);

          FCGX_PutS("</div>", r.out);
          FCGX_PutS("</div>", r.out);
        }
      }
    }
    //mysql_free_result(confres1);
  }
  //mysql_free_result(confres2);
//  free(cut);
//  free(more);
}

void art(FCGX_Request r)
{  
  FCGX_PutS("\r\n", r.out);  
  FCGX_PutS("\r\n", r.out);
  header(r);
  //   FCGX_PutS("<div  class="sh"><center><img
  //   src=\"pdn.png\"></center></div>", r.out)
  mysql_query(con, "SELECT id from msg where parent=0 order by date desc;");
  FCGX_PutS(mysql_error(con), r.out);
  MYSQL_RES *confres = mysql_store_result(con);
  MYSQL_ROW row;
  if (confres)
    while (row = mysql_fetch_row(confres))
    {
      long int id = atoi(row[0]);
      one(id, 1, r);
    }
}

void more(FCGX_Request r)
{
//  char *nu = malloc(32);
  nu = FCGX_GetParam("REQUEST_URI", r.envp);

  if (strstr(nu, "more"))
  {
    header(r);
//    char *e = malloc(32);
    //char *e = strchr(nu, 0);
    //char *plen = malloc(32);
  //  int plen = e - (nu + 6);
  //mak  memcpy(m, nu + 6, plen);
    char *e = strchr(nu, 0);
    char *m = malloc(6);
    int plen = e - (nu + 6);
    memcpy(m, nu + 6, plen);
    m[plen] = 0;
    long int id = atoi(m);
    one(id, 0, r);
    //m[plen] = 0;
    int uid = getsuid(r);

    echofile("footer.tpl", r);
  }
 // free(nu);
}


void justone(long int id, FCGX_Request r)
{
    sprintf(query,"SELECT msg.id, msg.date, data, parent, name, subj from msg left join users on msg.owner=users.id where msg.id='%d'",id);

    //"order by CASE when parent = 0 then date end desc, path asc, CASE when
    // parent<>0 then path end desc, date asc", 0 , max, min );
    // sprintf(left,"%d",strlen(query1));
    // FCGX_PutS(left, r.out);
    mysql_real_query(con, query, strlen(query));
    FCGX_PutS(mysql_error(con), r.out);
    int totalrows1 = 0;
    MYSQL_RES *confres1 = mysql_store_result(con);
    if (confres1)
    {

      totalrows1 = mysql_num_rows(confres1);
      if (totalrows1 > 0)
      {
        MYSQL_ROW row1;
        while (row1 = mysql_fetch_row(confres1))
        {

          FCGX_PutS("<div class=sh>", r.out);
          sprintf(left, "<div class=com>",
                  3, m);
          FCGX_PutS(left, r.out);
          FCGX_PutS(row1[2], r.out);
          sprintf(left,
                  "<br><p style=\"font-size: 12px;\", color: red;\">Posted by "
                  "%s on %s",
                  row1[4], row1[1]);
          FCGX_PutS(left, r.out);
          // free(left);
          
          FCGX_PutS("</div>", r.out);
          FCGX_PutS("</div>", r.out);
        }    
      }
    }
}
//}
/*
char *leprozory(const char *input)
{
  TidyBuffer output = {0};
  TidyBuffer errbuf = {0};
  int rc = -1;
  Bool ok;
  TidyDoc tdoc = tidyCreate(); // Initialize "document

  ok = tidyOptSetBool(tdoc, TidyXhtmlOut, yes); // Convert to XHTML
  if (ok)
    rc = tidySetErrorBuffer(tdoc, &errbuf); // Capture diagnostics
  if (rc >= 0)
    rc = tidyParseString(tdoc, input); // Parse the input
  if (rc >= 0)
    rc = tidyOptSetBool(tdoc, TidyMark, no);
  if (rc >= 0)
    rc = tidyOptSetInt(tdoc, TidyWrapLen, 0);
  if (rc >= 0)
    rc = tidyOptSetInt(tdoc, TidyBodyOnly, 1);
  if (rc >= 0)
    tidySetCharEncoding(tdoc, "utf8");
  if (rc >= 0)
    rc = tidyOptSetInt(tdoc, TidyDoctypeMode, TidyDoctypeOmit);
  if (rc >= 0)
    rc = tidyCleanAndRepair(tdoc); // Tidy it up!
  if (rc >= 0)
    rc = tidyRunDiagnostics(tdoc); // Kvetch
  if (rc > 1)                      // If error, force output.
    rc = (tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1);
  if (ok)
    rc = tidySetErrorBuffer(tdoc, &errbuf); // Capture diagnostics

  char *cleansed_buffer_ = (char *)malloc(1);
  rc = tidySaveBuffer(tdoc, &output);
  cleansed_buffer_ = (char *)malloc(output.size + 1);
  memcpy((char *)cleansed_buffer_, (char *)output.bp, output.size);

  //   rc = tidySaveString(tdoc, cleansed_buffer_, &size );
  /*    printf( "\nDiagnostics:\n\n%s", errbuf.bp );

      printf( "\nAnd here is the result:\n\n%s", output); */

/*  return cleansed_buffer_;
  tidyBufFree(&errbuf);
  tidyRelease(tdoc);
}
*/
void addmsg(FCGX_Request r)
{
  //  send_headers(r);
//  char *nu = (char *)malloc(32);
  nu = FCGX_GetParam("REQUEST_URI", r.envp);
  char *n = strstr(nu, "/addmsg");
  if (n)
  {
    //  echofile("header.tpl", r);
    user luser = getuser(r);
    if (luser.uid)
    {

      //char *eight = (char *)malloc(8);
      //sprintf(eight, "%d", luser.uid);
    //  char *m = malloc(5);
      m = FCGX_GetParam("REQUEST_METHOD", r.envp);
      //      FCGX_PutS(method,r.out);
      if (!strcmp(m, "POST"))
      {
        //	    FCGX_PutS(method,r.out);
      //  char *len = malloc(16);
        len = FCGX_GetParam("CONTENT_LENGTH", r.envp);
        int ilen = atoi(len);
        if (ilen > 0)
        {
          char *rawbufp = malloc(ilen);
          FCGX_GetStr(rawbufp, ilen, r.in);
          char *bufp = url_decode(rawbufp);
          bufp[ilen] = 0;

          free(rawbufp);

          char *e = strchr(nu, 0);
          char *parent = malloc(8);
          int plen = e - (nu + 8);
          memcpy(parent, nu + 8, plen);
          parent[plen] = 0;

          char *s2 = parse_post(bufp, "test1=", 0);
      //   char *s3 = leprozory(s2);
      /*    int query_size =
              strlen(s2) +
              strlen("INSERT INTO msg (owner, data, parent) VALUES "
                     "('%s', '%s', '%d')") +
              1024; */
          long int size = ilen * 2 + 1;
          char *end = malloc(size);
          mysql_real_escape_string(con, end, s2, strlen(s2));
          char *querybig = malloc(size);
          sprintf(
              querybig,
              "INSERT INTO msg (owner, data, parent) VALUES ('%u', '%s', '%s')",
              luser.uid, s2, parent);
          FCGX_PutS("Content-type: text/html\r\n\r\n", r.out);
          FILE *fe = fopen("err", "a");
          fputs(bufp, fe);
          mysql_query(con, querybig);
          FCGX_PutS(mysql_error(con), r.out);
          fputs(mysql_error(con), fe);
          fclose(fe);

          FCGX_PutS(mysql_error(con), r.out);
          //  free(parent);
//          free(s3);
          free(s2);
          free(querybig);
          free(bufp);
        //  free(eight);
        // free(m);
          free(parent);
        //  free(len);
          free(end);
        } // there in post zero.
        FCGX_PutS("Location: http://192.168.1.50/art", r.out);
        FCGX_PutS("<script language=\"javascript\" type=\"text/javascript\"> "
                  "window.location.href = "
                  "\"http://192.168.1.50/art\";</script>",
                  r.out);
      }
    }
    else
      FCGX_PutS("You are not registerd", r.out);
  }
// free(n);
 // free(nu);
}

void addart(FCGX_Request r)
{
  //  send_headers(r);
//  char *nu = (char *)malloc(32);
  //  echofile("header.tpl", r);
  user luser = getuser(r);
  if (luser.uid)
  {

  //  char *eight = (char *)malloc(8);
  //  sprintf(eight, "%d", uid);
    char *m = malloc(5);
    m = FCGX_GetParam("REQUEST_METHOD", r.envp);
    //      FCGX_PutS(method,r.out);
    if (!strcmp(m, "POST"))
    {
      //	    FCGX_PutS(method,r.out);
    //  char *len = malloc(16);
      len = FCGX_GetParam("CONTENT_LENGTH", r.envp);
      uint ilen = atoi(len);
      if (ilen > 0)
      {
        char *rawbufp = malloc(ilen);
        FCGX_GetStr(rawbufp, ilen, r.in);
        char *bufp = url_decode(rawbufp);
        bufp[ilen] = 0;

        free(rawbufp);

        char *s4 = parse_post(bufp, "subj=", 0);
    //    char *s5 = leprozory(s4);
        char *s2 = parse_post(bufp, "editordata=", 0);
  //      char *s3 = leprozory(s2);
      /*  int query_size =
            ilen +
            strlen("INSERT INTO msg (owner, data, parent,subj) VALUES "
                   "('%s', '%s', '%d', '%s')") +
            100; */
        char* topic = parse_post(bufp,"topic=",16);
        if(!isNumber(topic)) topic=0;
        long int size = ilen * 2 + 1;
        char *querybig = malloc(size);
        char *end = malloc(size);
        mysql_real_escape_string(con, end, s2, strlen(s2));
        char *endsubj = malloc(strlen(s4) * 2 + 1);
        mysql_real_escape_string(con, endsubj, s4, strlen(s4));
        if (!topic) topic = "0";
        sprintf(querybig,
                "INSERT INTO msg (owner, data, parent, subj, topic) VALUES ('%d', "
                "'%s', '%d', '%s', '%s');",
                luser.uid, end, 0, endsubj, topic);
        FCGX_PutS("Content-type: text/html\r\n\r\n", r.out);
        FILE *fe = fopen("err", "a");
        fputs(querybig, fe);
        mysql_query(con, querybig);
        FCGX_PutS(mysql_error(con), r.out);
        fputs(mysql_error(con), fe);
        fclose(fe);
        //        free(parent);
        //   free(s3);
        free(s2);
        free(s4);
    //    free(s3);
    //    free(s5);
        //  free(s5);
        free(querybig);
        free(bufp);
        free(end);
        free(endsubj);
//        free(topic);

      } // there in post zero.
      FCGX_PutS("Location: http://192.168.1.50/art", r.out);
      FCGX_PutS("<script language=\"javascript\" type=\"text/javascript\"> "
                "window.location.href = "
                "\"http://192.168.1.50/art\";</script>",
                r.out);

//      free(len);
    }
   // free(m);
  }
  else
    FCGX_PutS("You are not registerd", r.out);
//    free(nu);
}

#define MAXLINE 38400
#define FILESIZE 37632
#define MAXSUB 38016

char boundary[40] = "---------------------------";

/*ssize_t process_https(int sockfd, char *host, char *page, char *boundary, char
*poststr)
{
    char sendline[MAXLINE + 1], recvline[MAXLINE + 1];
    ssize_t n;
    snprintf(sendline, MAXLINE,
     "POST /%s HTTP/1.0\r\n"
     "Host: %s\r\n"
     "Content-type: multipart/form-data; boundary=%s\r\n"
     "Content-length: %d\r\n\r\n"
     "%s", page, host, boundary, strlen(poststr), poststr);
    write(sockfd, sendline, strlen(sendline));
    while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
    recvline[n] = '\0';
    printf("%s", recvline);
    }
    return n;
} */

void image(FCGX_Request r)
{
  // send_headers(r);
  // FCGX_PutS("Accept-Post: */*\r\n",r.out);
  // FCGX_PutS("\r\n\r\n",r.out);

  user luser = getuser(r);
  if (luser.uid)
    if (!strcmp(FCGX_GetParam("REQUEST_METHOD", r.envp), "POST"))
    {
      char *bufp = malloc(4096);
      char *buf = malloc(40060000);
      /*    int i = 0;
          int j = 0;
          char *start = bufp;
          while (bufp[i] = r.in) {
              char *found = strstr(start,"\r\n");
              if (found) {
                int len = found - start;
                char *jnbuf[j]; jnbuf[j] = malloc(len+1);
                memcpy(jnbuf[j], start, len +1);
                jnbuf[j][len] = 0;
                //char *knbuf[k]; start;
                j++;
                start = start + 2;
             }
            i++;
          }
      */
    //  char *len = malloc(16);
      len = FCGX_GetParam("CONTENT_LENGTH", r.envp);
      int ilen = atoi(len);

      char *found;
      char *bnum = malloc(40);
      char *filename = malloc(64);
      char *contype = malloc(32);
      int i = 0;
      const char *ch;
      while (bufp[i] = FCGX_GetChar(r.in))
      {
        // found = strstr(bufp,"--\r\n");
        // ch = (char *)&bufp[i];
        if (strcmp((char *)&bufp[i], "-") != 0)
          break;
        i++;
        if (i > 100)
          break;
      }
      bufp[0] = bufp[i];
      i = 1;
      while (bufp[i] = FCGX_GetChar(r.in))
      {
        if (found = strstr(bufp, "\r\n"))
          break;
        i++;
        if (i > 40)
          break;
      }
      bufp[i - 1] = 0;
      bufp[i] = 0;
      memcpy(bnum, bufp, strlen(bufp));
      i = 0;
      found = "";
      while (bufp[i] = FCGX_GetChar(r.in))
      {
        if (found = strstr(
                bufp,
                "Content-Disposition: form-data; name=\"image\"; filename=\""))
          break;
        i++;
        if (i > 256)
          break;
      }
      // bufp[i] = FCGX_GetChar(r.in);
      i = 0;
      while (filename[i] = FCGX_GetChar(r.in))
      {
        if (strstr(filename, "\"\r\n"))
          break;
        i++;
        if (i > 64)
          break;
      }
      filename[i - 2] = 0;
      i = 0;
      while (bufp[i] = FCGX_GetChar(r.in))
      {
        if (strstr(bufp, "Content-Type: "))
          break;
        i++;
        if (i > 32)
          break;
      }
      i = 0;
      while (contype[i] = FCGX_GetChar(r.in))
      {
        if (strstr(contype, "\r\n\r\n"))
          break;
        i++;
        if (i > 32)
          break;
      }
      contype[i - 3] = 0;
      /*   }    while (buf[i] = FCGX_GetChar(r.in)) {
            //found = strstr(bufp,"--\r\n");
           // ch = (char *)&bufp[i];
            if (strcmp((char *)&buf[i], "-") != 0)
            break;
            i++;
         } */
      ch = (char *)"\020";
      FCGX_PutS("Accept-Post: application/octet-stream\r\n\r\n", r.out);
      //   FCGX_PutS("Accept-Post: ",r.out);
      //   FCGX_PutS(contype,r.out);
      //   FCGX_PutS("\r\n\r\n",r.out);
      //   FCGX_PutS("Accept-Encoding: gzip, deflate\r\n\rn",r.out);

      //   FCGX_PutS("Accept-Post: */*;\r\n\r\n",r.out);

      //   if (!strcmp(FCGX_GetParam("REQUEST_METHOD", r.envp), "POST"))
      //   {
      // FCGX_GetStr(buf,ilen,r.in);
      // FCGX_GetStr(buf,ilen,r.in);

      //   }

      // FCGX_PutS("Accept: *\r\n\r\n",r.out);

      // FCGX_GetStr(buf,ilen,r.in);
      i = 0;
      int size = 0;
      //  while(sizeof(buf) < ilen){
      // while (i < ilen) {
      while (1 == 1)
      {
        while (i < ilen)
        {
          buf[i] = FCGX_GetChar(r.in);
          i++;
        }
        if (i >= ilen)
          break;
      }
      char *fname = malloc(96);
      sprintf(fname, "upload/%s", filename);
      FILE *f = fopen(fname, "w");
      // int m = strlen(buf);
      int n = fwrite(buf, 1, i, f);
      fclose(f);
      //   FCGX_PutS(len,r.out);
      char *st = malloc(100);
      //  FCGX_PutS("Content-type: text/html\r\n\r\n",r.out);
      sprintf(st, "dev.shushik.kiev.ua/%s\r\n", fname);
      printf(st);
      FCGX_PutS(st, r.out);
      //  FCGX_PutS(filename,r.out);
      //  FCGX_PutS("}",r.out);

      // FCGX_PutS(buf,r.out);

      // bufp[i] = 0;
      // filename = bufp;

      //0x7ffff6a6a010 '-' <repeats 29 times>, "\
//Content-Disposition: form-data; name=\"image\"; filename=\"

      // 235103533627700036105989979\r\nContent-Disposition: form-data;
      // name=\"image\"; filename=\"Iam.jpg\"\r\nContent-Type:
      // image/jpeg\r\n\r\n\377\330\377", <incomplete sequence \340>
      free(bnum);
      free(filename);
      free(contype);
      free(bufp);
      free(buf);
    }
}

void add(FCGX_Request r)
{
  FCGX_PutS("\r\n", r.out);
  FCGX_PutS("\r\n", r.out);
  user luser = getuser(r);
  
  echofile("shead.html", r);
  
  
      if (luser.uid)
      {  
        sprintf(left, "<div class=hello><p>Welcome,%s!</p></div><div class=menu><a class=sh href=\"/add\">Add</a><a class=sh href=\"/quit\">Quit</a></div>", luser.name);
        FCGX_PutS(left, r.out);
      
        echofile("new.html", r); 

          //char *query = (char * )malloc(256);


          FCGX_PutS("<li><input type=\"radio\" id=\"topic0\" name=\"topic\" value=\"0\" checked=\"checked\"><label for=\"topic0\">0</label></li>",r.out);
         
          sprintf(
          query,
          "WITH RECURSIVE topic_path (id,name, parent, lvl, path) AS (SELECT id, name, parent, 0 lvl ,  name as path FROM topic  where parent = 0 UNION ALL SELECT topic.id, topic.name, topic.parent, topicp.lvl + 1, concat(topicp.path, \">\", topic.name) FROM topic_path AS topicp JOIN topic AS topic ON topicp.id = topic.parent) SELECT * FROM topic_path order by path asc; ");  
          
          mysql_query(con,query);
          FCGX_PutS(mysql_error(con), r.out);

          MYSQL_RES *confres4 = mysql_store_result(con);  
          MYSQL_ROW row;

          int lvl = 0;
          int lvlup = 0;
          while(row = mysql_fetch_row(confres4)) {
            lvl = atoi(row[3]);
            if (lvlup < lvl) {
              while(lvlup != lvl) {
                FCGX_PutS("<ul>",r.out);
                lvlup++;
              }
            }
            if ((lvlup >  lvl)) { 
              FCGX_PutS("</ul>",r.out);
              while(lvlup != lvl) {
                FCGX_PutS("</ul>",r.out);
                lvlup--;
              }      
            }
          
            sprintf(left,"<li><input type=\"radio\" id=\"topic%s\" name=\"topic\" value=\"%s\"><label for=\"topic%s\">%s</lable></li>",row[0],row[0],row[0],row[1]);
            FCGX_PutS(left,r.out);
            }

            FCGX_PutS("<p><input type=\"submit\" value=\"Отправить\"></p>",r.out);
      
        FCGX_PutS("</form></div>", r.out);
      }
      echofile("footer.tpl",r);
  }
      
    



void newtopic(FCGX_Request r)
{
/*
 char *nu = (char *)malloc(32);
  nu = FCGX_GetParam("REQUEST_URI", r.envp);

  if (strstr(nu, "newtopic"))
  {  */
  FCGX_PutS("\r\n", r.out);
  FCGX_PutS("\r\n", r.out);
    user luser= header(r);
    if(luser.uid)
    { 
    MYSQL_ROW row;
    //MYSQL_ROW row1;
         
/*    char *e = strchr(nu, 0);
    char *mid = malloc(16);
    e = strchr(nu, 0);
    int plen = e - (nu + 5);
            memcpy(mid, nu + 5, plen); // char *str = (char *)malloc(110) */
        
        
//          mid[plen] = 0;

          char *query = malloc(500);   
          FCGX_PutS("<div class=sh>",r.out);
          sprintf(
          query,
          "WITH RECURSIVE topic_path (id,name, parent, lvl, path) AS (SELECT id, name, parent, 0 lvl ,  name as path FROM topic  where parent = 0 UNION ALL SELECT topic.id, topic.name, topic.parent, topicp.lvl + 1, concat(topicp.path, \">\", topic.name) FROM topic_path AS topicp JOIN topic AS topic ON topicp.id = topic.parent) SELECT * FROM topic_path order by path asc; ");  
          
          
          
          mysql_query(con,query);
          FCGX_PutS(mysql_error(con), r.out);
          MYSQL_RES *confres4 = mysql_store_result(con);  
          int lvl = 0;
          int lvlup = 0;
//<input type=\"radio\" id=\"topic%s\" name=\"topic\" value=\"%s\"><label for=\"topic%s\">%s</label>
          FCGX_PutS("<ul><form action=\"#\" method=\"POST\">",r.out);
          FCGX_PutS("<li><input type=\"radio\" id=\"topic0\" name=\"topic\" value=\"0\" checked=\"checked\"><label for=\"topic0\">0</label></li>",r.out);
          int ln = 0;
          int lo = 0;
          while(row = mysql_fetch_row(confres4)) {
            lvl = atoi(row[3]);
            if (lvlup < lvl) {
              while(lvlup != lvl) {
                FCGX_PutS("<ul>",r.out);
                lvlup++;
              }
            }
            if ((lvlup >  lvl)) { 
              FCGX_PutS("</ul>",r.out);
              while(lvlup != lvl) {
                FCGX_PutS("</ul>",r.out);
                lvlup--;
              }      
            }
          
         
            sprintf(left,"<li><input type=\"radio\" id=\"topic%s\" name=\"topic\" value=\"%s\"><label for=\"topic%s\">%s</lable></li>",row[0],row[0],row[0],row[1]);
            FCGX_PutS(left,r.out);
          }  

      //    sprintf(left,"New:<br><input type=\"text\" name=\"name\" id=\"name\"><button type=\"submit\">Ok</button></form><script>$(document).ready(function(){$('button').click(function(event){event.preventDefault();var	name = $('#name').val(); var topic = $('#topic').val(); $.ajax({ type:\"POST\", url: \"https://185.151.245.8/addtopic\", data: $(\"form\").serialize(), dataType: \"json\", success: function(result){}});}); });  if (window.localStorage)  if (!localStorage.getItem('reload')) { localStorage['reload'] = true; window.location.reload(); } else {localStorage.removeItem('reload');}   </script>");
          sprintf(left,"New:<br> <form action=\"#\" method=\"POST\"><input type=\"text\" name=\"name\" id=\"name\"><button type=\"submit\">Ok</button></form><script>$(document).ready(function(){$('button').click(function(event){event.preventDefault();var	name = $('#name').val(); var shit = $('#topic').val(); $.ajax({ type:\"POST\", url: \"/addtopic\", data: $(\"form\").serialize(), dataType: \"json\", success: function(result){}}); reloadPage();   }); });</script>");
    
          //  FCGX_PutS("Create new, as a child of selcted:<br> <input type=\"text\" name=\"name\" id=\"name\"><input type=\"submit\" value=\"Ok\"><script> $('form').on('submit', function(){event.preventDefault(); var	name = $('#name').val(); var topic = $('#topic').val(); $.ajax({ type:\"POST\", url: \"/addtopic\", data: { name:name, topic:topic }, dataType: \"json\", success: function(result){}})location.reload(); });});});</script>",r.out);    
      //  sprintf(left,"<p>Create new, as a child of selcted:<br>  <input type=\"text\" name=\"name\" id=\"name\"><button type=\"submit\">Ok</button><script>$(document).ready(function(){$('button').click(function(event){event.preventDefault();var	name = $('#name').val(); var topic =  $('#topic').val(); $.ajax({ type:\"POST\", url: \"/addtopic\",  data: $(\"form\").serialize(), dataType: \"json\", success: function(result){}})location.reload(); });});</script></p>");
          FCGX_PutS(left,r.out);
          FCGX_PutS("</form></ul>",r.out);
          

    /*      FCGX_PutS("<form action=\"\" method=\"post\">",r.out);
          mysql_query(con, "SELECT id, name from topic");
          MYSQL_RES *confres = mysql_store_result(con);
          if (confres)
          while (row1 = mysql_fetch_row(confres))
            { 
              sprintf(left, "<input type=\"radio\" value=\"%s\" id=\"topic\" name=\"topic\">  %s<br>",row1[0], row1[1]);
              FCGX_PutS(left, r.out);
            }

            sprintf(query1,"<li><input type=\"radio\" id=\"topic%s\" name=\"topic\" value=\"%s\"><label for=\"topic%s\">%s</label></li>",row[0],row[0],row[0], row[1]);
            
   /*         sprintf(left,"Create new, as a child of selcted:<br> <input type=\"text\" name=\"name\" id=\"name\"><button type=\"submit\">Ok</button></form><script>$(document).ready(function(){$('button').click(function(event){event.preventDefault();var	name = $('#name').val(); var topic = $('#topic').val(); $.ajax({ type:\"POST\", url: \"/addtopic\", data: { name:name, topic:topic }, dataType: \"json\", success: function(result){}})location.reload(); });});</script>");
          FCGX_PutS(left,r.out);
          FCGX_PutS("</form></div>",r.out); */
  /*        FCGX_PutS("<div class=left>",r.out); 
          
          
          
          mysql_query(con, "select id, subj from msg");
          MYSQL_RES *confres1 = mysql_store_result(con); 
          
          while (row = mysql_fetch_row(confres1))
          {
              sprintf(left, "<a href=\"http://192.168.1.50/menu%s\">%s</a> <br>",row[0],row[1]);
              FCGX_PutS(left,r.out);
          }
          FCGX_PutS(mysql_error(con), r.out);
          FCGX_PutS("</div>",r.out); */
        
    //      sprintf(query, "select subj,data,topic from msg left join topic on topic.id = msg.topic where msg.id ='%s'", mid);
        /*  if (confres1)
            while (row = mysql_fetch_row(confres1))
            {
              sprintf(left, "<a href=\"http://192.168.1.50/menu%s\"> %s %s<a> <br>", row[0], row[2], row[1]);
              FCGX_PutS(left, r.out);
            }
          */  
  /*        mysql_query(con, query);
          MYSQL_RES *confres2 = mysql_store_result(con);
          row = mysql_fetch_row(confres2);
          FCGX_PutS(mysql_error(con), r.out);
          // echofile("edit1.html",r);
          sprintf(left, "<div class=\"sh\"><p>Subject: %s</p>"
                        "Now he is here: %s<br>"
                         "And in what place do you wnt to see it?<br>",row[0],row[2]);
           FCGX_PutS(left,r.out); 
           FCGX_PutS("<br><input type=\"radio\" value=\"0\" name=\"topic\" checked> No topic <br>",r.out); 
          mysql_query(con, "SELECT id, name from topic");
          MYSQL_RES *confres3 = mysql_store_result(con);
          
          
           if (confres3)
           while (row1 = mysql_fetch_row(confres3))
            { 
              sprintf(left, "<input type=\"radio\" value=\"%s\"name=\"topic\"'>  %s<br>",row1[0], row1[1]);
              FCGX_PutS(left, r.out);
            }
            FCGX_PutS("<p><button type=\"submit\">Send it there</button></form><script>$(document).ready(function(){$('button').click(function(event){event.preventDefault();var	name = $('#name').val(); var	topic = $('#topic').val(); $.ajax({ type:\"POST\", url: \"/addtopic\", data: { name:name, topic:topic }, dataType: \"json\", success: function(result){}});});});</script></p>", r.out);  

          //FCGX_PutS(left, r.out);
          // Here is data going */
     //     free(query);
     //    free(left);
      }
    
  

  echofile("footer.tpl",r);
 }
//}

void bin(FCGX_Request r)
{
 // char *nu = (char *)malloc(32);
  nu = FCGX_GetParam("REQUEST_URI", r.envp);

  if (strstr(nu, "bin"))
  {
    header(r);
    char *e = strchr(nu, 0);
    char *mid = malloc(16);
    e = strchr(nu, 0);
    int plen = e - (nu + 4);
    memcpy(mid, nu + 4, plen); // char *str = (char *)malloc(110);
    char *kilo = (char *)calloc(200, sizeof(char));
    char *cookie = (char *)malloc(69);
    cookie = FCGX_GetParam("HTTP_COOKIE", r.envp);
    if (FCGX_GetParam("HTTP_COOKIE", r.envp) != NULL)
    {
      cookie = FCGX_GetParam("HTTP_COOKIE", r.envp);
    }
    if (cookie)
    {
      char *kilo = (char *)malloc(70);
      char *query = (char *)malloc(300);
      memcpy(kilo, cookie + 5, 64);
      kilo[64] = 0;
      //   FCGX_PutS(kilo, r.out);
      sprintf(query, "select id, name from users where kilo='%s' and admin='1'",
              kilo);
      mysql_query(con, query);
      //   FCGX_PutS(mysql_error(con), r.out);
      MYSQL_RES *confres = mysql_store_result(con);
      if (confres != NULL)
      {
        int totalrows = mysql_num_rows(confres);
        if (totalrows > 0)
        {
          MYSQL_ROW row;
          row = mysql_fetch_row(confres);
          //    mysql_free_result(confres);
          //	CGX_PutS(row[0]F,r.out);
          //  int uid = atoi(row[0]);

          //   FCGX_PutS(row[0], r.out);
          echofile("shead.html", r);
          FCGX_PutS("<div class=left>test", r.out);
          char *left = (char *)malloc(256);
          mysql_query(con, "select id, subj from msg;");
          MYSQL_RES *confres1 = mysql_store_result(con);
          FCGX_PutS(mysql_error(con), r.out);
          sprintf(query, "select subj,data from msg where id ='%s'", mid);
          if (confres1)
            while (row = mysql_fetch_row(confres1))
            {
              sprintf(left, "<a href=\"http://192.168.1.50/bin%s\"> %s %s<a> <br>", row[0], row[2], row[1]);
              FCGX_PutS(left, r.out);
            }
          mysql_query(con, query);
          MYSQL_RES *confres2 = mysql_store_result(con);
          row = mysql_fetch_row(confres2);
          FCGX_PutS(mysql_error(con), r.out);
          // echofile("edit1.html",r);
          sprintf(left, "</div><div class=\"sh\">"
                        "<form action=\"/addart\" method=\"post\">"
                        "<p>Name: <input type=\"text\" id=\"subj\" name=\"subj\" value=\"%s\">"
                        "<textarea id=\"summernote\" name=\"editordata\">",
                  row[0]);
          FCGX_PutS(left, r.out);
          // Here is data going

          FCGX_PutS(row[1], r.out);
          echofile("edit2.html", r);

          mysql_query(con, "SELECT id, name from topic");
          MYSQL_RES *confres = mysql_store_result(con);
          if (confres)
            while (row = mysql_fetch_row(confres))
            {
              sprintf(left, "<br><input type=\"checkbox\" value="
                            " name=\"topic%s\"> %s",
                      row[0], row[1]);
              FCGX_PutS(left, r.out);
            }
        }
        FCGX_PutS("</form>/div></div>", r.out);
        echofile("footer.tpl", r);
      }
      else
        FCGX_PutS("Database error", r.out);
    }
    else
      FCGX_PutS("Database error", r.out);
  }
}
#define FROM_MAIL "noreply@dev.shushik.kiev.ua"


char *pay_txt(){


  char *payload_text = malloc(1024);
  sprintf(payload_text,"From: <%s>\r\n"
    "To: <%s>\r\n"
    "Subject: validate you email at dev.shushik.kiev.ua\n"
 //   "MIME-Version: 1.0"
 //   "Content-Type: multipart/alternative; boundary=\"outer-boundary\""
    "\r\n"
      "\r\n"
      "Validate your email by going to this http://192.168.1.50/valid=%s \r\n"
    "\r\n"
    ".\r\n", FROM_MAIL, mailto, payload, payload);
    
    return payload_text;
}



void send_payload(char *mailto, char *hash)

{
    struct tm *mt;
    time_t mtt;
    char ftime[30];

    setenv("TZ", "EEST", 1);
    tzset();
    mtt = time(NULL);
    mt = localtime(&mtt);
    strftime(ftime,sizeof(ftime),"%c %Z",mt);
    char *payload_text = malloc(8096);  
  
  sendpay(mailto);
} 
struct upload_status {
  size_t bytes_read;
};
 
static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp)
{
  char *payload_text = pay_txt();
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  const char *data;
  size_t room = size * nmemb;
 
  if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
    return 0;
  }
 
  data = &payload_text[upload_ctx->bytes_read];
 
  if(data) {
    size_t len = strlen(data);
    if(room < len)
      len = room;
    memcpy(ptr, data, len);
    upload_ctx->bytes_read += len;
 
    return len;
  }
 
  return 0;

}
 
int sendpay(char *mailto)
{
  CURL *curl;
  CURLcode res = CURLE_OK;
  struct curl_slist *recipients = NULL;
  struct upload_status upload_ctx = { 0 };
 
  curl = curl_easy_init();
  if(curl) {
    /* Set username and password */
    curl_easy_setopt(curl, CURLOPT_USERNAME, "noreplay@dev.shushik.kiev.ua");
    curl_easy_setopt(curl, CURLOPT_PASSWORD, "azwsdcrf321");
 
    /* This is the URL for your mailserver. Note the use of port 587 here,
     * instead of the normal SMTP port (25). Port 587 is commonly used for
     * secure mail submission (see RFC4403), but you should use whatever
     * matches your server configuration. */
    curl_easy_setopt(curl, CURLOPT_URL, "smtp://soap.shushik.kiev.ua:25");
 
    /* In this example, we will start with a plain text connection, and upgrade
     * to Transport Layer Security (TLS) using the STARTTLS command. Be careful
     * of using CURLUSESSL_TRY here, because if TLS upgrade fails, the transfer
     * will continue anyway - see the security discussion in the libcurl
     * tutorial for more details. */
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
 
    /* If your server does not have a valid certificate, then you can disable
     * part of the Transport Layer Security protection by setting the
     * CURLOPT_SSL_VERIFYPEER and CURLOPT_SSL_VERIFYHOST options to 0 (false). */
         curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
     /* That is, in general, a bad idea. It is still better than sending your
     * authentication details in plain text though.  Instead, you should get
     * the issuer certificate (or the host certificate if the certificate is
     * self-signed) and add it to the set of certificates that are known to
     * libcurl using CURLOPT_CAINFO and/or CURLOPT_CAPATH. See docs/SSLCERTS
     * for more information. */
    // curl_easy_setopt(curl, CURLOPT_CAINFO, "/path/to/certificate.pem");
 
    /* Note that this option is not strictly required, omitting it will result
     * in libcurl sending the MAIL FROM command with empty sender data. All
     * autoresponses should have an empty reverse-path, and should be directed
     * to the address in the reverse-path which triggered them. Otherwise,
     * they could cause an endless loop. See RFC 5321 Section 4.5.5 for more
     * details.
     */
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM_MAIL);
 
    /* Add two recipients, in this particular case they correspond to the
     * To: and Cc: addressees in the header, but they could be any kind of
     * recipient. */
    recipients = curl_slist_append(recipients, mailto);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
 
    /* We are using a callback function to specify the payload (the headers and
     * body of the message). You could just use the CURLOPT_READDATA option to
     * specify a FILE pointer to read from. */
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA,  &upload_ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
 
    /* Since the traffic will be encrypted, it is very useful to turn on debug
     * information within libcurl to see what is happening during the transfer.
     */
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
 
    /* Send the message */
    res = curl_easy_perform(curl);
 
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 
    /* Free the list of recipients */
    curl_slist_free_all(recipients);
 
    /* Always cleanup */
    curl_easy_cleanup(curl);
  }
 
  return (int)res;
}