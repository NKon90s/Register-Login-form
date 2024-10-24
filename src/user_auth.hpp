#ifndef USER_AUTH_HPP
#define USER_AUTH_HPP
#include "db_connection.hpp"
#include "password_hash.hpp"
#include "tokengenerator.hpp"
#include <iostream>
#include <ctime>
#include <chrono>
#include <memory>
#include <limits>
#include <string>
#include <thread>
#include <mutex>
#include <vector>

using namespace std;

/**
 * @class UserManager
 * @brief This class manages user-related functionality including user registration, login, session tracking, password resets, 
 * and user account deletion. It interacts with the database and handles various user operations while ensuring 
 * proper handling of sensitive information such as passwords and sessions.
 * 
 * The class handles database transactions, input validation, and rollback mechanisms to ensure the consistency 
 * of the system. It also uses secure hashing techniques for password management and secure session token generation 
 * for managing user sessions.
 */

class UserManager {

    private:

        /**
         * @brief Performs a constant-time comparison between two strings.
         * 
         * This function compares two strings in constant time to prevent timing attacks.
         * It ensures that the time taken to compare the strings does not vary based on
         * how early a difference is found, which can be exploited by attackers.
         * 
         * @param a The first string to compare.
         * @param b The second string to compare.
         * @return true If the strings are equal.
         * @return false If the strings are not equal.
         */

        bool constantTimeComparison(const string& a, const string& b);


        // Mock function to simulate sending an email with the reset token
        void sendPasswordResetEmail(const string& email, const string& resetToken);



        //Mock function to simulate IP address fetching. 
        string getIPAddressFromRequest(); 


    public:

        //Default Constructor
        UserManager() {

        }

        //Default Destructor
        ~UserManager() {

        }  

        /**
        * @brief Creating a new user by inserting their details (username, first name, last name, email, and hashed password) into the database.
        * The function checks if the password matches its confirmation, hashes the password, and stores the user information securely in the 'users' table.
        * If the registration fails due to database issues or validation problems, the process is rolled back, and the function returns false.
        * 
        *  - @param username: The desired username for the new user.
        *  - @param first_name: User's first name.
        *  - @param last_name: User's last name.
        *  - @param email: User's email address.
        *  - @param password: The password entered by the user (to be hashed before storage).
        *  - @param confirmPassword: Confirmation of the password entered by the user.
        * 
        *  - @return true: If the user is registered successfully.
        *  - @return false: If registration fails due to a password mismatch or database-related errors.
        */

        bool registerUser(const string& username, const string& first_name, const string& last_name, const string& email, const string& password, const string& confirmPassword); 

        


        /**
         * @brief Tracks user sessions for login and session expiration management.
         * 
         * This function tracks user sessions by checking for active sessions in the database
         * and ensuring no duplicate sessions are open. If the session has expired, it will log
         * the user out and end the session. If no active session exists or the session has expired,
         * a new session is created.
         * 
         * @param username The username of the user to track.
         * @return true If the session is successfully tracked or a new session is created.
         * @return false If there is an error or the session has expired.
         */
 
        bool trackUserSessions(const string&username);




        /**
         * @brief Authenticates a user by validating their username or email and password.
         * 
         * This function performs the following steps:
         * - Connects to the database.
         * - Fetches the user details (user ID, hashed password, and password reset requirement) 
         *   based on the provided username or email.
         * - Compares the input password (after hashing) with the stored password hash using a 
         *   constant-time comparison to prevent timing attacks.
         * - Tracks the user's session, ensuring no expired sessions remain.
         * 
         * If the login succeeds, it returns true; otherwise, it returns false.
         * 
         * @param username The username or email of the user attempting to log in.
         * @param password The plain text password provided by the user.
         * @return true If the login is successful and session tracking succeeds.
         * @return false If the username, password is invalid, or any error occurs during login.
         */


        bool loginUser(const string& username, const string& password); 




        /**
         * @brief Deletes a user from the database based on the username.
         * 
         * This function performs the following steps:
         * - Establishes a database connection.
         * - Retrieves the user's ID based on the provided username.
         * - If the user exists, deletes the user from the `users` table.
         * - Ensures that the operation is transactional (commits changes or rolls back in case of errors).
         * 
         * @param username The username of the user to be deleted.
         * @return true If the user is successfully deleted.
         * @return false If the user is not found or an error occurs during the process.
         */

        bool deleteUser(const string& username); 


  
    /**
     * @brief Initiates the password reset process for a user by generating a reset token.
     * 
     * This function performs the following steps:
     * - Establishes a database connection.
     * - Validates if the provided email belongs to a registered user.
     * - Generates a password reset token with an expiration time.
     * - Inserts the reset token and expiration time into the `password_resets` table.
     * - Marks the user as requiring a password reset by updating the `users` table.
     * - Sends a password reset email to the user with the generated reset token.
     * 
     * @param email The email address of the user requesting a password reset.
     * @return true If the password reset process was initiated successfully.
     * @return false If the user is not found or an error occurs during the process.
     */

    bool forgotPassword(const string& email); 




    /**
     * @brief Resets the user's password if a reset is required.
     * 
     * This function performs the following:
     * - Validates the provided email address.
     * - Verifies that the new password and its confirmation match.
     * - Hashes the new password and updates it in the database.
     * - Marks the user as no longer requiring a password reset.
     * - Commits the transaction and handles any errors during the process.
     * 
     * @param email The user's email address for which the password needs to be reset.
     * @param newPassword The new password provided by the user.
     * @param confirmNewPassword The confirmation of the new password.
     * @return true If the password reset is successful.
     * @return false If the reset fails due to validation errors or database issues.
     */

    bool passwordResetRequired(const string& email, const string& newPassword, const string& confirmNewPassword); 




    /**
     * @brief Logs out a user by ending their active session.
     * 
     * This function performs the following:
     * - Fetches the user's active session based on their username.
     * - Marks the session as ended by setting the `end_session_at` timestamp.
     * - Commits the transaction to the database.
     * - Handles any potential errors during the process.
     * 
     * @param username The username of the user who wishes to log out.
     * @return true If the logout process is successful.
     * @return false If the logout process fails due to errors such as missing sessions or database issues.
     */

    bool logOutUser(const string& username); 
};






/**
 * @class SessionMonitor
 * @brief Monitors user sessions and logs out expired sessions automatically.
 * 
 * The `SessionMonitor` class runs a background thread that periodically checks the database for expired
 * user sessions. If a session is expired but not ended, it automatically logs out the user by updating
 * the session's `end_session_at` timestamp.
 */

class SessionMonitor {

    private:
        thread monitorThread;   ///< Thread responsible for running the session monitor in the background.
        bool stopMonitoring;    ///< Flag to signal when to stop monitoring sessions.
       // mutex monitorMutex;     ///< Mutex to protect shared data (if necessary).

        // Logs out the session based on session token
        void logOutSession(const string& sessionToken); 

    public:

        /**
         * @brief Default constructor initializes the stop flag.
         */
        SessionMonitor() : stopMonitoring(false) {}

        /**
         * @brief Starts the session monitor in a separate thread.
         * 
         * This function launches the `monitorSessions` method in a background thread, which periodically checks for
         * expired sessions.
         */

        void start(); 

         /**
         * @brief Stops the session monitor and joins the background thread.
         * 
         * This function signals the session monitor to stop by setting the `stopMonitoring` flag to true and joins
         * the background thread to ensure proper cleanup.
         */
        void stop(); 

        /**
         * @brief Monitors session expiration and forces logout for expired sessions.
         * 
         * This method continuously checks the database for sessions that have expired but have not been logged out.
         * If an expired session is found, it logs out the session by updating the `end_session_at` field.
         * The method runs in a loop until `stopMonitoring` is set to true, with a sleep interval of 1 minute between checks.
         */

        void monitorSessions();
};

#endif //USER_AUTH_HPP