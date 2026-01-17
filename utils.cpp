#include <iostream>
#include <openssl/evp.h>

#include "utils.h"

using namespace std;

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

template<> // We are essentially handling the case when T is string differently
string input<string>(const char *prompt) {
    string result;
    cout << prompt;
    getline(cin, result);

    return result;
}

template int input<int>(const char*);
template double input<double>(const char*);
template char input<char>(const char*);

void calc_hash(std::string msg, unsigned char *hash, unsigned int *hash_len) {
    EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();

    EVP_DigestInit_ex(md_ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(md_ctx, msg.c_str(), msg.size());
    EVP_DigestFinal_ex(md_ctx, hash, hash_len);

    EVP_MD_CTX_free(md_ctx);
}
