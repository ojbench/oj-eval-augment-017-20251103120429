#include "train.h"
#include "util.h"
#include <cstring>

static const int MAX_TRAINS = 3000;
static const int HASH_SIZE = 200003;

static Train trains[MAX_TRAINS];
static int tr_cnt = 0;
static int head[HASH_SIZE];
static int nxt[MAX_TRAINS];

static unsigned long long hstr(const std::string &s) {
    unsigned long long h=1469598103934665603ull;
    for (char c: s) { h ^= (unsigned char)c; h *= 1099511628211ull; }
    return h;
}

void trains_init(){ tr_cnt=0; for(int i=0;i<HASH_SIZE;++i) head[i]=-1; }
int trains_count(){ return tr_cnt; }

int trains_find(const std::string &id){
    unsigned long long hv = hstr(id)%HASH_SIZE;
    for(int i=head[hv]; i!=-1; i=nxt[i]) if(trains[i].valid && trains[i].id==id) return i;
    return -1;
}

int trains_add(const Train &src){
    if (tr_cnt>=MAX_TRAINS) return -1;
    if (trains_find(src.id)!=-1) return -1;
    int idx = tr_cnt++;
    trains[idx] = src;
    trains[idx].released=false; trains[idx].valid=true;
    trains[idx].segments = trains[idx].stationNum-1;
    trains[idx].dayCount = trains[idx].saleEnd - trains[idx].saleStart + 1;
    trains[idx].seats=nullptr;
    trains[idx].pendHead=nullptr;
    trains[idx].pendTail=nullptr;
    precompute_times(trains[idx]);
    unsigned long long hv = hstr(src.id)%HASH_SIZE;
    nxt[idx]=head[hv]; head[hv]=idx;
    return idx;
}

bool trains_release(const std::string &id){
    int idx = trains_find(id);
    if (idx==-1) return false;
    Train &t = trains[idx];
    if (t.released) return false;
    int days = t.dayCount; int segs = t.segments;
    long long total = (long long)days * segs;
    t.seats = new int[total];
    for (long long i=0;i<total;++i) t.seats[i]=t.seatNum;
    t.pendHead = new int[days];
    t.pendTail = new int[days];
    for(int i=0;i<days;++i){ t.pendHead[i]=-1; t.pendTail[i]=-1; }
    t.released = true;
    return true;
}

bool trains_delete(const std::string &id){
    int idx = trains_find(id);
    if (idx==-1) return false;
    Train &t=trains[idx];
    if (t.released) return false;
    t.valid=false;
    return true;
}

Train &trains_get(int idx){ return trains[idx]; }

int station_index(const Train &t, const std::string &name){
    for(int i=1;i<=t.stationNum;++i) if (t.stations[i]==name) return i;
    return -1;
}

int seats_min(const Train &t, int baseStartDay, int lSeg, int rSeg){
    int dayIdx = baseStartDay - t.saleStart;
    if (dayIdx<0 || dayIdx>=t.dayCount) return 0;
    int mn = 1e9;
    int base = dayIdx * t.segments;
    for (int s=lSeg; s<=rSeg; ++s) {
        int v = t.seats[base + (s-1)];
        if (v<mn) mn=v;
    }
    return mn;
}

void seats_add(const Train &t, int baseStartDay, int lSeg, int rSeg, int delta){
    int dayIdx = baseStartDay - t.saleStart;
    if (dayIdx<0 || dayIdx>=t.dayCount) return;
    int base = dayIdx * t.segments;
    for (int s=lSeg; s<=rSeg; ++s) {
        t.seats[base + (s-1)] += delta;
    }
}

void precompute_times(Train &t){
    // 1-indexed stations
    for (int i=1;i<=t.stationNum;++i){ t.arr[i]=-1; t.dep[i]=-1; }
    t.dep[1] = t.startTime;
    for (int i=2;i<=t.stationNum;++i){
        t.arr[i] = t.dep[i-1] + t.travel[i-1];
        if (i < t.stationNum) t.dep[i] = t.arr[i] + t.stopover[i-1];
    }
}

bool compute_base_start_day_for_boarding(const Train &t, int k, int boardingDate, int &baseStartDay){
    if (k<1 || k>t.stationNum) return false;
    int depk = t.dep[k];
    if (depk<0) return false;
    int depkDayOff = depk / 1440;
    baseStartDay = boardingDate - depkDayOff;
    if (baseStartDay < t.saleStart || baseStartDay > t.saleEnd) return false;
    return true;
}

