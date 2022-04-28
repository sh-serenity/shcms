#define __USE_MISC

#include <stdint.h>
#include "fcgi_config.h"
#include "fcgiapp.h"
#include "sodium.h"
#include <assert.h>
#include <ctype.h>
#include <mysql/mysql.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stddef.h>
#include <tidy/tidy.h>
#include <tidy/tidybuffio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <curl/curl.h>

typedef unsigned int            uint;

MYSQL *con;
MYSQL *siriinit();
FCGX_Request r;

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

#define SOCKET_PATH "0.0.0.0:9998"
static int socketId;

int doit()
{
    con = siriinit();
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

      while (FCGX_Accept_r(&r) >= 0)
    {
    //  regex_t ex;
      regex_t exm,ex;
      
      int val, valm;
      
      char *page = (char *)calloc(1024, sizeof(char));
      page = FCGX_GetParam("REQUEST_URI", r.envp);
      if (strcmp(page, "/signin") == 0)
      {
        signin(r);
      }
      if ((strcmp(page, "/art")== 0) || (strcmp(page, "/")== 0) || (strcmp(page, "")== 0))
      {
          art(r);
      }
      val = regcomp(&ex, "[:/addmsg?1-90e:]", 0);
      int res = regexec(&ex, page, 0, NULL, 0);
      if (!res)
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
      if (strcmp(page, "/addart") == 0)
      {
          addart(r);
      }
      valm = regcomp(&exm, "[:/more?1-90e:]", 0);
      int resm = regexec(&exm, page, 0, NULL, 0);
      if (!resm)
      {
          more(r);
      }
          FCGX_Finish_r(&r);
    }
   
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

  mysql_real_connect(con, "localhost", "root", "7ghdbwc3mk", "serenity", 0,
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
 
     while (s < url_len) {
         c = url[s++];
 
         if (c == '%' && s + 2 < url_len) {
            char c2 = url[s++];
            char c3 = url[s++];
             if (isxdigit(c2) && isxdigit(c3)) {
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
 
             } else { /* %zz or something other invalid */
                 dest[d++] = c;
                 dest[d++] = c2;
                 dest[d++] = c3;
             }
         } else if (c == '+') {
             dest[d++] = ' ';
         } else {
             dest[d++] = c;
         }
 
     }
 
     return dest;
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
  FILE *f = fopen(filename, "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET); /* same as rewind(f); */

  char *string = (char *)calloc(fsize+1, sizeof(char));
  fread(string, fsize, 1, f);
  fclose(f);

  string[fsize] = 0;
  FCGX_PutS(string, request.out);
  free(string);
  return 0;
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
    //end = strchr(end+3,'&');
  //int nb = 3;
 char *end;
 char *next;
 next = start;
 char *fuck;
 while(1){
  end = strchr(next, '&');
  //  if (end == strstr(input,"&nbsp;")) end = strchr(strstr(end+1, "&nbsp;")+1,'&');
  //  if (end == strstr(input,"&quot;")) end = strchr(strstr(end+1, "&quot;")+1,'&');
     if ( (end != (fuck = strstr(next,"&nbsp;"))) && (end != (fuck = strstr(next,"&quot;"))) )
     break;
   else 
   {
    if(fuck)
    next = fuck+1;
      if (!end)
    {
      //end = next;
      end = strchr(next, 0);
      break;
    }
   }
  }
  
  int slen = end - start;
  if (mosk == 0)
  {
    char *sdata = (char *)malloc(slen + 1);
    memcpy(sdata, start, slen + 1);
    sdata[slen] = 0;
    return sdata;
  }
  else
  {
    if (slen < mosk)
    {
      char *sdata = (char *)malloc(slen + 1);
      memcpy(sdata, start, slen + 1);
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
  for (i = 0; i < SHA256_DIGEST_LENGTH; i++)
  {
    sprintf(res, "%02x", d[i]);
    strcat(laende, res);
  }
  return laende;
}

int reg(FCGX_Request r)
{
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

    char *len = malloc(16);
    len = FCGX_GetParam("CONTENT_LENGTH", r.envp);
    int ilen = atoi(len);
    char *rawbufp = (char *)calloc(ilen, sizeof(char));
    FCGX_GetStr(rawbufp, ilen, r.in);

    char* bufp = url_decode(rawbufp);
    bufp[ilen] = 0;

    free(rawbufp);
    char* pogin = parse_post(bufp, "name=", 40);
    char* email = parse_post(bufp, "email=", 40);
    char* pass = parse_post(bufp, "pass=", 40);
    char* pmore = parse_post(bufp, "pmore=", 40);

    if (!pogin)
    {
      FCGX_PutS("Something is wrong, go back to <a "
                "href=\"http://serenity-net.org/regform\">Registration</a> and "
                "try again.",
                r.out);
      return 0;
    }
    if (!email)
    {
      FCGX_PutS("Something is wrong, go back to <a "
                "href=\"http://serenity-net.org/regform\">Registration</a> and "
                "try again.",
                r.out);
      return 0;
    }
    if (!pass)
    {
      FCGX_PutS("Something is wrong, go back to <a "
                "href=\"http://serenity-net.org/regform\">Registration</a> and "
                "try again.",
                r.out);
      return 0;
    }
    if (!pmore)
    {
      FCGX_PutS("Something is wrong, go back to <a "
                "href=\"http://serenity-net.org/regform\">Registration</a> and "
                "try again.",
                r.out);
      return 0;
    }
    val = regcomp(&ex, "[A-Za-z0-9;()-_;:.@]", 0);
    char *query = malloc(300);
    sprintf(query, "select * from users where name='%s'", pogin);
    // FCGX_PutS(query,r.out);
    int res;
    mysql_query(con, query);
    MYSQL_RES *confres = mysql_store_result(con);
    int totalrows = mysql_num_rows(confres);
    char s[10];
    if (totalrows == 0)
    {
      res = regexec(&ex, pogin, 0, NULL, 0);
      if (!res)
      {
        FCGX_PutS("name match regexp <br>", r.out);
      }
      else
      {
        FCGX_PutS("name do not match regexp <br>", r.out);
        err = 1;
      }
      res = regexec(&ex, pass, 0, NULL, 0);
      if (!res)
      {
        FCGX_PutS("pass match regex <br>", r.out);
      }
      else
      {
        FCGX_PutS("pass do not match regexp <br>", r.out);
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
      res = regexec(&ex, email, 0, NULL, 0);
      if (!res)
      {
        FCGX_PutS("email match regexp <br>", r.out);
      }
      else
      {
        FCGX_PutS("email do not match regexp <br>", r.out);
        err = 1;
      }
      if (err == 0)
      {
        sprintf(query,
                "insert into users (name, pass, email) "
                "values('%s',md5('%s'),'%s')",
                pogin, pass, email);
        mysql_query(con, query);
        FCGX_PutS(mysql_error(con), r.out);
        FCGX_PutS("User added. Login <a "
                  "href=\"http://meathost.org/"
                  "login.html\">meathost.org\"</a>",
                  r.out);
        free(query);
        free(pogin);
        free(email);
        free(pass);
        free(pmore);
        free(bufp);
        regfree(&ex);
      }
      else
      {
        //		send_headers(r);
        FCGX_PutS("Something is wrong, go back to <a "
                  "href=\"http://serenity-net.org/regform\">Registration</a> "
                  "and try again.",
                  r.out);
        free(query);
        free(pogin);
        free(email);
        free(pass);
        free(pmore);
        free(bufp);
        regfree(&ex);
      }
    }
    else
    {
      //	    send_headers(r);
      FCGX_PutS("This name is taken", r.out);
      free(query);
      free(pogin);
      free(email);
      free(pass);
      free(pmore);
      free(bufp);
      regfree(&ex);
    }
    FCGX_PutS("</div>", r.out);
  }
}


int signin(FCGX_Request r)
{
  char *query = malloc(200);
  // char met[5];
  
  if (!strcmp(FCGX_GetParam("REQUEST_METHOD", r.envp), "POST"))
  {

    char *len = malloc(16);
    len = FCGX_GetParam("CONTENT_LENGTH", r.envp);
    int ilen = atoi(len);
    char *rawbufp = (char *)calloc(ilen, sizeof(char));
  
    FCGX_GetStr(rawbufp, ilen, r.in);
    char* bufp = url_decode(rawbufp);
    bufp[ilen] = 0;
    free(rawbufp);
    //	char *query = malloc(4096);

    char* pogin = parse_post(bufp, "name=", 32);
    char* pass = parse_post(bufp, "pass=", 32);

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
      //  FCGX_PutS("Location: http:/seti/art/",r.out);
      FCGX_PutS(" SameSite=Strict", r.out);
      FCGX_PutS("Location: http://meathost.org/art", r.out);
      FCGX_PutS("\r\n", r.out);
      FCGX_PutS("\r\n", r.out);
      //    FCGX_PutS("Location: http://meathost.org/art",r.out);
      //         FCGX_PutS("<head><meta http-equiv=\"refresh\"
      //         content=\"0;URL=\http://meathost.org/art\"/></head>
      //         ",r.out);
      FCGX_PutS("<script language=\"javascript\" type=\"text/javascript\"> "
                "window.location.href = "
                "\"http://meathost.org/art\";</script>",
                r.out);
    } // else {
      //  FCGX_PutS("Content-type: text/html\r\n", r.out);
    FCGX_PutS("\r\n", r.out);
    FCGX_PutS("\r\n", r.out);
    // echofile("index.html",r.out)

    // FCGX_PutS("Theris not such user.", r.out);
    //  }
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
  
  }
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
  //  free(cookie);
  //  free(kilo);
}



void art(FCGX_Request r)
{
  // send_headers(r);
  //  FCGX_PutS("Content-type: text/html\r\n", r.out);
  //    FCGX_PutS("\r\n", r.out);
  //    FCGX_PutS("\r\n", r.out);

  int uid = getsuid(r);
/*  if (uid) */
    //  FCGX_PutS("Content-type: text/html\r\n", r.out);

    FCGX_PutS("\r\n", r.out);
    FCGX_PutS("\r\n", r.out);
    
    // FCGX_PutS("<head><meta http-equiv=\"refresh\"
    // content=\"0\"></head>",r.out);

    echofile("header.tpl", r);
 //   FCGX_PutS("<div  class="sh"><center><img src=\"pdn.png\"></center></div>", r.out)
    mysql_query(con,"SELECT max(id) from msg;");
    FCGX_PutS(mysql_error(con), r.out);
    MYSQL_RES *confres2 = mysql_store_result(con);
    if(confres2) {
      MYSQL_ROW row2 = mysql_fetch_row(confres2);
      long int max = atoi(row2[0]);
      long int min = max - 50;
      if (min < 0 ) {
          min = 0;
      }

    char *cut = malloc(600);
    char *more = malloc(96);
    char *query1 = malloc(700);
    sprintf(
        query1,
        "WITH RECURSIVE msg_path (id,date, data, parent, lvl, path, name, subj) AS "
        "( SELECT id, date, data, parent, 0 lvl, data as path, (select name "
        "from users"
        " where users.id = msg.owner),subj FROM msg WHERE parent = '%d' "
        " UNION ALL SELECT msg.id, msg.date, msg.data,msg.parent, msgp.lvl + 1,"
        " concat(msgp.path, \">\", msg.data),(select users.name from users "
        "where users.id = msg.owner), msg.subj FROM msg_path AS msgp"
        " JOIN msg AS msg ON msgp.id = msg.parent ) SELECT * FROM msg_path order by path desc, date asc ", 0 , min, max );
        //"order by CASE when parent = 0 then date end desc, path asc, CASE when parent<>0 then path end desc, date asc", 0 , max, min );
    char *left = (char *)malloc(500);
    //FCGX_PutS(query1,r.out);

    mysql_real_query(con, query1,strlen(query1));
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
          int n = 3 * lvl;
          int m = 720 - (20 * lvl);
          FCGX_PutS("<div class=sh>", r.out);
          sprintf(left, "<div class=com style=\"left: %d\%; width: %dpx;\">",
                  n - 3, m);
          FCGX_PutS(left, r.out);
          if(row1[7]) { FCGX_PutS("<h1>",r.out); FCGX_PutS(row1[7],r.out); FCGX_PutS("</h1>",r.out); }
          if (strlen(row1[2]) >= 600) {
            memcpy(cut, row1[2], 600);
            FCGX_PutS(cut,r.out);
            sprintf(more,"<br><a href=\"https://meathost.org/more?%s\">more...</a>",row1[0]);
            FCGX_PutS(more,r.out);
          } else 
             FCGX_PutS(row1[2], r.out);          
          sprintf(left,
                  "<br><p style=\"font-size: 12px;\", color: red;\">Posted by "
                  "%s on %s",
                  row1[6], row1[1]);
          FCGX_PutS(left, r.out);
          // free(left);
          sprintf(left,
              "<div id=\"blogcom\" style=\"width: 550px;\" ><form action=\"/addmsg?%s\" method=\"POST\" id=\"blogco\">"
              "<textarea name=\"test1\">Сomment</textarea>"
              "<input type=\"submit\" value=\"post\"></form></div>",row1[0]);
             // "<script>Query(document).ready(function ($) {"
             // "$('#submit').click(function () { tinyMCE.triggerSave('#LibrarianResponse'); }); });"
            //  "</script>
            //<input "type=\"submit\" value=\"post\"></form></div>",row1[0]);
            // "</div><button id=\"button\">show box</button><script>".
             // "document.getElementById(\"button\").addEventListener(\"click\", function(){document.getElementById(\"blogcom%sm\").style.display = \"\";});"
            //  "document.getElementById(\"blogcom%sm\").addEventListener(\"change\", function(){"
            //  "document.getElementById(\"blogcom%sm\").style.display = \"none\";});</script>",row1[0],row1[0],row1[0],row1[0],row1[0], n - 3,m); 
          FCGX_PutS(left, r.out);

          //style=\"height: 45px; width:550px;\"  name=\"test1\" 

          FCGX_PutS("</div>", r.out);
          FCGX_PutS("</div>", r.out); 
         // FCGX_PutS("</div>", r.out); 
         // FCGX_PutS("</div>", r.out); 
        
          
        }
      }
    }
    
         echofile("footer.tpl", r);
  }
}
  
void more(FCGX_Request r)
{
    char *nu = (char *)malloc(32);
    nu = FCGX_GetParam("REQUEST_URI", r.envp);

    if (strstr(nu,"more")) {

    char *e = strchr(nu, 0);
    char *mid = malloc(16);
    e = strchr(nu, 0);
    int plen = e - (nu + 6);
    memcpy(mid, nu + 6, plen);

    
    mid[plen] = 0;
  int uid = getsuid(r);
/*  if (uid) */
    //  FCGX_PutS("Content-type: text/html\r\n", r.out);

    FCGX_PutS("\r\n", r.out);
    FCGX_PutS("\r\n", r.out);
    
    // FCGX_PutS("<head><meta http-equiv=\"refresh\"
    // content=\"0\"></head>",r.out);

    echofile("header.tpl", r);
//   FCGX_PutS(mid, r.out);
 //   FCGX_PutS("<div  class="sh"><center><img src=\"pdn.png\"></center></div>", r.out)
    mysql_query(con,"SELECT max(id) from msg;");
    FCGX_PutS(mysql_error(con), r.out);
    MYSQL_RES *confres2 = mysql_store_result(con);
    if(confres2) {
      MYSQL_ROW row2 = mysql_fetch_row(confres2);
      long int max = atoi(row2[0]);
      long int min = max - 50;
      if (min < 0 ) {
          min = 0;
      }
    char *cut = malloc(256);
    char *more = malloc(320);
    char *query1 = malloc(700);
    sprintf(
        query1,
        "WITH RECURSIVE msg_path (id,date, data, parent, lvl, path, name, subj) AS "
        "( SELECT id, date, data, parent, 0 lvl, data as path, (select name "
        "from users"
        " where users.id = msg.owner),subj FROM msg WHERE id = %s  "
        " UNION ALL SELECT msg.id, msg.date, msg.data,msg.parent, msgp.lvl + 1,"
        " concat(msgp.path, \">\", msg.data),(select users.name from users "
        "where users.id = msg.owner), msg.subj FROM msg_path AS msgp"
        " JOIN msg AS msg ON msgp.id = msg.parent ) SELECT * FROM msg_path order by date;", mid);
//        "order by case when parent<>'0' then path end asc, case when parent='0' then date end desc LIMIT %d, %d ", 0 , min, max );
    char *left = (char *)malloc(500);
    //FCGX_PutS(query1,r.out);

    mysql_query(con, query1);
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
          int n = 3 * lvl;
          int m = 720 - (20 * lvl);
          FCGX_PutS("<div class=sh>", r.out);
          sprintf(left, "<span class=com style=\"display:inline-block; left: %d\%; width: %dpx;\">",
                  n - 3, m);
          FCGX_PutS(left, r.out);
          if(row1[7]) { FCGX_PutS("<h1>",r.out); FCGX_PutS(row1[7],r.out); FCGX_PutS("</h1>",r.out); }
          FCGX_PutS(row1[2], r.out);
          sprintf(left,
                  "<br><p style=\"font-size: 12px;\", color: red;\">Posted by "
                  "%s on %s",
                  row1[6], row1[1]);
          FCGX_PutS(left, r.out);
          // free(left);
          sprintf(left,
              "<span id=\"blogcom\" style=\"width: 550px;\" ><form action=\"/addmsg?%s\" method=\"POST\" id=\"blogco\">"
              "<textarea id=\"test1\" name=\"test1\" \"></textarea><input "
              "type=\"submit\" value=\"post\"></form></span>",row1[0]);
            // "</div><button id=\"button\">show box</button><script>".
             // "document.getElementById(\"button\").addEventListener(\"click\", function(){document.getElementById(\"blogcom%sm\").style.display = \"\";});"
            //  "document.getElementById(\"blogcom%sm\").addEventListener(\"change\", function(){"
            //  "document.getElementById(\"blogcom%sm\").style.display = \"none\";});</script>",row1[0],row1[0],row1[0],row1[0],row1[0], n - 3,m); 
          FCGX_PutS(left, r.out);

          //style=\"height: 45px; width:550px;\"  name=\"test1\" 

          FCGX_PutS("</span>", r.out);
          FCGX_PutS("</div>", r.out); 
         // FCGX_PutS("</div>", r.out); 
         // FCGX_PutS("</div>", r.out); 
        
          
        }
      }
    }
    
         echofile("footer.tpl", r);
  }
}
}

//} 

char *leprozory(const char *input)
{
  TidyBuffer output = {0};
  TidyBuffer errbuf = {0};
  int rc = -1;
  Bool ok;
  TidyDoc tdoc = tidyCreate();                     // Initialize "document


   ok = tidyOptSetBool( tdoc, TidyXhtmlOut, yes );  // Convert to XHTML
  if ( ok )
    rc = tidySetErrorBuffer( tdoc, &errbuf );      // Capture diagnostics
  if ( rc >= 0 )
    rc = tidyParseString( tdoc, input );           // Parse the input
  if ( rc >= 0 )  
    rc = tidyOptSetBool(tdoc, TidyMark, no);
  if ( rc >= 0 )
    rc = tidyOptSetInt(tdoc, TidyWrapLen, 0);
  if ( rc >= 0 )
    rc = tidyOptSetInt(tdoc, TidyBodyOnly, 1);
  if ( rc >= 0 )
    tidySetCharEncoding(tdoc, "utf8");
  if ( rc >= 0 )
     rc = tidyOptSetInt(tdoc, TidyDoctypeMode, TidyDoctypeOmit);
  if ( rc >= 0 )
    rc = tidyCleanAndRepair( tdoc );               // Tidy it up!
  if ( rc >= 0 )
    rc = tidyRunDiagnostics( tdoc );               // Kvetch
  if ( rc > 1 )                                    // If error, force output.
    rc = ( tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1 );
  if ( ok )
    rc = tidySetErrorBuffer( tdoc, &errbuf );      // Capture diagnostics
  
    char *cleansed_buffer_ = (char *)malloc(1);
    rc =  tidySaveBuffer(tdoc, &output);
    cleansed_buffer_ = (char *)malloc(output.size+1);                  
    memcpy ((char *)cleansed_buffer_, (char *) output.bp,output.size);

//   rc = tidySaveString(tdoc, cleansed_buffer_, &size ); 
/*    printf( "\nDiagnostics:\n\n%s", errbuf.bp );

    printf( "\nAnd here is the result:\n\n%s", output); */

    return cleansed_buffer_;
  tidyBufFree( &errbuf );
  tidyRelease( tdoc );
}


void addmsg(FCGX_Request r)
  {
    //  send_headers(r);
    char *nu = (char *)malloc(32);
    nu = FCGX_GetParam("REQUEST_URI", r.envp);
    char *n = strstr(nu, "/addmsg");
    if (n)
    {
      //  echofile("header.tpl", r);
      int uid = getsuid(r);
      if(uid)
      {

        char *eight = (char *)malloc(8);
        sprintf(eight, "%d", uid);
        char *m = malloc(5);
        m = FCGX_GetParam("REQUEST_METHOD", r.envp);
        //      FCGX_PutS(method,r.out);
          if (!strcmp(m, "POST"))
        {
          //	    FCGX_PutS(method,r.out);
          char *len = malloc(16);
          len = FCGX_GetParam("CONTENT_LENGTH", r.envp);
          int ilen = atoi(len);
          if (ilen > 0) {
            char *rawbufp = malloc(ilen);
            FCGX_GetStr(rawbufp, ilen, r.in);
            char* bufp = url_decode(rawbufp);
            bufp[ilen] = 0;
            
            free(rawbufp);

            char *e = strchr(nu, 0);
            char *parent = malloc(8);
            e = strchr(nu, 0);
            int plen = e - (nu + 8);
            memcpy(parent, nu + 8, plen);
            parent[plen] = 0;

            char* s2 = parse_post(bufp, "test1=", 0);
            char* s3 = leprozory(s2);
            int query_size =
              strlen(s2) +
              strlen("INSERT INTO msg (owner, data, parent) VALUES "
                     "('%s', '%s', '%d')") +
              1024;
            char *query = malloc(query_size);
            sprintf(
              query,
              "INSERT INTO msg (owner, data, parent) VALUES ('%u', '%s', '%s')",
              uid, s2, parent);
             FCGX_PutS("Content-type: text/html\r\n\r\n",r.out); 
            FILE *fe = fopen("err","a");
            fputs(query,fe);
            mysql_query(con, query);
           FCGX_PutS(mysql_error(con), r.out);
            fputs(mysql_error(con),fe);
            fclose(fe);

           FCGX_PutS(mysql_error(con), r.out);
          //  free(parent);
            free(s3);
            free(s2);
            free(query);
            free(bufp);
          } // there in post zero.
          FCGX_PutS("Location: http://meathost.org/art",r.out);
               FCGX_PutS("<script language=\"javascript\" type=\"text/javascript\"> "
                "window.location.href = "
                "\"http://meathost.org/art\";</script>",
                r.out);
          
        }
      } else FCGX_PutS("You are not registerd", r.out);

    }

}

void addart(FCGX_Request r)
{
    //  send_headers(r);
    char *nu = (char *)malloc(32);
      //  echofile("header.tpl", r);
      int uid = getsuid(r);
      if(uid)
      {

        char *eight = (char *)malloc(8);
        sprintf(eight, "%d", uid);
        char *m = malloc(5);
        m = FCGX_GetParam("REQUEST_METHOD", r.envp);
        //      FCGX_PutS(method,r.out);
        if (!strcmp(m, "POST"))
        {
          //	    FCGX_PutS(method,r.out);
          char *len = malloc(16);
          len = FCGX_GetParam("CONTENT_LENGTH", r.envp);
          uint ilen = atoi(len);
          if (ilen > 0) {
            char *rawbufp = malloc(ilen);
            FCGX_GetStr(rawbufp, ilen, r.in);
            char *bufp = url_decode(rawbufp);
            bufp[ilen] = 0;

            char* s4 = parse_post(bufp,"subj=", 0);
            s4[strlen(s4)-1] = 0;
            char *s5 = leprozory(s4); 
            char* s2 = parse_post(bufp, "editordata=", 0);
            char* s3 = leprozory(s2);
            int query_size =
              ilen +
              strlen("INSERT INTO msg (owner, data, parent,subj) VALUES "
                     "('%s', '%s', '%d', '%s')") +
              100;
            long int size = ilen*2 + 1;
            char *query = malloc(size);
            char *end = malloc(size);
            mysql_real_escape_string(con,end,s3,strlen(s3));
            char *endsubj = malloc(strlen(s5)*2+1);
            mysql_real_escape_string(con,endsubj,s5,strlen(s5));

           
            sprintf(
              query,
              "INSERT INTO msg (owner, data, parent, subj) VALUES ('%s', '%s', '%d', '%s');",
              eight, end, 0, endsubj);
            FCGX_PutS("Content-type: text/html\r\n\r\n",r.out); 
            FILE *fe = fopen("err","a");
            fputs(query,fe);
            mysql_query(con, query);
           FCGX_PutS(mysql_error(con), r.out);
            fputs(mysql_error(con),fe);
            fclose(fe);
    //        free(parent);
         //   free(s3);
            free(s2);
            free(s4);
            free(s3);
            free(s5);
          //  free(s5);
            free(query);
            free(bufp);
          } // there in post zero.
          FCGX_PutS("Location: http://meathost.org/art",r.out);
               FCGX_PutS("<script language=\"javascript\" type=\"text/javascript\"> "
                "window.location.href = "
                "\"http://meathost.org/art\";</script>",
                r.out);
          
        }
      } else FCGX_PutS("You are not registerd", r.out);

}



#define MAXLINE 38400
#define FILESIZE 37632
#define MAXSUB  38016

char boundary[40] = "---------------------------";

/*ssize_t process_http(int sockfd, char *host, char *page, char *boundary, char *poststr)
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


void image(FCGX_Request r) {
  //send_headers(r);
  //FCGX_PutS("Accept-Post: */*\r\n",r.out);
 // FCGX_PutS("\r\n\r\n",r.out);
  
  int uid = getsuid(r);
  if (uid) 
  if (!strcmp(FCGX_GetParam("REQUEST_METHOD", r.envp), "POST"))
  {
    char *bufp =  malloc(4096);
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
    char *len = malloc(16);
    len = FCGX_GetParam("CONTENT_LENGTH", r.envp);
    int ilen = atoi(len);
             
   char *found;
   char *bnum = malloc(40);
   char *filename = malloc(64);
   char *contype = malloc(32);
   int i = 0;
   const char *ch;
   while (bufp[i] = FCGX_GetChar(r.in)) {
      //found = strstr(bufp,"--\r\n");
     // ch = (char *)&bufp[i];
      if (strcmp((char *)&bufp[i], "-") != 0) 
      break;
      i++;
      if (i > 100) break;
   }
   bufp[0] = bufp[i];
   i = 1; 
   while (bufp[i] = FCGX_GetChar(r.in)) {
      if (found = strstr(bufp,"\r\n"))
      break;
      i++;
      if (i > 40) break;
   }
   bufp[i-1] = 0;
   bufp[i] = 0; 
   memcpy(bnum,bufp,strlen(bufp));
   i = 0;
   found = "";
   while (bufp[i] = FCGX_GetChar(r.in)) {
      if (found = strstr(bufp,"Content-Disposition: form-data; name=\"image\"; filename=\""))
      break;
      i++;
      if (i > 256) break;
   }
  //bufp[i] = FCGX_GetChar(r.in);   
   i = 0;
   while (filename[i] = FCGX_GetChar(r.in)) {
      if (strstr(filename,"\"\r\n")) 
      break;
      i++;
      if (i > 64) break;
   }
   filename[i-2] = 0;
   i = 0;
   while (bufp[i] = FCGX_GetChar(r.in)) {
      if (strstr(bufp,"Content-Type: ")) 
      break;
      i++;
      if (i > 32) break;
   }
   i = 0;
   while (contype[i] = FCGX_GetChar(r.in)) {
      if (strstr(contype,"\r\n\r\n")) 
      break;
      i++;
      if (i > 32) break;  
   }
   contype[i-3] = 0;
/*   }    while (buf[i] = FCGX_GetChar(r.in)) {
      //found = strstr(bufp,"--\r\n");
     // ch = (char *)&bufp[i];
      if (strcmp((char *)&buf[i], "-") != 0) 
      break;
      i++;
   } */
   ch = (char *)"\020";
     FCGX_PutS("Accept-Post: application/octet-stream\r\n\r\n",r.out);
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
   //while (i < ilen) {
    while (1 == 1) {
    while (i < ilen) {
      buf[i] = FCGX_GetChar(r.in);
      i++;
    }
      if (i >= ilen) break;
  }
   char *fname = malloc(96);
   sprintf(fname,"upload/%s",filename);
   FILE *f = fopen(fname,"w");
  // int m = strlen(buf);
   int n = fwrite(buf,1, i,f);  
   fclose(f);
//   FCGX_PutS(len,r.out);
   char *st = malloc(100);
 //  FCGX_PutS("Content-type: text/html\r\n\r\n",r.out); 
   sprintf(st,"meathost.org/%s\r\n",fname);
   printf(st);
   FCGX_PutS(st,r.out);
 //  FCGX_PutS(filename,r.out); 
 //  FCGX_PutS("}",r.out);
  
  // FCGX_PutS(buf,r.out);
   

  // bufp[i] = 0; 
  // filename = bufp;
   
     


   //0x7ffff6a6a010 '-' <repeats 29 times>, "\
//Content-Disposition: form-data; name=\"image\"; filename=\"


//235103533627700036105989979\r\nContent-Disposition: form-data; name=\"image\"; filename=\"Iam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n\377\330\377", <incomplete sequence \340>
  
  }
 }




void add(FCGX_Request r) {
  send_headers(r);
  FCGX_PutS("Content-type: text/html\r\n", r.out);
  FCGX_PutS("\r\n", r.out);
  echofile("new.html",r);

}

void bin(FCGX_Request r) {
  // char *str = (char *)malloc(110);
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
    sprintf(query, "select id, name from users where kilo='%s' and admin='1'", kilo);
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
        FCGX_PutS("Content-type: text/html\r\n", r.out);
        FCGX_PutS("\r\n", r.out);
        FCGX_PutS("\r\n", r.out);
          FCGX_PutS("<html><head><title>Bin</title></head><body>Welcome,",r.out);
          FCGX_PutS(row[0],r.out);
          FCGX_PutS("</body></html>",r.out);
 //         echofile("adv.tpl",r);
      } else FCGX_PutS("Database error", r.out);
    } else
    FCGX_PutS("Database error", r.out);
    //     free(query);
    //     mysql_free_result(confres);
  } // else {
  // FCGX_PutS("Content-type: text/html\r\n", r.out);
  // FCGX_PutS("\r\n", r.out);
  //  FCGX_PutS("No cookies.", r.out);
  //  }
  //  free(cookie);
  //  free(kilo);

}
