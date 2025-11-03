#ifndef USER_H
#define USER_H
#include <string>

struct User {
    std::string username;
    std::string password;
    std::string name;
    std::string mail;
    int privilege;
    bool loggedIn;
};

void users_init();
int users_count();
int users_find(const std::string &username); // returns index or -1
int users_add(const std::string &username, const std::string &password,
              const std::string &name, const std::string &mail, int privilege);
bool users_login(const std::string &username, const std::string &password);
bool users_logout(const std::string &username);
User &users_get(int idx);

#endif

