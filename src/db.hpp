/*
    Database file functions
    For the file format documentaton see INFO.txt
*/
#ifndef __DB_HPP
#define __DB_HPP
#include <string>

#define DB_KD_ROUNDS  (500)
#define DB_AES_KEYLEN  (32)
#define DB_KDSALT_LEN  (32)
#define DB_AESIV_LEN   (12)
#define DB_HMAC_LEN    (32)

class db
{
    bool loaded;
    bool decrypted;
    const char* errmsg;
    // Version
    unsigned char major;
    unsigned char minor;
    // Key derivation salt
    unsigned char kdsalt[DB_KDSALT_LEN];
    // AES counter mode initialization vector
    unsigned char aesiv[DB_AESIV_LEN];
    // HMAC of encrypted file
    unsigned char hmac[DB_HMAC_LEN];
    // Data and password
    uint32_t data_len;
    char* data;
    size_t pw_len;
    char* pw;
    
    public:
    
    enum status {
        OK,                  // Successfully loaded
        ERR_IO,              // File IO error
        ERR_FORMAT,          // Wrong file format
        ERR_VERSION,         // Version mismatch
        ERR_NO_DATA,         // No file has been loaded and no data was set
        ERR_PBKDFERR,        // Password based key derivation function failed
        ERR_DECRYPTED,       // The file should be encrypted yet is decrypted
        ERR_ENCRYPTED,       // The file should be decrypted yet is encrypted
        ERR_NO_PASSWORD,     // No password has been set yet
        ERR_HMAC_MISMATCH,   // HMACs don't match (wrong key/password or corrupted file)
    };
    
    // Get textual description of what went wrong in the last operation.
    std::string get_error() const;
    
    // Default constructor: Empty file.
    db();
    
    // Destructor: Delete data if a file is loaded.
    ~db();
    
    // Load and parse database file.
    status load(const char* filename);
    
    // Get the plaintext (empty if no plaintext there)
    std::string get_plaintext() const;
    
    // Clean old data and copy given plaintext
    void set_plaintext(const char* plaintext, size_t len);
    
    // Make a copy of the given passwort for internal use.
    void set_password(const char* password, size_t len);
    
    // Decrypt internal data (ciphertext).
    status decrypt();
    
    // Generate random AES IV, PBKDF salt, encrypt internal data (plaintext) and calculate the new HMAC.s
    status encrypt();
    
    // Write the encrypted data to the given file.
    status write(const char* filename);
    
    private:
    
    // Uses the stored password to derive a key using a Password Based Key Derivation Function.
    status derive_key(unsigned char* key);
    
    // Computes a HMAC with the given key and checks if it matches the internal one.
    status check_hmac(unsigned char* key);
    
    // Run AES in counter mode with the given key and initialization vector on data.
    // This goes both ways: It encrypts the plaintext and decrypts the ciphertext.
    void aes_ctr(unsigned char* key);
    
    // Cleanup memory
    void delete_data();
    void delete_pw();
};

#endif