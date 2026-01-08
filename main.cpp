#include <iostream>
#include <sqlite3.h>

using namespace std;

//Will create tables according to account setups for USER DATABASE

//Make main db and form tables like usertable(), etc

void init_maindb(sqlite3* mdb){
    char* err = nullptr;
    string user_stmt = "CREATE TABLE IF NOT EXISTS users ("
    "id INT PRIMARY KEY AUTOINCREMENT NOT NULL, "
    "user_name VARCHAR(50) UNIQUE,"                 //Statements for user table which user will set initially
    "first_name VARCHAR(50),"                       //TO DO (make constraints)
    "last_name VARCHAR(50),"
    "age INT,"
    "pass VARCHAR(20));";

    string moneypool_stmt = "CREATE TABLE IF NOT EXISTS moneypools ("
    "id INT PRIMARY KEY AUTOINCREMENT NOT NULL, "
    "pool_name VARCHAR(50),"                 //Statements for moneypool table which user will set himself in finance_setup()
    "initial_balance DECIMAL(10,2),"
    "user_id INT, FOREIGN KEY (user_id) REFERENCES users(id));";


    if(sqlite3_exec(mdb, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &err)  != SQLITE_OK){   //To enable foreign keys and to check if any error
        cerr << err << endl;
        return;
    }       
    if(sqlite3_exec(mdb, user_stmt.c_str(), nullptr, nullptr, &err) != SQLITE_OK){      //To create USER TABLE and to check for any errors
        cerr << err << endl;
        return;
    }
    if(sqlite3_exec(mdb, moneypool_stmt.c_str(), nullptr, nullptr, &err) != SQLITE_OK){      //To create MONEYPOOL TABLE and to check for any errors
        cerr << err << endl;
        return;
    }
    
    return;
}

void init_usertable(sqlite3* mdb){
    return;
}

//To setup user accounts and register them on the USER TABLE
//(Confirm if you want assign finance database here the moment user is registered on USER udb or assign it later)

void account_setup(sqlite3* mdb){
    return;
}

//Will finance table which will be ASSIGNED to the USER TABLE using foreign key

void init_financetable(sqlite3* mdb){
    return;
}

//To setup the init_fiancne table (just like account_setup())

void finance_setup(sqlite3* mdb){
    return;
}

//This is to register user onto the USER TABLE and create his FINANCE TABLE USING ABOVE FUNCTIONS

void signup_UI(){
    return;
}

//This will only pull data from the USER TABLE and FINANCE TABLE and work with FUNCTIONS BELOW

void login_UI(){
    return;
}

//Enters here when login_UI() is successful
//Will add entries based off money_pool types 

void menu_UI(){
    return;
}

//To add items in the predefined slots of finance table based off which user is using
//Will be added to finance table grouped using USER_ID in USER TABLE as foreign key

void add_item(sqlite3* mdb){
    return;
}

//To remove items in order to handle clutter or the ones which user doesn't want to see
//(Sort of pointless as user is using this to keep TRACK but ehh better safe than sorry)
//Will be added to finance table grouped using USER_ID in USER TABLE as foreign key

void remove_item(sqlite3* mdb){
    return;
}

//Filters through the finance table based off timestamps showing data based off last week, last year, etc
//Uses the USER_ID from USER TABLE as foreign key ofc

void filter_bytime(sqlite3* mdb){
    return;
}

//Filters through udb based off category (Can include other miscellaneous metrics like date, amount withdrawn etc)
//*EXTRA NOTE* 
//(Make the search functions based off seperate utilities e.g based off some amount withdrawn like showing all activities where amount withdrawn > 10,000)
//Also decide if you want to make this into ONE FUNCTION or seperate functions like filter_bytime(), search_bycategory, search_bycash() 
//Better to make it as seperate functions so you can make several filtering methods when making GUI of this project

void search_bycategory(sqlite3* mdb){
    return;
}

//Filters based off cash ranges like amount >10,000 withdrawn etc

void search_bycash(sqlite3* mdb){
    return;
}

//This includes the moneypool types and will search based off above 3 search_types

void search_UI(){
    return;
}

int main(){
    sqlite3* mdb = nullptr;
    sqlite3_open("main.db", &mdb);

}


