#include <iostream>
#include <sqlite3.h>

#include "utils.h"
#include "users.h"
#include "ui.h"

using namespace std;

/*
 * Will create tables according to account setups for USER DATABASE
 * Make main db and form tables like usertable(), etc
 */
int init_maindb(sqlite3* mdb) {
    char* err = nullptr;

    /* Statements for user table which user will set initially
     * TODO add constraints
     */
    const char *user_stmt =
        "CREATE TABLE IF NOT EXISTS users ("
            "id         INTEGER PRIMARY KEY NOT NULL, "
            "user_name  VARCHAR(50) NOT NULL UNIQUE, "
            "first_name VARCHAR(50) NOT NULL, "
            "last_name  VARCHAR(50) NOT NULL, "
            "age        INTEGER NOT NULL, "
            "password   CHAR(32) NOT NULL"
        ");"
    ;

    /* Statements for moneypool table which user will set themself in finance_setup()
     */
    const char *moneypool_stmt =
        "CREATE TABLE IF NOT EXISTS moneypools ("
            "id                 INTEGER PRIMARY KEY NOT NULL, "
            "user_id            INTEGER NOT NULL, "
            "pool_name          VARCHAR(50) NOT NULL, "
            "initial_balance    DECIMAL(10,2) DEFAULT 0, "

            "FOREIGN KEY (user_id) REFERENCES users(id)"
            "UNIQUE (user_id, pool_name)"
        ");"
    ;

    /* Statement for the category table. It could be food, rent, whatever...
     * Useful when budgeting. Like say you wanted to cap food spendings for a bit,
     * you'd restrict all transaction with the food category such that the total
     * amount of those transaction for the time of that budget doesn't go over a
     * certain maximum.
     */
    const char *category_stmt =
        "CREATE TABLE IF NOT EXISTS categories ("
            "id             INTEGER PRIMARY KEY NOT NULL, "
            "user_id        INTEGER NOT NULL, "
            "name           VARCHAR(50) NOT NULL, "
            "budget_amount  DECIMAL (10,2) DEFAULT 0.00,"

            "FOREIGN KEY (user_id) REFERENCES users(id), "
            "UNIQUE (user_id, name)"
        ");"
    ;

    /* Statement for the transaction table. Yada yada
     */
    const char *transaction_stmt =
        "CREATE TABLE IF NOT EXISTS transactions ("
            "id             INTEGER PRIMARY KEY NOT NULL, "
            "user_id        INTEGER NOT NULL, "
            "category_id    INTEGER NULL,"
            "moneypool_id   INTEGER NOT NULL, "
            "amount         DECIMAL(10,2) NOT NULL, "
            "currency       CHAR(3) DEFAULT 'PKR', "
            "exchange_rate  DECIMAL(10,2) DEFAULT 1, " /* Exchange rate is always relative to the pakistani rupee. */
            "timestamp      DATETIME DEFAULT CURRENT_TIMESTAMP," /* Fancy shmancy */
            "notes          VARCHAR(500) NULL, "

            "FOREIGN KEY (user_id) REFERENCES users(id), "
            "FOREIGN KEY (category_id) REFERENCES categories(id), "
            "FOREIGN KEY (moneypool_id) REFERENCES moneypools(id)"
        ");"
    ;

    /* Statement for the budget table.
     * Spending limits for a certain period of time for certain categories of transactions
     * This will only serve as a suggestion and would force to be implemented if user adds an upper limit for budget, otherwise it can be ignored
     */
    const char *budget_stmt =
        "CREATE TABLE IF NOT EXISTS budgets ("
            "id             INTEGER PRIMARY KEY NOT NULL,"
            "user_id        INTEGER NOT NULL, "
            "category_id    INTEGER NOT NULL, "
            "upper_limit    DECIMAL(10,2), "
            "start_time     DATETIME, "
            "end_time       DATETIME, "

            "FOREIGN KEY (user_id) REFERENCES users(id), "
            "FOREIGN KEY (category_id) REFERENCES categories(id)"
        ");"
    ;

    if (sqlite3_exec(mdb, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &err)  != SQLITE_OK) {   //To enable foreign keys and to check if any error
        cerr << err << endl;
        cerr << sqlite3_errmsg(mdb) << endl;
        return -1;
    }

    if (sqlite3_exec(mdb, user_stmt, nullptr, nullptr, &err) != SQLITE_OK) {      //To create USER TABLE and to check for any errors
        cerr << err << endl;
        cerr << sqlite3_errmsg(mdb) << endl;
        return -1;
    }

    if (sqlite3_exec(mdb, moneypool_stmt, nullptr, nullptr, &err) != SQLITE_OK) {      //To create MONEYPOOL TABLE and to check for any errors
        cerr << err << endl;
        cerr << sqlite3_errmsg(mdb) << endl;
        return -1;
    }

    if (sqlite3_exec(mdb, category_stmt, nullptr, nullptr, &err) != SQLITE_OK) {       //To create CATEGORY TABLE and to check for any errors
        cerr << err << endl;
        cerr << sqlite3_errmsg(mdb) << endl;
        return -1;
    }

    if (sqlite3_exec(mdb, transaction_stmt, nullptr, nullptr, &err) != SQLITE_OK) {    //To create TRANSACTION TABLE and to check for any errors
        cerr << err << endl;
        cerr << sqlite3_errmsg(mdb) << endl;
        return -1;
    }

    if (sqlite3_exec(mdb, budget_stmt, nullptr, nullptr, &err) != SQLITE_OK) {         //To create BUDGET TABLE and to check for any errors
        cerr << err << endl;
        cerr << sqlite3_errmsg(mdb) << endl;
        return -1;
    }

    return 0;
}

int main() {
    sqlite3* mdb = nullptr;
    sqlite3_open("main.db", &mdb);

    init_maindb(mdb);

    int choice;
    do{
        clear_screen();
        cout << "0.Exit\n1.Sign up\n2.Login\n\n";
        choice = input<int>("Enter the damn choice: ");

        clear_screen();
        switch(choice){
        case 1:
            account_setup(mdb);
            break;

        case 2: {
            User u = login_verify(mdb);     //Stores prepared user (meth)
            menu_UI(mdb,u);
            break;
        }

        case 0:
            break;

        default:
            cout << "call kru bacha?";
            continue;
        }
    } while(choice != 0);

    sqlite3_close(mdb);
}
