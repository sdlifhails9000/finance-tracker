#include <sqlite3.h>

#include "filters.h"

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

