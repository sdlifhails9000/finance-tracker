#ifndef UI_H_
#define UI_H_

#include <string>
#include <sqlite3.h>

#include "users.h"

using namespace std;

void menu_UI(sqlite3* mdb, User u);
void category_UI(sqlite3* mdb, int user_id);
void transaction_UI(sqlite3* mdb, int user_id, int moneypool_id, const string& moneypool_name);
void moneypool_UI(sqlite3 *mdb, int user_id);

#endif
