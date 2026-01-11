#include <iostream>
#include <limits>
#include <sqlite3.h>


using namespace std;

const long long int MAXIGNORE = numeric_limits<streamsize>::max();

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
            "user_name  VARCHAR(50) UNIQUE, "
            "first_name VARCHAR(50) NOT NULL, "
            "last_name  VARCHAR(50) NOT NULL, "
            "age        INTEGER NOT NULL, "
            "password   VARCHAR(20) NOT NULL"
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
            "id         INTEGER PRIMARY KEY NOT NULL, "
            "user_id    INTEGER NOT NULL, "
            "name       VARCHAR(50), "

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
            "category_id    INTEGER NOT NULL, "
            "moneypool_id   INTEGER NOT NULL, "
            "amount         DECIMAL(10,2) NOT NULL, "
            "currency       CHAR(3) DEFAULT 'PKR', "
            "exchange_rate  DECIMAL(10,2) DEFAULT 1, " /* Exchange rate is always relative to the pakistani rupee. */
            "timestamp      DATETIME DEFAULT CURRENT_TIMESTAMP," /* Fancy shmancy */
            "notes          VARCHAR(100), "

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
            "upper_limit    DECIMAL(10,2) NOT NULL, "
            "start_time     DATETIME NOT NULL, "
            "end_time       DATETIME NOT NULL, "

            "FOREIGN KEY (user_id) REFERENCES users(id), "
            "FOREIGN KEY (category_id) REFERENCES categories(id)"
        ");"
    ;

    sqlite3_extended_result_codes(mdb, true);

    if (sqlite3_exec(mdb, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &err)  != SQLITE_OK) {   //To enable foreign keys and to check if any error
        cerr << err << endl;
        return -1;
    }

    if (sqlite3_exec(mdb, user_stmt, nullptr, nullptr, &err) != SQLITE_OK) {      //To create USER TABLE and to check for any errors
        cerr << err << endl;
        return -1;
    }

    if (sqlite3_exec(mdb, moneypool_stmt, nullptr, nullptr, &err) != SQLITE_OK) {      //To create MONEYPOOL TABLE and to check for any errors
        cerr << err << endl;
        return -1;
    }

    if (sqlite3_exec(mdb, category_stmt, nullptr, nullptr, &err) != SQLITE_OK) {       //To create CATEGORY TABLE and to check for any errors
        cerr << err << endl;
        return -1;
    }

    if (sqlite3_exec(mdb, transaction_stmt, nullptr, nullptr, &err) != SQLITE_OK) {    //To create TRANSACTION TABLE and to check for any errors
        cerr << err << endl;
        return -1;
    }

    if (sqlite3_exec(mdb, budget_stmt, nullptr, nullptr, &err) != SQLITE_OK) {         //To create BUDGET TABLE and to check for any errors
        cerr << err << endl;
        return -1;
    }

    return 0;
}


void init_usertable(sqlite3* mdb) {
    return;
}

/*
 * To setup user accounts and register them on the USER TABLE
 * (Confirm if you want assign finance database here the moment user is registered on USER udb or assign it later)
 */
void account_setup(sqlite3* mdb) {
    return;
}

/*
 * Will finance table which will be ASSIGNED to the USER TABLE using foreign key
 */
void init_financetable(sqlite3* mdb) {
    return;
}

/*
 * To setup the init_finance table (just like account_setup())
 */
void finance_setup(sqlite3* mdb) {
    return;
}

void clear_screen() {
    cout << "\033[2J\033[H";
}

template <typename T> // This bad boy is covering every type of input in one function.
T input(const char *prompt) {
    T result;

    while (true) {
        cout << prompt;
        cin >> result;
        if (!cin.fail()) {
            cin.ignore(MAXIGNORE, '\n');
            break;
        }

        cin.clear();
        cin.ignore(MAXIGNORE, '\n');
        cout << "Bad input!" << endl;
    }

    return result;
}

template <> // We are essentially handling the case when T is string differently
string input<string>(const char *prompt) {
    string result;
    cout << prompt;
    getline(cin, result);

    return move(result);
}

/*
 * This is to register user onto the USER TABLE and create his FINANCE TABLE USING ABOVE FUNCTIONS
 */
void signup_UI(sqlite3* mdb) {
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

    while (true) {
        do {
            username = input<string>("Enter your username: ");
            if (username.size() > 50) {
                cout << "Username too big (> 50)!" << endl;
            }
        } while (username.size() > 50);

        sqlite3_bind_text(stmt, 1, username.c_str(), first_name.size(), SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, first_name.c_str(), first_name.size(), SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, last_name.c_str(), last_name.size(), SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, age);
        sqlite3_bind_text(stmt, 5, password.c_str(), password.size(), SQLITE_STATIC);

        int rc = sqlite3_step(stmt);

        // Useful for testing and shi
        // cout << rc << endl;
        // cout << sqlite3_errmsg(mdb) << endl;

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
void login_UI(sqlite3* mdb) {
    return;
}

/*
 * Enters here when login_UI() is successful
 * Will add entries based off money_pool types
 */
void menu_UI() {
    return;
}

/*
 * To add items in the predefined slots of finance table based off which user is using
 * Will be added to finance table grouped using USER_ID in USER TABLE as foreign key
 */

void add_item(sqlite3* mdb) {
    return;
}

/*
 * To remove items in order to handle clutter or the ones which user doesn't want to see
 * (Sort of pointless as user is using this to keep TRACK but ehh better safe than sorry)
 * Will be added to finance table grouped using USER_ID in USER TABLE as foreign key
 */

void remove_item(sqlite3* mdb) {
    return;
}

/*
 * Filters through the finance table based off timestamps showing data based off last week, last year, etc
 * Uses the USER_ID from USER TABLE as foreign key ofc
 */

void filter_bytime(sqlite3* mdb) {
    return;
}

/*
 * Filters through udb based off category (Can include other miscellaneous metrics like date, amount withdrawn etc)
 * *EXTRA NOTE*
 * (Make the search functions based off seperate utilities e.g based off some amount withdrawn like showing all activities where amount withdrawn > 10,000)
 * Also decide if you want to make this into ONE FUNCTION or seperate functions like filter_bytime(), search_bycategory, search_bycash()
 * Better to make it as seperate functions so you can make several filtering methods when making GUI of this project
 */

void search_bycategory(sqlite3* mdb) {
    return;
}

/*
 * Filters based off cash ranges like amount >10,000 withdrawn etc
 */

void search_bycash(sqlite3* mdb) {
    return;
}

/*
 * This includes the moneypool types and will search based off above 3 search_types
 */

void search_UI() {
    return;
}

int main() {
    sqlite3* mdb = nullptr;
    sqlite3_open("main.db", &mdb);

    clear_screen();
    init_maindb(mdb);

    int choice;
    do{
        cout << "0.Exit\n1.Sign up\n2.Login\n\n";
        choice = input<int>("Enter the damn choice: ");

        clear_screen();
        switch(choice){
        case 0:
            break;

        case 1:
            signup_UI(mdb);
            break;

        case 2:
            // login_UI(mdb);
            break;

        default:
            cout << "call kru bacha?";
            continue;
        }

        break;
    } while(choice != 0);

    sqlite3_close(mdb);
}


