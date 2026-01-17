#ifndef UTILS_H_
#define UTILS_H_

#include <iostream>
#include <limits>

#include "utils.h"

using namespace std;

const long long int MAXIGNORE = numeric_limits<streamsize>::max();

/*
 *
 */
void clear_screen();

/*
 *
 */
void calc_hash(std::string msg, unsigned char *hash, unsigned int *hash_len);

/*
 *
 */
template <typename T> // This bad boy is covering every type of input in one function.
T input(const char *prompt);


#endif
