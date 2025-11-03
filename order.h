#ifndef ORDER_H
#define ORDER_H
#include <string>

struct Order {
    int userIdx;
    int trainIdx;
    int fromIdx;
    int toIdx;
    int num;
    int price;
    int boardingDate; // day index when leaving fromIdx
    int baseStartDay; // start date for station 1
    int status; // 0=pending,1=success,2=refunded
    int next; // for per-user linked list (newest first)
};

void orders_init(int maxUsers);
int order_create(int userIdx, int trainIdx, int fromIdx, int toIdx, int num, int price, int boardingDate, int baseStartDay, int status);
int user_order_head(int userIdx);
const Order &orders_get(int idx);
Order &orders_get_mut(int idx);

#endif

