#include "order.h"

static const int MAX_ORDERS = 200000;
static Order orders[MAX_ORDERS];
static int ord_cnt=0;
static int *user_head=nullptr;
static int max_users_cached=0;

void orders_init(int maxUsers){
    if (user_head) delete [] user_head;
    max_users_cached = maxUsers;
    user_head = new int[maxUsers];
    for (int i=0;i<maxUsers;++i) user_head[i]=-1;
    ord_cnt=0;
}

int order_create(int userIdx, int trainIdx, int fromIdx, int toIdx, int num, int price, int boardingDate, int baseStartDay, int status){
    if (ord_cnt>=MAX_ORDERS) return -1;
    int idx = ord_cnt++;
    orders[idx].userIdx=userIdx;
    orders[idx].trainIdx=trainIdx;
    orders[idx].fromIdx=fromIdx;
    orders[idx].toIdx=toIdx;
    orders[idx].num=num;
    orders[idx].price=price;
    orders[idx].boardingDate=boardingDate;
    orders[idx].baseStartDay=baseStartDay;
    orders[idx].status=status;
    orders[idx].next = user_head[userIdx];
    user_head[userIdx] = idx;
    return idx;
}

int user_order_head(int userIdx){ return user_head[userIdx]; }

const Order &orders_get(int idx){ return orders[idx]; }
Order &orders_get_mut(int idx){ return orders[idx]; }

