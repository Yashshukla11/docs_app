#include "utils/Crypto.h"
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <stdexcept>

std::string Crypto::generateSalt()
{
    unsigned char salt[16];
    if (RAND_bytes(salt, sizeof(salt)) != 1)
    {
        throw std::runtime_error("Failed to generate salt");
    }
    
    std::ostringstream oss;
    oss << std::hex;
    for (int i = 0; i < 16; ++i)
    {
        oss << std::setw(2) << std::setfill('0') << static_cast<int>(salt[i]);
    }
    return oss.str();
}

std::string Crypto::sha256(const std::string& input)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input.c_str(), input.length());
    SHA256_Final(hash, &sha256);
    
    std::ostringstream oss;
    oss << std::hex;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
    {
        oss << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return oss.str();
}

std::string Crypto::hashPassword(const std::string& password)
{
    // Generate salt
    std::string salt = generateSalt();
    
    // Hash password + salt (multiple iterations for security)
    std::string hash = password + salt;
    for (int i = 0; i < 10000; ++i) // 10k iterations
    {
        hash = sha256(hash);
    }
    
    // Return format: salt:hash
    return salt + ":" + hash;
}

bool Crypto::verifyPassword(const std::string& password, const std::string& stored_hash)
{
    // Extract salt and hash from stored format (salt:hash)
    size_t colon_pos = stored_hash.find(':');
    if (colon_pos == std::string::npos)
    {
        return false;
    }
    
    std::string salt = stored_hash.substr(0, colon_pos);
    std::string stored_hash_value = stored_hash.substr(colon_pos + 1);
    
    // Hash password with same salt
    std::string hash = password + salt;
    for (int i = 0; i < 10000; ++i)
    {
        hash = sha256(hash);
    }
    
    return hash == stored_hash_value;
}

