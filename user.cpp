#include "user.h"
#include <cstring>

static const int MAX_USERS = 50000;
static const int HASH_SIZE = 200003; // prime

static User users[MAX_USERS];
static int user_cnt = 0;

static int head[HASH_SIZE];
static int nxt[MAX_USERS];

static unsigned long long hstr(const std::string &s) {
    unsigned long long h=1469598103934665603ull; // FNV-1a
    for (char c: s) { h ^= (unsigned char)c; h *= 1099511628211ull; }
    return h;
}

void users_init() {
    user_cnt = 0;
    for (int i=0;i<HASH_SIZE;++i) head[i]=-1;
}

int users_count(){ return user_cnt; }

int users_find(const std::string &username) {
    unsigned long long hv = hstr(username)%HASH_SIZE;
    for (int i=head[hv]; i!=-1; i=nxt[i])
        if (users[i].username==username) return i;
    return -1;
}

int users_add(const std::string &username, const std::string &password,
              const std::string &name, const std::string &mail, int privilege) {
    if (user_cnt>=MAX_USERS) return -1;
    if (users_find(username)!=-1) return -1;
    int idx = user_cnt++;
    users[idx].username = username;
    users[idx].password = password;
    users[idx].name = name;
    users[idx].mail = mail;
    users[idx].privilege = privilege;
    users[idx].loggedIn = false;
    unsigned long long hv = hstr(username)%HASH_SIZE;
    nxt[idx] = head[hv]; head[hv] = idx;
    return idx;
}

bool users_login(const std::string &username, const std::string &password) {
    int idx = users_find(username);
    if (idx==-1) return false;
    if (users[idx].loggedIn) return false;
    if (users[idx].password!=password) return false;
    users[idx].loggedIn = true;
    return true;
}

bool users_logout(const std::string &username) {
    int idx = users_find(username);
    if (idx==-1) return false;
    if (!users[idx].loggedIn) return false;
    users[idx].loggedIn = false;
    return true;
}

User &users_get(int idx) { return users[idx]; }

