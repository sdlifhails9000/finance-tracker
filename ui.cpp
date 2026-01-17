#include <iostream>
#include <iomanip>
#include <vector>
#include <sqlite3.h>

#include "utils.h"
#include "users.h"
#include "categories.h"
#include "transactions.h"
#include "moneypools.h"

#include "ui.h"

using namespace std;

void category_UI(sqlite3* mdb, int user_id) {
    sqlite3_stmt *stmt;
    vector<int> category_ids;

    const char* view_stmt =
        "SELECT id, name, budget_amount FROM categories "
        "WHERE user_id = ?;"; 

    sqlite3_prepare_v2(mdb, view_stmt, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, user_id);

    clear_screen();
    cout << "\nWelcome to ur categories\n" << endl;

    int max_id;
    switch (sqlite3_step(stmt)) {
    case SQLITE_DONE:
        cout << "No categories at the moment. Please enter something you dipshit" << endl;
        break;

    case SQLITE_ROW:
        cout << "Id | name | budget" << endl;
        do {
            int id = sqlite3_column_int(stmt, 0);
            category_ids.push_back(id);

            const char* name = (const char*)sqlite3_column_text(stmt, 1);
            double budget = sqlite3_column_double(stmt, 2);

            max_id = id;
            cout << id << " | " 
                 << name << " | "
                 << fixed << setprecision(2) << budget << endl;
        } while (sqlite3_step(stmt) == SQLITE_ROW);
        break;

    default:
        cout << "Call karu bacha?\n" << sqlite3_errmsg(mdb) << endl;
    }

    int selection;
    do {
        cout << "1.Add Category\n2.Edit Category\n0.Exit\n";
        selection = input<int>("Make your damn choice big boy: ");
        
        switch(selection){
        case 0:
            break;

        case 1:
            add_category(mdb, user_id);
            break;

        case 2:
            edit_category(mdb, category_ids);   //max_id is to be passed so that we have the upper limit of VALID CHOICES (improves efficiency so we dont have to reconstruct it in edit_transaction())
            break;
        default:
            cout << "Call kru bacha?" << endl;
            continue;    
        }
        break;
    } while (selection != 0);
    return;
}

void transaction_UI(sqlite3* mdb, int user_id, int moneypool_id, const string& moneypool_name) {
    sqlite3_stmt *stmt;
    vector<int> transaction_ids;
    
    clear_screen();

    cout << "Moneypool: " << moneypool_name << "\n" << endl;

    const char* view_stmt =
        "SELECT "
            "transactions.id, transactions.amount, transactions.currency, transactions.exchange_rate, transactions.timestamp, transactions.notes,"
            "categories.name "
        "FROM transactions "
        "LEFT JOIN categories ON transactions.category_id = categories.id "
        "WHERE transactions.user_id = ? AND transactions.moneypool_id = ?;";

    sqlite3_prepare_v2(mdb, view_stmt, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_int(stmt, 2, moneypool_id);

    cout << "\nWelcome to ur finances\n" << endl;

    switch (sqlite3_step(stmt)) {
    case SQLITE_DONE:
        cout << "Your list is empty. Please enter something you dipshit" << endl;
        break;

    case SQLITE_ROW:
        cout << "ID | category | amount | type | note | Time" << endl;
        do {
            int id = sqlite3_column_int(stmt, 0);
            transaction_ids.push_back(id);          //Preparing a vector that stores ids for edit_transaction

            double amount = sqlite3_column_double(stmt, 1);
            
            //Test case 101 if anything breaks blame this mfer const char (replace with string then)
            const char* currency = (const char*)sqlite3_column_text(stmt, 2);

            double exchange_rate = sqlite3_column_double(stmt, 3);
            const char* timestamp = (const char*)sqlite3_column_text(stmt, 4);
            const char* notes = (const char*)sqlite3_column_text(stmt, 5);
            const char *category_name = (const char*)sqlite3_column_text(stmt, 6); 
            
            if (category_name == nullptr) {
                category_name = "(none)";
            }

            cout << id << " | "
                 << category_name << " | "
                 << fixed << setprecision(2) << amount << " | "
                 << currency << " | "
                 << fixed << setprecision(2) << exchange_rate << " | "
                 << notes << " | "
                 << timestamp << endl;
        } while (sqlite3_step(stmt) == SQLITE_ROW);
        break;

    default:
        cout << "Call karu bacha?";
    }

    int selection;
    do {
        cout << "1.Add Transaction\n2.Edit Transaction\n0.Exit\n";
        selection = input<int>("Make your damn choice big boy: ");
        
        switch(selection){
        case 0:
            break;

        case 1:
            add_transaction(mdb, user_id, moneypool_id);
            break;

        case 2:
            edit_transaction(mdb, user_id, moneypool_id, transaction_ids);   //max_id is to be passed so that we have the upper limit of VALID CHOICES (improves efficiency so we dont have to reconstruct it in edit_transaction())
            break;

        default:
            cout << "Call kru bacha?" << endl;    
        }

        break;
    } while (selection != 0);
    return;
    
}

void moneypool_UI(sqlite3 *mdb, int user_id) {
    int selection;

    clear_screen();
    do {
        cout << "1.View moneypools\n"
             << "2.Edit moneypools\n"
             << "3.Add moneypools\n"
             << "0.Back" << endl;
        
        selection = input<int>("Enter an option: ");
        switch (selection) {
        case 1:
            view_moneypool(mdb, user_id);
            break;
        
        case 2:
            edit_moneypool(mdb, user_id);
            break;
        
        case 3:
            add_moneypool(mdb, user_id);
            break;

        case 0:
            break;

        default:
            cout << "Invalid option!" << endl;
        }
    } while (selection != 0);
}

/*
 * Enters here when login_UI() is successful
 * Will add entries based off money_pool types
 
 * EXTRA NOTES:
 * This will be the very first UI right after `login_verify`
 */
void menu_UI(sqlite3* mdb, User u) {
    clear_screen();
    int selection;
    
    /*

    cout << "0.Exit" << endl;
    cout << "1.Money Pools Setup" << endl;
    cout << "2.Categories Setup" << endl;
    

    */
   
   do{
        cout << "1.View Money Pools\n"
             << "2.Categories Setup\n"
             << "0.Exit" << endl;
        selection = input<int>("Make your damn choice: ");

        switch(selection){
        case 1:
            moneypool_UI(mdb, u.id);
            break;

        case 2:
            category_UI(mdb, u.id);
            break;

        case 0: 
            break;

        default:
            cout << "Call kru bacha?" << endl;
        }
    } while (selection != 0);
}
