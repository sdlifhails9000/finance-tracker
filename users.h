#ifndef USERS_H_
#define USERS_H_

#include <string>
#include <sqlite3.h>

using namespace std;

/*
 * This will be used to contain our user information on successful login esp after login_verify() succeeds
 */

struct User {
    int id;
    string username;
    string firstname;
    string lastname;
    int age;
};

/*
 * This is to register user onto the USER TABLE and create his FINANCE TABLE USING ABOVE FUNCTIONS
 */
void account_setup(sqlite3* mdb);

/*
 * This will only pull data from the USER TABLE and FINANCE TABLE and work with FUNCTIONS BELOW
 */
User login_verify(sqlite3* mdb);

#endif
