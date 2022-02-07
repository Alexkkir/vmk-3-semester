#include <stdio.h>
#include <time.h> // struct tm, mktime
#include <string.h> // memset

enum { ZERO_YEAR = 1900, DAYS_IN_YEAR = 365, DAYS_IN_WEEK = 7, DEZINF_WEEK_1 = 2, DEZINF_WEEK_2 = 4 };

void preprare_struct(struct tm *p_time, int year, int yday) {
    memset(p_time, 0, sizeof(*p_time));
    p_time->tm_year = year - ZERO_YEAR;
    p_time->tm_mon = 0;
    p_time->tm_mday = yday;
    p_time->tm_hour = 12;
    p_time->tm_isdst = -1;
}

int main() {
    int year;
    scanf("%d", &year);

    struct tm time;
    int is_leap = year % 4 == 0 ? 1 : 0; // is leap?
    int thursday_count = -1;
    for (int yday = 1; yday <= DAYS_IN_YEAR + is_leap; yday++) { // if leap then add 1
        preprare_struct(&time, year, yday);
        mktime(&time);
        if (time.tm_mday == 1) {
            thursday_count = 0;
        }
        int mday = time.tm_mday; // in [1, 31]
        if (time.tm_wday == 4) {
            thursday_count++;
        }
        if ((thursday_count == DEZINF_WEEK_1 || thursday_count == DEZINF_WEEK_2) &&
            time.tm_wday == 4 &&
            mday % 3 != 0) {
            printf("%d %d\n", time.tm_mon + 1, time.tm_mday);
        }
    }

    return 0;
}
