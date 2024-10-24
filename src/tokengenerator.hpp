#ifndef TOKENGENERATOR_HPP
#define TOKENGENERATOR_HPP
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>

using namespace std;

/**
 * @brief A class responsible for generating secure, URL-safe session tokens.
 *
 * This class provides methods to generate random session tokens using a 
 * cryptographically secure random number generator and encode the output 
 * into a Base64-like URL-safe format. The tokens can be used for managing 
 * user sessions, authentication, or other security-related purposes.
 */


class TokenGenerator {

    private:

        /**
         * @brief Encodes raw bytes into a URL-safe Base64-like string.
         *
         * This function converts a vector of random bytes into a Base64-like 
         * URL-safe string. The Base64 character set used here replaces 
         * '+' and '/' with '-' and '_', respectively, making the encoded 
         * result safe for URLs and file paths.
         *
         * @param data A vector of unsigned bytes (raw data) to be encoded.
         * @return A string representing the URL-safe Base64 encoded data.
         */

        string toBase64UrlSafe(const vector<unsigned char>& data) {

            // Custom Base64-like character set, URL-safe (no '+', '/')
            static const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

            string base64;   // Final encoded string
            int val = 0;     // Holds accumulated bits from the input data
            int valb = -6;   // Bit buffer (initialized to -6 to trigger first shift)

            // Iterate through each byte in the input data
            for(unsigned char c : data) {
                val = (val << 8) + c;       // Shift in the byte and accumulate
                valb += 8;                  // Increase bit buffer size
            

            // Process every 6 bits into a Base64 character
            while(valb >= 8) {
                base64.push_back(base64_chars[(val >> valb) & 0x3F]);       // Extract top 6 bits
                valb -= 6;                                                  // Decrease buffer size by 6 bits
                }
            }

            // Handle any remaining bits (if valb > -6)
            if(valb > -6) 
            base64.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);

            return base64;  // Return the final Base64 URL-safe string
        }


    public:
        //default constructor for TokenGenerator class
        TokenGenerator() {

        }

         /**
         * @brief Generates a cryptographically secure random session token.
         *
         * This method generates a session token of a specified byte length, 
         * using a cryptographically secure random number generator. The raw 
         * random bytes are then encoded into a Base64-like URL-safe string.
         *
         * @param byteLength The desired length of the random token in bytes (default is 32 bytes).
         * @return A URL-safe string representing the generated session token.
         */


        string generateSessionToken(int byteLenght = 32) {

            random_device rd;                                   // Cryptographically secure random number generator
            vector<unsigned char> randomBytes(byteLenght);      // Vector to hold random bytes

            // Fill the vector with cryptographically secure random bytes
            for(int i = 0; i < byteLenght; i++) {
                randomBytes[i] = static_cast<unsigned char>(rd() & 0xFF);       // Mask to get 8 bits
            }

            // Convert the random bytes into a URL-safe Base64 encoded string
            string token = toBase64UrlSafe(randomBytes);

            return token;       // Return the generated session token

        }

};

#endif //TOKENGENERATOR_HPP