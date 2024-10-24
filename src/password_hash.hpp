#ifndef PASSWORD_HASH_HPP
#define PASSWORD_HASH_HPP
#include <iomanip>
#include <iostream>
#include <sstream>
#include <windows.h>
#include <bcrypt.h>

//#pragma comment(lib, "bcrypt.lib")

using namespace std;

/**
 * @brief Hashes a password using the SHA-256 algorithm.
 *
 * This function takes a password string, applies the SHA-256 hashing algorithm using
 * Windows Cryptography API: Next Generation (CNG), and returns the result as a
 * hexadecimal string. It ensures secure handling of the cryptographic hash operation.
 *
 * @param password The input password as a string to be hashed.
 * @return A string representing the hashed password in hexadecimal format.
 */


string HashPassword(const string& password) {
    BCRYPT_ALG_HANDLE hAlgorithm = nullptr; // Algorithm handle for SHA-256
    BCRYPT_HASH_HANDLE hHash = nullptr;     // Hash handle for the hashing process
    DWORD cbHash = 32;                      // SHA-256 produces a 32-byte (256-bit) hash
    //DWORD cbData = 0;  Not used in this context, placeholder for required size
    PBYTE pbHash = new BYTE[cbHash];        // Allocate buffer to store the raw hash result
    string hashStr;                         // String to store the final hexadecimal representation of the hash

    // Open an algorithm handler for SHA256 hashing
    if (BCryptOpenAlgorithmProvider(&hAlgorithm, BCRYPT_SHA256_ALGORITHM, NULL, 0) == 0) {

        // Create a hash handle
        if (BCryptCreateHash(hAlgorithm, &hHash, NULL, 0, NULL, 0, 0) == 0) {

            // Hash the data (password)
            BCryptHashData(hHash, (PBYTE)password.c_str(), password.size(), 0);

            // Get the result of the hash
            BCryptFinishHash(hHash, pbHash, cbHash, 0);

            // Convert the hash to a hexadecimal string
            for (DWORD i = 0; i < cbHash; i++) {
                char buf[3];                        // Buffer to store the hexadecimal representation of each byte
                sprintf_s(buf, "%02x", pbHash[i]);  // Format each byte as a two-digit hexadecimal value
                hashStr += buf;                     // Append the formatted string to the final result
            }

            // Destroy the hash handler
            BCryptDestroyHash(hHash);
        }
        // Close the algorithm handler
        BCryptCloseAlgorithmProvider(hAlgorithm, 0);
    }

    // Cleanup: Free the allocated buffer memory for the hash
    delete[] pbHash;

    // Return the hexadecimal representation of the hashed password
    return hashStr;
}

#endif //PASSWORD_HASH_HPP
