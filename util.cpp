#include "util.h"
#include <cstdio>

static int monthDays(int month) {
    // 2021: June(6)=30, July(7)=31, August(8)=31
    if (month == 6) return 30;
    if (month == 7) return 31;
    return 31;
}

int dayIndexFromMMDD(const std::string &mmdd) {
    // format MM-DD
    int mm = (mmdd[0]-'0')*10 + (mmdd[1]-'0');
    int dd = (mmdd[3]-'0')*10 + (mmdd[4]-'0');
    int idx = 0;
    for (int m=6; m<mm; ++m) idx += monthDays(m);
    idx += (dd - 1);
    return idx;
}

std::string mmddFromIndex(int idx) {
    int m = 6;
    int d = idx;
    while (true) {
        int md = monthDays(m);
        if (d < md) break;
        d -= md;
        ++m;
    }
    char buf[6];
    std::snprintf(buf, sizeof(buf), "%02d-%02d", m, d+1);
    return std::string(buf);
}

int parseHHMM(const std::string &hhmm) {
    int hh = (hhmm[0]-'0')*10 + (hhmm[1]-'0');
    int mm = (hhmm[3]-'0')*10 + (hhmm[4]-'0');
    return hh*60 + mm;
}

std::string formatMMDD_HHMM(int dayIndex, int minutesOfDay) {
    while (minutesOfDay >= 1440) { minutesOfDay -= 1440; ++dayIndex; }
    while (minutesOfDay < 0) { minutesOfDay += 1440; --dayIndex; }
    char buf[17];
    std::string md = mmddFromIndex(dayIndex);
    int hh = minutesOfDay/60; int mi = minutesOfDay%60;
    std::snprintf(buf, sizeof(buf), "%s %02d:%02d", md.c_str(), hh, mi);
    return std::string(buf);
}

int toInt(const std::string &s) {
    int x=0; int i=0; int sign=1;
    if (!s.empty() && s[0]=='-') { sign=-1; i=1; }
    for (; i<(int)s.size(); ++i) x = x*10 + (s[i]-'0');
    return x*sign;
}

int split(const std::string &s, char delim, std::string out[], int maxParts) {
    int n=0; std::string cur;
    for (size_t i=0;i<s.size();++i) {
        char c = s[i];
        if (c==delim) {
            if (n<maxParts) out[n]=cur;
            ++n; cur.clear();
        } else cur.push_back(c);
    }
    if (n<maxParts) out[n]=cur; ++n;
    return n>maxParts?maxParts:n;
}

int cmpStr(const std::string &a, const std::string &b) {
    if (a==b) return 0;
    return a < b ? -1 : 1;
}

