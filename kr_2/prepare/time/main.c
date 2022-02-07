#include <stdio.h>
#include <time.h>

int
main()
{
    time_t t = time(NULL);
    struct tm *items = localtime(&t);
    printf("sec: %d\n"
           "min: %d\n"
           "hour %d\n"
           "mday %d\n"
           "mon %d\n"
           "year %d\n"
           "wday %d\n"
           "yday %d\n"
           "isdst %d\n",
           items->tm_sec,
           items->tm_min,
           items->tm_hour,
           items->tm_mday,
           items->tm_mon,
           items->tm_year,
           items->tm_wday,
           items->tm_yday,
           items->tm_isdst);

    printf("========\n");
    items->tm_isdst = 1;
    t = mktime(items);
    items = localtime(&t);
    printf("sec: %d\n"
           "min: %d\n"
           "hour %d\n"
           "mday %d\n"
           "mon %d\n"
           "year %d\n"
           "wday %d\n"
           "yday %d\n"
           "isdst %d\n",
           items->tm_sec,
           items->tm_min,
           items->tm_hour,
           items->tm_mday,
           items->tm_mon,
           items->tm_year,
           items->tm_wday,
           items->tm_yday,
           items->tm_isdst);

    return 0;
}
