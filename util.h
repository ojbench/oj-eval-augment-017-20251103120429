#ifndef UTIL_H
#define UTIL_H
#include <string>

// Date range is 2021-06-01 to 2021-08-31 inclusive.
// We map 06-01 -> 0, ..., 08-31 -> 91 (92 days total)

int dayIndexFromMMDD(const std::string &mmdd);
std::string mmddFromIndex(int idx);
int parseHHMM(const std::string &hhmm);
std::string formatMMDD_HHMM(int dayIndex, int minutesOfDay);

int toInt(const std::string &s);

// split by a single char delimiter into at most maxParts tokens (<=100)
int split(const std::string &s, char delim, std::string out[], int maxParts);

// simple string compare: return <0 if a<b, 0 if equal, >0 if a>b
int cmpStr(const std::string &a, const std::string &b);

#endif

