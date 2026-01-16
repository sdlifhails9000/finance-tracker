#include <iostream>
#include <limits>
#include <sqlite3.h>
#include <vector>
#include <iomanip>
#include <algorithm>

#include <openssl/evp.h>

using namespace std;

const long long int MAXIGNORE = numeric_limits<streamsize>::max();

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

/* ----------- HELPER FUNCTIONS ----------- */


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

    return result;
}

void calc_hash(std::string msg, unsigned char *hash, unsigned int *hash_len) {
    EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();

    EVP_DigestInit_ex(md_ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(md_ctx, msg.c_str(), msg.size());
    EVP_DigestFinal_ex(md_ctx, hash, hash_len);

    EVP_MD_CTX_free(md_ctx);
}

/* ---------------------------------------- */

void add_transaction(sqlite3* mdb, int user_id, int moneypool_id){
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
    return;
}

void edit_transaction(sqlite3* mdb, int user_id, int moneypool_id, vector<int> transaction_ids) {
    return;
}


void transaction_UI(sqlite3* mdb, int user_id, int moneypool_id) {
    sqlite3_stmt *stmt;
    vector<int> transaction_ids;

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

/*
 * This functions is used when user logins successfully and wants to view his account types i.e bank, card, easypaisa, etc
 */
void view_moneypool(sqlite3* mdb, int user_id) {
    sqlite3_stmt* stmt;
    const char* select_stmt = "SELECT id, pool_name, initial_balance FROM moneypools WHERE user_id = ?;";
    sqlite3_prepare_v2(mdb, select_stmt, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, user_id);

    cout << endl;

    int count = 1;
    vector <int> choices;
    if (sqlite3_step(stmt) != SQLITE_ROW && count == 1){
        cout << "No moneypools available" << endl;
        return;
    } 

    // Goes through each row and outputs it with a number so user can make choice
    do {
        string name = (const char*)sqlite3_column_text(stmt,1);
        double balance = sqlite3_column_double(stmt,2);
        choices.push_back(sqlite3_column_int(stmt,0));  //Stores the potential indexes in a vector which user will SELECT
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

    int moneypool_id = choices[selection -1];

    transaction_UI(mdb, user_id, moneypool_id);
}


/*
 * This functions is used when user logins successfully and wants to edit his account types i.e bank, card, easypaisa, etc
 */
void moneypool_add(sqlite3* mdb, int user_id) {
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
    return;    
}

void moneypool_edit(sqlite3* mdb, int user_id){
    return;
}


void add_category(sqlite3*mdb, int user_id){
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

            }while (budget_amount <= 0);
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
    return;    
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



void category_UI(sqlite3* mdb, int user_id){
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
        cout << "Id | name | Budget" << endl;
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

/*
 * Enters here when login_UI() is successful
 * Will add entries based off money_pool types
 */
void menu_UI(sqlite3* mdb, User u) {
    clear_screen();
    int selection;
    
    do{
        cout << "1.View Money Pools" << endl;
        cout << "2.Add Money Pool" << endl;
        cout << "3.Edit Existing Money Pools" << endl;
        cout << "4.Categories Setup (optional)" << endl;
        cout << "0.Exit" << endl;
        selection = input<int>("Make your damn choice: ");

        switch(selection){
            case 1:
                view_moneypool(mdb, u.id);
                break;                          //Directly passing user id from the struct defined instead of the ENTIRE struct
            case 2:
                moneypool_add(mdb,u.id);
                break;
            case 3:
                moneypool_edit(mdb, u.id);
                break;
            case 4:
                category_UI(mdb, u.id);
                break;
            case 0: 
                break;
            default:
                cout << "Call kru bacha?" << endl;
        }
    } while (selection != 0);
    
    
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

    init_maindb(mdb);

    int choice;
    do{
        clear_screen();
        cout << "0.Exit\n1.Sign up\n2.Login\n\n";
        choice = input<int>("Enter the damn choice: ");

        clear_screen();
        switch(choice){
        case 0:
            break;

        case 1:
            account_setup(mdb);
            break;

        case 2:{
            User u = login_verify(mdb);     //Stores prepared user (meth)
            menu_UI(mdb,u);
            break;
        }

        default:
            cout << "call kru bacha?";
            continue;
        }
    } while(choice != 0);

    sqlite3_close(mdb);
}


