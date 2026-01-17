#ifndef MONEYPOOLS_H_
#define MONEYPOOLS_H_

#include <sqlite3.h>

void edit_moneypool(sqlite3 *mdb, int moneypool_id);
void add_moneypool(sqlite3* mdb, int user_id);
void view_moneypool(sqlite3* mdb, int user_id);

#endif
