#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <sqlite3.h>

#include "utils.h"
#include "ui.h"

#include "moneypools.h"

using namespace std;

/*
 * This functions is used when user logins successfully and wants to view his account types i.e bank, card, easypaisa, etc
 */
void view_moneypool(sqlite3* mdb, int user_id) {
    sqlite3_stmt* stmt;
    const char* select_stmt = "SELECT id, pool_name, initial_balance FROM moneypools WHERE user_id = ?;";
    sqlite3_prepare_v2(mdb, select_stmt, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, user_id);

    cout << endl;

    struct bruh {int id; string name;};

    int count = 1;
    vector<bruh> choices;
    if (sqlite3_step(stmt) != SQLITE_ROW && count == 1){
        cout << "No moneypools available" << endl;
        return;
    } 

    // Goes through each row and outputs it with a number so user can make choice
    do {
        string name = (const char*)sqlite3_column_text(stmt,1);
        double balance = sqlite3_column_double(stmt,2);
        
        choices.push_back({
            .id = sqlite3_column_int(stmt,0),
            .name = name
        });  //Stores the potential indexes in a vector which user will SELECT
        
        cout << count << "." << name << "\tInitial Balance is: " << balance << endl;
        count++;
    } while (sqlite3_step(stmt) == SQLITE_ROW);

    cout << endl;
    sqlite3_finalize(stmt);

    int selection;
    do {
        selection = input<int>("Select a moneypool (enter zero to return to main menu): ");
        if (selection > choices.size() || selection < 0) {
            cout << "Invalid choice... Input a valid choice!" << endl;
        }
    } while (selection > choices.size() || selection < 0);

    if (selection == 0) {
        return;
    }

    bruh moneypool = choices[selection -1];

    transaction_UI(mdb, user_id, moneypool.id, moneypool.name);
}

/*
 * This functions is used when user logins successfully and wants to edit his account types i.e bank, card, easypaisa, etc
 */
void add_moneypool(sqlite3* mdb, int user_id) {
    string pool_name;
    double balance;

    sqlite3_stmt* stmt;
    const char* insert_stmt = "INSERT INTO moneypools (user_id, pool_name, initial_balance) VALUES"
    "(?, ?, ?);";
    sqlite3_prepare_v2(mdb, insert_stmt, -1, &stmt, nullptr);

    do{
        balance = input<double>("Enter the initial balance for this money pool");
        if (balance < 0) {
            cout << "Negative balance isn't possible" << endl;
        }
    } while (balance < 0);

   
    while(true) {
        do {
            pool_name = input<string>("Enter the name of moneypool you want to add: ");
            if (pool_name.size() > 50) {
                cout << "Name too big (> 50)" << endl;
            }
        } while (pool_name.size() > 50);

        sqlite3_clear_bindings(stmt);
        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_bind_text(stmt, 2, pool_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 3, balance);

        int rc = sqlite3_step(stmt);

        if (rc == SQLITE_CONSTRAINT) {
            int extended_err = sqlite3_extended_errcode(mdb);
            if (extended_err == SQLITE_CONSTRAINT_UNIQUE){
                cout << "Money pool already created with this name" << endl;
                sqlite3_reset(stmt);
            }

            continue;
        } else if (rc == SQLITE_DONE) {
            // Nikal jaa loop sey seedhi baat no bakwas
            break;
        } else {
            cout << "Something bad happened yawr\n Fix it Zaddy\nHere's the msg\n";
            cout << sqlite3_errmsg(mdb) << endl;
        }
        
    }
}

void edit_moneypool(sqlite3* mdb, int user_id){
    sqlite3_stmt* stmt;
    const char *select_stmt = "SELECT * FROM moneypools WHERE user_id = ?;";
    vector<int> moneypool_ids;
    
    sqlite3_prepare_v2(mdb, select_stmt, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, user_id);
    
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        cout << "There are absolutely no fucking pools" << endl;
        return;
    }

    cout << "ID | name | initial balance" << endl;
    do {
        int id = sqlite3_column_int(stmt, 0);
        const char *name = (const char*)sqlite3_column_text(stmt, 2);
        double initial_balance = sqlite3_column_double(stmt, 3);

        moneypool_ids.push_back(id);
        cout << id << " | "
             << name << " | "
             << fixed << setprecision(2) << initial_balance << endl;
    } while (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);

    int moneypool = -1;

    while (true) {
        moneypool = input<int>("Enter a moneypool id that you want to edit you moronic jackass (-1 to exit): ");
        if (moneypool == -1) {
            return;
        } else if (find(moneypool_ids.begin(), moneypool_ids.end(), moneypool) == moneypool_ids.end()) {
            cout << "Invalid moneypool ID" << endl;
            continue;
        }
        break;
    }

    const char *edit_stmt =
        "UPDATE moneypools "
        "SET pool_name = ?, initial_balance = ? "
        "WHERE id = ?;";

    string name;
    double initial_balance;

    do {
        name = input<string>("Enter a new name: ");
        if (name.size() == 0) {
            cout << "Name can't be empty" << endl;
        }
    } while (name.size() == 0);

    do {
        initial_balance = input<double>("Enter a new initial balance: ");
        if (initial_balance < 0) {
            cout << "Can't be negative" << endl;
        }
    } while (initial_balance < 0);


    sqlite3_prepare_v2(mdb, edit_stmt, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, initial_balance);
    sqlite3_bind_int(stmt, 3, moneypool);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cout << sqlite3_errmsg(mdb) << endl;
        sqlite3_finalize(stmt);
        sqlite3_close(mdb);
        exit(EXIT_FAILURE);
    }

    sqlite3_finalize(stmt);
}