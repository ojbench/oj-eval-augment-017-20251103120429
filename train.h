#ifndef TRAIN_H
#define TRAIN_H
#include <string>

struct Train {
    std::string id;
    int stationNum;
    int seatNum;
    std::string stations[105];
    int prices[105]; // size stationNum-1
    int travel[105]; // size stationNum-1
    int stopover[105]; // size stationNum-2 (indices 2..stationNum-1)
    int startTime; // minutes of day
    char type;
    int saleStart; // day index
    int saleEnd;   // day index
    bool released;
    bool valid;
    int dep[105]; // minutes from base day 0
    int arr[105]; // minutes from base day 0 (arr[1] invalid)
    int dayCount; // saleEnd - saleStart + 1
    int segments; // stationNum-1
    int *seats;   // size dayCount*segments, only after release
};

void trains_init();
int trains_find(const std::string &id); // returns index or -1 (only if valid)
int trains_add(const Train &t); // returns idx or -1 on conflict
bool trains_release(const std::string &id);
bool trains_delete(const std::string &id);
Train &trains_get(int idx);
int trains_count();

int station_index(const Train &t, const std::string &name); // 1..stationNum or -1

// seat helpers (require released)
int seats_min(const Train &t, int baseStartDay, int lSeg, int rSeg); // seg indices [lSeg, rSeg]
void seats_add(const Train &t, int baseStartDay, int lSeg, int rSeg, int delta); // delta can be -n to buy, +n to refund

// time helpers
void precompute_times(Train &t);
bool compute_base_start_day_for_boarding(const Train &t, int boardingStationIdx, int boardingDate, int &baseStartDay);

#endif

