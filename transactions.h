#ifndef TRANSACTIONS_H_
#define TRANSACTIONS_H_

void add_transaction(sqlite3* mdb, int user_id, int moneypool_id);
void edit_transaction(sqlite3* mdb, int user_id, int moneypool_id, vector<int> transaction_ids);

#endif
