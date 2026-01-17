#include <iostream>
#include <vector>
#include <algorithm>
#include <sqlite3.h>

#include "utils.h"

#include "categories.h"

using namespace std;

void add_category(sqlite3 *mdb, int user_id){
    string category_name;
    double budget_amount = 0;

    sqlite3_stmt* stmt;
    const char* insert_stmt = "INSERT INTO categories (user_id, name, budget_amount) VALUES"
    "(?, ?, ?);";
    sqlite3_prepare_v2(mdb, insert_stmt, -1, &stmt, nullptr);
   
    while(true) {
        do {
            category_name = input<string>("Enter the name of category you want to add: ");
            if (category_name.size() > 50) {
                cout << "Name too big (> 50)" << endl;
            }
        } while (category_name.size() > 50);

        char want_budget;
        want_budget = input<char>("Do you want to set a budget for this category (Y/n)? (Optional)");
        want_budget = tolower(want_budget);

        if (want_budget == 'y'){
            do {
                budget_amount = input<double>("Enter the budget for this category");
                if (budget_amount <= 0) {
                    cout << "Negative balance isn't possible" << endl;
                }
            } while (budget_amount <= 0);
        }

        sqlite3_clear_bindings(stmt);
        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_bind_text(stmt, 2, category_name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 3, budget_amount);

        int rc = sqlite3_step(stmt);

        if (rc == SQLITE_CONSTRAINT) {
            int extended_err = sqlite3_extended_errcode(mdb);
            if (extended_err == SQLITE_CONSTRAINT_UNIQUE){
                cout << "A Category already exists with this name" << endl;
                sqlite3_reset(stmt);
            }

            continue;
        } else if (rc == SQLITE_DONE) {
            // Nikal jaa loop sey seedhi baat no bakwas
            break;
        } else {
            cout << "Something bad happened yawr\n Fix it Zaddy\nHere's the msg\n";
            cout << sqlite3_errmsg(mdb) << endl;
            cout << rc; //Testing
            exit(EXIT_FAILURE);
            
        }
        
    }
}

void edit_category(sqlite3 *mdb, vector<int> category_ids){
    sqlite3_stmt *stmt;
    const char *stmt_str =
        "UPDATE categories"
        "SET name = ?, budget_amount = ? "
        "WHERE id = ?";

    int selection;
    while (true) {
        selection = input<int>("Enter the category you wanna edit: ");
        if (find(category_ids.begin(), category_ids.end(), selection) == category_ids.end()) {
            cout << "Invalid ID!" << endl;
        } else {
            break;
        }
    }

    sqlite3_prepare_v2(mdb, stmt_str, -1, &stmt, nullptr);

    string name;
    double budget_amount = 0;

    while (true) {
        do {
            name = input<string>("Enter new name (at most 50 characters): ");
            if (name.size() != 0) {
                cout << "Name can't be empty" << endl;
            } else if (name.size() > 50) {
                cout << "Name too big. Should be at most 50 character" << endl;
            }
        } while (name.size() != 0 && name.size() > 50);

        do {
            budget_amount = input<double>("Enter budget amount (zero for no budget): ");
            if (budget_amount < 0) {
                cout << "Budget can't be negative" << endl;
            }
        } while (budget_amount < 0);

        sqlite3_clear_bindings(stmt);
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 2, budget_amount);
        sqlite3_bind_int(stmt, 3, selection);

        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_CONSTRAINT) {
            int extended = sqlite3_extended_errcode(mdb);
            if (extended == SQLITE_CONSTRAINT_UNIQUE) {
                cout << "Name already exists!" << endl;
                sqlite3_reset(stmt);
                continue;
            } else {
                cout << sqlite3_errmsg(mdb) << endl;
                exit(EXIT_FAILURE);
            }
        } else if (rc != SQLITE_DONE) {
            cout << sqlite3_errmsg(mdb) << endl;
            exit(EXIT_FAILURE);
        }
    }
}
