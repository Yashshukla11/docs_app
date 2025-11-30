#include "utils/JWT.h"
#include <sstream>
#include <iomanip>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <ctime>
#include <stdexcept>

namespace {
    const std::string SECRET_KEY = "your-secret-key-change-in-production";
    const int TOKEN_EXPIRY_HOURS = 24;
    
    std::string base64Encode(const std::string& input)
    {
        BIO* bio = BIO_new(BIO_f_base64());
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO* bmem = BIO_new(BIO_s_mem());
        bio = BIO_push(bio, bmem);
        
        BIO_write(bio, input.c_str(), input.length());
        BIO_flush(bio);
        
        BUF_MEM* bptr;
        BIO_get_mem_ptr(bio, &bptr);
        
        std::string result(bptr->data, bptr->length);
        BIO_free_all(bio);
        
        return result;
    }
    
    std::string base64Decode(const std::string& input)
    {
        BIO* bio = BIO_new(BIO_f_base64());
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO* bmem = BIO_new_mem_buf(input.c_str(), input.length());
        bio = BIO_push(bio, bmem);
        
        char buffer[1024];
        int length = BIO_read(bio, buffer, sizeof(buffer));
        BIO_free_all(bio);
        
        if (length > 0)
            return std::string(buffer, length);
        return "";
    }
    
    std::string createHMAC(const std::string& data)
    {
        unsigned char* digest = HMAC(EVP_sha256(), SECRET_KEY.c_str(), SECRET_KEY.length(),
                                     reinterpret_cast<const unsigned char*>(data.c_str()),
                                     data.length(), nullptr, nullptr);
        
        std::ostringstream oss;
        oss << std::hex;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        {
            oss << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
        }
        return oss.str();
    }
}

std::string JWT::generate(const std::string& user_id)
{
    // Header
    std::ostringstream header;
    header << R"({"alg":"HS256","typ":"JWT"})";
    std::string encodedHeader = base64Encode(header.str());
    
    // Payload
    std::time_t now = std::time(nullptr);
    std::time_t exp = now + (TOKEN_EXPIRY_HOURS * 3600);
    
    std::ostringstream payload;
    payload << R"({"user_id":")" << user_id 
            << R"(","iat":)" << now 
            << R"(,"exp":)" << exp << "}";
    std::string encodedPayload = base64Encode(payload.str());
    
    // Signature
    std::string data = encodedHeader + "." + encodedPayload;
    std::string signature = createHMAC(data);
    std::string encodedSignature = base64Encode(signature);
    
    return data + "." + encodedSignature;
}

bool JWT::verify(const std::string& token)
{
    size_t dot1 = token.find('.');
    size_t dot2 = token.find('.', dot1 + 1);
    
    if (dot1 == std::string::npos || dot2 == std::string::npos)
        return false;
    
    std::string header = token.substr(0, dot1);
    std::string payload = token.substr(dot1 + 1, dot2 - dot1 - 1);
    std::string signature = token.substr(dot2 + 1);
    
    // Verify signature
    std::string data = header + "." + payload;
    std::string expectedSignature = base64Encode(createHMAC(data));
    
    if (signature != expectedSignature)
        return false;
    
    // Decode payload to check expiration
    std::string decodedPayload = base64Decode(payload);
    // Simple expiration check (in production, parse JSON properly)
    size_t expPos = decodedPayload.find("\"exp\":");
    if (expPos != std::string::npos)
    {
        size_t expStart = decodedPayload.find_first_of("0123456789", expPos);
        size_t expEnd = decodedPayload.find_first_not_of("0123456789", expStart);
        std::string expStr = decodedPayload.substr(expStart, expEnd - expStart);
        std::time_t expTime = std::stoll(expStr);
        std::time_t now = std::time(nullptr);
        
        if (now > expTime)
            return false;
    }
    
    return true;
}

std::string JWT::verifyAndGetUserId(const std::string& token)
{
    if (!verify(token))
        return "";
    
    size_t dot1 = token.find('.');
    size_t dot2 = token.find('.', dot1 + 1);
    
    if (dot1 == std::string::npos || dot2 == std::string::npos)
        return "";
    
    std::string payload = token.substr(dot1 + 1, dot2 - dot1 - 1);
    std::string decodedPayload = base64Decode(payload);
    
    // Extract user_id from payload
    size_t userIdPos = decodedPayload.find("\"user_id\":\"");
    if (userIdPos != std::string::npos)
    {
        size_t start = userIdPos + 11; // Length of "user_id":"
        size_t end = decodedPayload.find('"', start);
        if (end != std::string::npos)
        {
            return decodedPayload.substr(start, end - start);
        }
    }
    
    return "";
}
