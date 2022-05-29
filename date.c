#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main (int argc, char *argv[])
{
    struct tm *mt;
    time_t mtt;
    char ftime[30];

    setenv("TZ", "EEST", 1);
    tzset();
    mtt = time(NULL);
    mt = localtime(&mtt);
    strftime(ftime,sizeof(ftime),"%c %Z",mt);

    printf("%s\n", ftime);
}