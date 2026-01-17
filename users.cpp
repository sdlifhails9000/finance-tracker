#include <iostream>
#include <sqlite3.h>

#include "users.h"
#include "utils.h"

using namespace std;

/*
 * This is to register user onto the USER TABLE and create his FINANCE TABLE USING ABOVE FUNCTIONS
 */
void account_setup(sqlite3* mdb) {
    const char *insert_stmt_str =
        "INSERT INTO users (user_name, first_name, last_name, age, password) VALUES"
        "(?, ?, ?, ?, ?);"
    ;
    sqlite3_stmt *stmt = nullptr;
    string username;
    string first_name, last_name;
    int age;
    string password;

    sqlite3_prepare_v2(mdb, insert_stmt_str, -1, &stmt, nullptr);

    do {
        first_name =  input<string>("Enter your first name: ");
        if (first_name.size() > 50) {
            cout << "First name too big (> 50)!" << endl;
        }
    } while (first_name.size() > 50);

    do {
        last_name = input<string>("Enter your last name: ");
        if (last_name.size() > 50) {
            cout << "Last name too big (> 50)!" << endl;
        }
    } while (last_name.size() > 50);

    age = input<int>("Enter your age: ");

    do {
        password = input<string>("Enter your password: ");
        if (password.size() > 20) {
            cout << "Password too big (> 20)!" << endl;
        }
    } while (password.size() > 20);

    unsigned char hash[32];
    unsigned int hash_len;

    calc_hash(password, hash, &hash_len);

    while (true) {
        do {
            username = input<string>("Enter your username: ");
            if (username.size() > 50) {
                cout << "Username too big (> 50)!" << endl;
            }
        } while (username.size() > 50);

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, first_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, last_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, age);
        sqlite3_bind_text(stmt, 5, (const char *)hash, 32, SQLITE_STATIC);

        int rc = sqlite3_step(stmt);

        if (rc == SQLITE_CONSTRAINT) {
            int extended_err = sqlite3_extended_errcode(mdb);
            if (extended_err == SQLITE_CONSTRAINT_UNIQUE) {
                cout << "Someone with that username already exists." << endl;
                sqlite3_reset(stmt);
                sqlite3_clear_bindings(stmt);
                continue;
            }
        } else if (rc != SQLITE_DONE) {
            cout << "ERROR: Something bad happened yawr" << endl;

            cout << rc << endl;
            cout << sqlite3_errmsg(mdb) << endl;
            sqlite3_finalize(stmt);
            exit(EXIT_FAILURE);
        }
        break;
    }

    sqlite3_finalize(stmt);
}

/*
 * This will only pull data from the USER TABLE and FINANCE TABLE and work with FUNCTIONS BELOW
 */
User login_verify(sqlite3* mdb) {
    string username;
    User u;     //Our user

    string pass;
    unsigned char hash[32];
    unsigned int hash_len = 0;

    sqlite3_stmt* stmt;
    const char *check = "SELECT * FROM users WHERE user_name = ? AND password = ?;";    

    sqlite3_prepare_v2(mdb, check, -1, &stmt, nullptr);

    while (true) {
        username = input<string>("Enter your username: ");
        pass = input<string>("Enter your password: ");
       
        calc_hash(pass, hash, &hash_len);
       
        sqlite3_clear_bindings(stmt);
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, (const char *)hash, 32, SQLITE_STATIC);

        int rc = sqlite3_step(stmt);

        if(rc != SQLITE_ROW){
            cout << "Invalid username or password please try again!" << endl;
            sqlite3_reset(stmt);
            continue;
        }
        break;      //To break out in-case user enters correct username and pw
    }

    u.username = username;
    u.id = sqlite3_column_int(stmt,0);                      //Stores data in struct to return and be passed to other functions
    u.firstname = (const char*)sqlite3_column_text(stmt,2);
    u.lastname = (const char*)sqlite3_column_text(stmt,3);
    u.age = sqlite3_column_int(stmt,4);

    sqlite3_finalize(stmt);
    return u;       //PREPARED METH
}
