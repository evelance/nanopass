#include <string>
#include <fstream>
#include <random>
#include <stdio.h>
#include <errno.h>
#include <string.h>
extern "C" {
   #include "crypto/hmac_sha3_256.h"
   #include "crypto/aes_ctr_256.h"
   #include "crypto/pbkdf.h"
}
#include "db.hpp"
using namespace std;

static const char MAGICSTR[] = "NANOPASSDB";
#define MAGICSTR_LEN ((sizeof MAGICSTR) - 1)
static const size_t VERSION_LEN = 3;
static const size_t DATALEN_LEN = 4;
static const size_t HEADER_SIZE = MAGICSTR_LEN + VERSION_LEN + DB_KDSALT_LEN + DB_AESIV_LEN + DB_HMAC_LEN + DATALEN_LEN;
static const size_t MAX_FILESIZE = HEADER_SIZE + 0xFFFFFFFF;

typedef unsigned char uc;
static void generate_random(uc* buf, size_t len);

db::db() : decrypted(false), errmsg(NULL), data_len(0), data(NULL), pw_len(0), pw(NULL) {}

db::~db()
{
    this->delete_pw();
    this->delete_data();
}

string db::get_error() const
{
    return this->errmsg;
}

#define RETURN_COND(cond, code, msg) if (cond) { this->errmsg = (msg); return (code); }

db::status db::load(const char* filename)
{
    // Open file and check if it is a file and readable
    fstream f(filename, ios::in | ios::binary | ios::ate);
    RETURN_COND((! f.good()), ERR_IO, strerror(errno));
    
    // Check for proper file size and rewind
    streampos fsz = f.tellg();
    RETURN_COND((fsz < HEADER_SIZE) , ERR_FORMAT, "File has insufficient size (file corrupted?)");
    f.seekg(0);
    
    // Read header and check file format
    this->delete_data();
    char magic[MAGICSTR_LEN];
    char version[VERSION_LEN];
    uc dlen[DATALEN_LEN];
    f.read(magic, MAGICSTR_LEN);
    f.read(version, VERSION_LEN);
    f.read((char*)this->kdsalt, DB_KDSALT_LEN);
    f.read((char*)this->aesiv, DB_AESIV_LEN);
    f.read((char*)this->hmac, DB_HMAC_LEN);
    f.read((char*)dlen, DATALEN_LEN);
    RETURN_COND((f.bad() || f.fail()), ERR_IO, "Cannot read file header");
    RETURN_COND((memcmp(MAGICSTR, magic, MAGICSTR_LEN) != 0 || version[1] != '.'), ERR_FORMAT, "This file is not database file for this program");
    RETURN_COND((! (version[0] >= '0' && version[0] <= '9' && version[2] >= '0' && version[2] <= '9')), ERR_FORMAT, "This file is not database file for this program");
    this->major = version[0] - '0';
    this->minor = version[2] - '0';
    RETURN_COND((major != 1 || minor != 0), ERR_VERSION, "Unsupported file version");
    this->data_len = 0;
    for (size_t i = 0; i < DATALEN_LEN; ++i) {
        this->data_len <<= 8;
        this->data_len  |= dlen[i];
    }
    
    // Read encrypted data
    this->data = new char[this->data_len];
    f.read(this->data, this->data_len);
    if (f.bad() || f.fail()) {
        this->delete_data();
        this->errmsg = "Cannot read ciphertext from file (file corrupted?)";
        return ERR_IO;
    }
    this->decrypted = false;
    return OK;
}

string db::get_plaintext() const
{
    if (this->data != NULL && this->decrypted) {
        return string(this->data, this->data_len);
    }
    return "";
}

void db::set_plaintext(const char* plaintext, size_t len)
{
    // Delete old data and copy given plaintext
    this->delete_data();
    this->data_len = len;
    this->data = new char[len];
    memcpy(this->data, plaintext, len);
    this->decrypted = true;
}

db::status db::write(const char* filename)
{
    // Ensure that encrypted ciphertext exists
    RETURN_COND((this->data == NULL), ERR_NO_DATA, "No data loaded");
    
    // Encrypt if necessary
    if (this->decrypted) {
        db::status s = this->encrypt();
        if (s != OK)
            return s;
    }
    
    // Open file and check if it is a file and writable
    fstream f(filename, ios::out | ios::binary | ios::trunc);
    RETURN_COND((! f.good()), ERR_IO, strerror(errno));
    
    // Calculate length and write data
    uc dlen[DATALEN_LEN];
    for (size_t i = 0; i < DATALEN_LEN; ++i) {
        dlen[i] = (unsigned char)((this->data_len >> (((DATALEN_LEN - 1) - i) * 8)) & 0xFF);
    }
    f.write(MAGICSTR, MAGICSTR_LEN);
    f.write("1.0", VERSION_LEN);
    f.write((char*)this->kdsalt, DB_KDSALT_LEN);
    f.write((char*)this->aesiv, DB_AESIV_LEN);
    f.write((char*)this->hmac, DB_HMAC_LEN);
    f.write((char*)dlen, DATALEN_LEN);
    f.write((char*)this->data, this->data_len);
    RETURN_COND((f.bad() || f.fail()), ERR_IO, "Cannot write to file");
    return OK;
}

void db::set_password(const char* password, size_t len)
{
    // Delete old password and copy given password
    this->delete_pw();
    this->pw_len = len;
    this->pw = new char[len];
    memcpy(this->pw, password, len);
}

db::status db::decrypt()
{
    // Ensure that encrypted ciphertext exists
    RETURN_COND((this->data == NULL), ERR_NO_DATA, "No data loaded");
    RETURN_COND((this->decrypted), ERR_DECRYPTED, "The file is already decrypted");
    
    // Derive AES key from given password
    uc key[DB_AES_KEYLEN];
    db::status s = this->derive_key(key);
    if (s != OK)
        return s;
    
    // Check if it actually was used to encrypt the file
    s = this->check_hmac(key);
    if (s != OK)
        return s;
    
    // Decrypt data and clean up
    aes_ctr(key);
    memset(key, 0, sizeof key);
    this->decrypted = true;
    return OK;
}

db::status db::encrypt()
{
    // Ensure that the file is decrypted (encrypting it twice gives the plaintext).
    RETURN_COND((this->data == NULL), ERR_NO_DATA, "No data loaded");
    RETURN_COND((! this->decrypted), ERR_ENCRYPTED, "The file is already encrypted");
    
    // Generate new random PBDKF salt and AES initialization vector
    generate_random(this->kdsalt, DB_KDSALT_LEN);
    generate_random(this->aesiv, DB_AESIV_LEN);
    
    // Derive AES key from given password
    uc key[DB_AES_KEYLEN];
    db::status s = this->derive_key(key);
    if (s != OK)
        return s;
    
    // Encrypt data and calculate new HMAC
    aes_ctr(key);
    hmac_sha3_256((uc*)this->data, this->data_len, key, DB_AES_KEYLEN, this->hmac);
    memset(key, 0, sizeof key);
    this->decrypted = false;
    return OK;
}

db::status db::derive_key(uc* key)
{
    RETURN_COND(this->pw == NULL, ERR_NO_PASSWORD, "No password given");
    memset(key, 0, DB_AES_KEYLEN);
    int ret = pbkdf((uc*)this->pw, this->pw_len, (uc*)this->kdsalt, DB_KDSALT_LEN, key, DB_AES_KEYLEN, DB_KD_ROUNDS, hmac_sha3_256, 32);
    RETURN_COND((ret != 0), ERR_PBKDFERR, "Password based key derivation function failed");
    return OK;
}

db::status db::check_hmac(uc* key)
{
    uc computed_hmac[DB_HMAC_LEN];
    hmac_sha3_256((uc*)data, data_len, (uc*)key, DB_AES_KEYLEN, computed_hmac);
    RETURN_COND((memcmp(computed_hmac, this->hmac, DB_HMAC_LEN) != 0), ERR_HMAC_MISMATCH, "This file was not encrypted with the given password");
    return OK;
}

void db::aes_ctr(uc* key)
{
    aes_ctr_256((uc*)this->data, this->data_len, key, this->aesiv, (uc*)this->data);
}

void db::delete_pw()
{
    if (this->pw != NULL) {
        memset(this->pw, 0, this->pw_len);
        this->pw_len = 0;
        delete[] this->pw;
    }
}

void db::delete_data()
{
    if (this->data != NULL) {
        memset(this->data, 0, this->data_len);
        this->data_len = 0;
        delete[] this->data;
    }
}

static void generate_random(uc* buf, size_t len)
{
    random_device rd;
    for (size_t i = 0; i < len; ++i) {
        buf[i] = (uc)rd();
    }
}

