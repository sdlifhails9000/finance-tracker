/* TODO Add transaction deletion */

#include <iostream>
#include <vector>
#include <algorithm>
#include <sqlite3.h>

#include "utils.h"

#include "transactions.h"

using namespace std;

void add_transaction(sqlite3* mdb, int user_id, int moneypool_id) {
    sqlite3_stmt *stmt;
    string currency, notes;
    double exchange_rate;
    int category = -1;

    const char* insert_stmt = "INSERT INTO transactions (amount, currency, exchange_rate, notes, user_id, moneypool_id, category_id) VALUES"
    "(?,?,?,?,?,?,?);";

    sqlite3_prepare_v2(mdb, insert_stmt, -1, &stmt, nullptr);

    double amount = input<double>("Enter the amount: ");        //negative for spending and positive for receiving
    do {
        currency = input<string>("Enter currency type (PKR, USD, etc, default is PKR): ");
        if (currency.size() == 0) {
            currency = "PKR";
            break;
        } else if (currency.size() !=3) {
            cout << "Invalid currency type bacha";
        }
    } while(currency.size() != 3);

    if (currency == "PKR") {
        exchange_rate = 1;
    } else {
        do {
            exchange_rate = input<double>("Enter the current exchange rate for the currency type you entered: ");
            if (exchange_rate <= 0 ){
                cout << "Illegal in 14 states sir" << endl;
            }
            
        }while (exchange_rate <= 0);
    }

    // Display categories

    sqlite3_stmt *stmt1;
    const char *select_stmt = "SELECT id, name FROM categories WHERE user_id = ?;";

    sqlite3_prepare_v2(mdb, select_stmt, -1, &stmt1, nullptr);
    sqlite3_bind_int(stmt1, 1, user_id);

    cout << endl;

    if (sqlite3_step(stmt1) != SQLITE_ROW) {
        cout << "There are no categories! Make some, BIG BOY!!!" << endl;
    } else {
        vector<int> category_list;

        cout << "ID | name" << endl;
        do {
            int id = sqlite3_column_int(stmt1, 0);
            const char *name = (const char*)sqlite3_column_text(stmt1, 1);

            // Pushes a set of valid categories into this fucking list that we made.
            category_list.push_back(id);

            cout << id << " | " <<  name << endl;
        } while (sqlite3_step(stmt1) == SQLITE_ROW);

        sqlite3_finalize(stmt1);

        while (true) {
            category = input<int>("Select a category (-1 for none): ");
            if (category == -1) {
                break;
            } else if (find(category_list.begin(), category_list.end(), category) == category_list.end()) {
                cout << "Invalid category ID" << endl;
            }
            break;
        }
    }

    // ------------------

    do{
        notes = input<string>("Enter any comment regarding this transaction ( < 500 ): ");
        if (notes.size() > 500){
            cout << "Mfer this is a notes section not your r/Hentai section you tharki pedo" << endl;
        }
    }while (notes.size() > 500);

    sqlite3_clear_bindings(stmt);
    sqlite3_bind_double(stmt, 1, amount);
    sqlite3_bind_text(stmt, 2, currency.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, exchange_rate);
    sqlite3_bind_text(stmt, 4, notes.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, user_id);
    sqlite3_bind_int(stmt, 6, moneypool_id);

    if (category != -1) {
        sqlite3_bind_int(stmt, 7, category);
    }

    int rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
        cout << "Body Cam Off Since Vietnam BOY!" << endl;
        cout << sqlite3_errmsg(mdb) << endl;
        sqlite3_finalize(stmt);
        exit(EXIT_FAILURE);
    }

    sqlite3_finalize(stmt);
}

void edit_transaction(sqlite3* mdb, int user_id, int moneypool_id, vector<int> transaction_ids) {
    sqlite3_stmt *stmt;
    const char *stmt_str =
        "UPDATE transactions "
        "SET amount = ?, currency = ?, exchange_rate = ?, category_id = ?, notes = ? "
        "WHERE id = ?;";

    int transaction_id = -1;
    int category = -1;
    double amount;
    double exchange_rate = 1.0;
    string currency;
    string notes;

    while (true) {
        transaction_id = input<int>("Enter a transaction ID to edit (-1 to exit): ");
        if (transaction_id == -1) {
            return;
        } else if (find(transaction_ids.begin(), transaction_ids.end(), transaction_id) == transaction_ids.end()) {
            cout << "Invalid transaction ID" << endl;
            continue;
        }
        break;
    }

    do {
        amount = input<double>("Enter new amount (must non-zero): ");
        if (amount == 0) {
            cout << "Moron" << endl;
        }
    } while (amount == 0);

    do {
        currency = input<string>("Enter a new currency (Leave empty for PKR): ");
        if (currency.size() == 0) {
            break;
        }
        if (currency.size() != 3) {
            cout << "Invalid format" << endl;
        }
    } while (currency.size() != 3);

    for (auto& c : currency) {
        c = toupper(c);
    }

    if (currency.size() != 0 && currency != "PKR") {
        do {
            exchange_rate = input<double>("Enter an exchange rate relative to PKR: ");
            if (exchange_rate <= 0) {
                cout << "Can't be non-positive" << endl;
            }
        } while (exchange_rate <= 0);
    }

    if (currency.size() == 0) {
        currency = "PKR";
    }

    do {
        notes = input<string>("Enter notes: ");
        if (notes.size() > 500) {
            cout << "Note too big" << endl;
        }
    } while (notes.size() > 500);

    // Display categories

    sqlite3_stmt *stmt1;
    const char *select_stmt = "SELECT id, name FROM categories WHERE user_id = ?;";

    sqlite3_prepare_v2(mdb, select_stmt, -1, &stmt1, nullptr);
    sqlite3_bind_int(stmt1, 1, user_id);

    cout << endl;

    if (sqlite3_step(stmt1) != SQLITE_ROW) {
        cout << "There are no categories! Make some, BIG BOY!!!" << endl;
    } else {
        vector<int> category_list;

        cout << "ID | name" << endl;
        do {
            int id = sqlite3_column_int(stmt1, 0);
            const char *name = (const char*)sqlite3_column_text(stmt1, 1);

            // Pushes a set of valid categories into this fucking list that we made.
            category_list.push_back(id);

            cout << id << " | " <<  name << endl;
        } while (sqlite3_step(stmt1) == SQLITE_ROW);

        sqlite3_finalize(stmt1);

        while (true) {
            category = input<int>("Select a category (-1 for none): ");
            if (category == -1) {
                break;
            } else if (find(category_list.begin(), category_list.end(), category) == category_list.end()) {
                cout << "Invalid category ID" << endl;
                continue;
            }
            break;
        }
    }

    // ---------------------

    sqlite3_prepare_v2(mdb, stmt_str, -1, &stmt, nullptr);
    sqlite3_bind_double(stmt, 1, amount);
    sqlite3_bind_text(stmt, 2, currency.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, exchange_rate);

    // Shaheer is a bit spooked here.    
    if (category != -1) {
        sqlite3_bind_int(stmt, 4, category);
    }

    sqlite3_bind_text(stmt, 5, notes.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, transaction_id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        cout << sqlite3_errmsg(mdb) << endl;
        sqlite3_finalize(stmt);
        sqlite3_close(mdb);
        exit(EXIT_FAILURE);
    }
    
    return;
}
