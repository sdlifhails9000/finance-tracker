#ifndef CATEGORIES_H_
#define CATEGORIES_H_

#include <vector>

#include <sqlite3.h>

void add_category(sqlite3 *mdb, int user_id);
void edit_category(sqlite3 *mdb, vector<int> category_ids);

#endif
