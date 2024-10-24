
#include "db_connection.hpp"
#include "password_hash.hpp"
#include "tokengenerator.hpp"
#include "user_auth.hpp"
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


        bool UserManager::constantTimeComparison(const string& a, const string& b) {
            // If the strings are of different lengths, return false immediately.
            if(a.size() != b.size()) return false;

            // Use a volatile variable to prevent compiler optimizations.
            volatile unsigned char result = 0;

            // Iterate over each character in the strings, performing an XOR operation.
            // This ensures that each character is compared, regardless of when a difference is found.
            for(size_t i = 0; i < a.size(); ++i){
                result |= a[i] ^ b[i];      // XOR each character and accumulate the result
            }
            return result == 0;      // Return true only if result is 0 (i.e., all characters were equal).
        }


        // Mock function to simulate sending an email with the reset token
        void UserManager::sendPasswordResetEmail(const string& email, const string& resetToken) {

        cout << "Password reset link sent to " << email << " with token: " << resetToken << endl;

        // In a real application, you'd integrate with an email service to send the reset token or the 
        // reset token would be a part of password reset link.
        }



        //Mock function to simulate IP address fetching. 
        string UserManager::getIPAddressFromRequest() {
        // This would be adapted to your app's method of getting the IP
        // For example if you are using a web server, with Crow (web server), you'd get it from `request.remote_endpoint().address()`.
        // If it is a local desktop app, you might use system libraries to fetch the IP.
        return "127.0.0.1";  // Placeholder for localhost
        }



        bool UserManager::registerUser(const string& username, const string& first_name, const string& last_name, const string& email, const string& password, const string& confirmPassword) {

            // Check if the password and confirmation password match
            if (password != confirmPassword) {
                cerr << "Passwords do not match. Please try again." << endl;
                return false;
            }

            // Establish a connection to the database
            DBConnection db;
            sql::Connection *con = db.connect();
            if(!con) return false;  // Return false if the connection fails

            //Hashing the password for secure storing
            string hashedPassword = HashPassword(password);

            try{

                // Disable auto-commit to handle transactions manually.
                con->setAutoCommit(false);

                // Prepare the SQL query to insert user data into the 'users' table
                sql::PreparedStatement *prep_stmnt;
                prep_stmnt = con->prepareStatement("INSERT INTO users (username, first_name, last_name, email, password_hash) VALUES (?, ?, ?, ?, ?)");

                // Set the values for each placeholder in the prepared statement
                prep_stmnt->setString(1, username);
                prep_stmnt->setString(2, first_name);
                prep_stmnt->setString(3, last_name);
                prep_stmnt->setString(4, email);
                prep_stmnt->setString(5, hashedPassword);

                // Execute the insert operation
                prep_stmnt->executeUpdate();

                // Commit the transaction to save the changes in the database
                con->commit(); 

                // Clean up resources after successful registration
                delete prep_stmnt;
                delete con;

                // Return true to indicate successful registration
                return true;

            }catch(sql::SQLException &error) {
                // Handle any SQL exceptions during the operation
                cerr << "Error during registration: " << error.what() << endl;
                if (con) {
                    con->rollback();  // Rollback if something goes wrong
                }
                delete con;     // Clean up and release the connection
                return false;   // Return false to indicate failure during registration
            }

        }


        bool UserManager::trackUserSessions(const string&username) {

            // Establish a connection to the database
            DBConnection db;
            sql::Connection *con = db.connect();
            if(!con) return false;  // Return false if the connection fails

            TokenGenerator tokenGen;
            string sessionToken = tokenGen.generateSessionToken();      // Generate session token

            string ip_address = getIPAddressFromRequest();              // Capture the user's IP address

            // Convert the expiration time to a thread-safe structure (platform-dependent).
            time_t now = time(nullptr);
            time_t expiresAt = now + 3 * 60 * 60;  //Sessions expiration: 1 hours later in this case, but you can change it depending on need.
            struct tm expirationTime;

            #ifdef _WIN32
                if(gmtime_s(&expirationTime, &expiresAt) != 0) {    // Thread-safe on Windows
                    cerr << "Failed to convert expiration time." << endl;
                    delete con;
                    return false;
                }
            #else
                if(gmtime_r(&expiresAt, &expirationTime) == nullptr)  { // Thread-safe on Unix-like systems
                    cerr << "Failed to convert expiration time." << endl;
                    delete con;
                    return false;
                    }
            #endif

            // Format the expiration time into a string (YYYY-MM-DD HH:MM:SS).
            char expiresAtStr[20];
            if (strftime(expiresAtStr, sizeof(expiresAtStr), "%Y-%m-%d %H:%M:%S", &expirationTime) == 0) {
                cerr << "Failed to format expiration time." << endl;
                delete con;
                return false;
            }


            try {  

                // Disable auto-commit to handle transactions manually.
                con->setAutoCommit(false);

                // Check if the user already has an active session in the database.
                sql::PreparedStatement *fetchSession;
                fetchSession = con->prepareStatement("SELECT session_token, expires_at, end_session_at FROM user_sessions WHERE user_id = (SELECT user_id FROM users WHERE username = ?) AND end_session_at IS NULL");
                fetchSession->setString(1, username);

                sql::ResultSet *res = fetchSession->executeQuery(); 

                // If there is an active session, check if it's expired.
                if(res->next()) {

                    string expiresAtDB = res->getString("expires_at");

                    // Parse the expires_at timestamp from the database.
                    struct tm tmDB = {};
                    istringstream ss(expiresAtDB);
                    ss >> get_time(&tmDB, "%Y-%m-%d %H:%M:%S");
                    
                    if(ss.fail()) {
                        cerr << "Failed to parse expires_at timestamp: " << expiresAtDB << endl;
                        // Handle parsing error appropriately
                        delete res;
                        delete fetchSession;
                        con->rollback();
                        delete con;
                        return false;   // Parsing error.
                    }

                    time_t dbExpiresAt;

                    // Convert the parsed expiration time into time_t (platform-dependent).
                    #ifdef _WIN32
                        dbExpiresAt = _mkgmtime(&tmDB); // Convert tm to time_t in UTC (Windows)
                    #else
                        dbExpiresAt = timegm(&tmDB); // Convert tm to time_t in UTC (Unix-like)
                    #endif

                    // Check if the session has expired.
                    string dbSessionToken = res->getString("session_token");

                    if(now > dbExpiresAt){
                        //Session expired, log the user out.
                        cout << "Session expired for user: " << username << endl;

                        sql::PreparedStatement *endSession;
                        endSession = con->prepareStatement("UPDATE user_sessions SET end_session_at = CURRENT_TIMESTAMP WHERE session_token = ?");
                        endSession->setString(1, dbSessionToken);

                        endSession->executeUpdate();

                        con->commit();  // Commit the session end update.

                        logOutUser(username);

                        // Cleanup resources before returning.
                        delete endSession;
                        delete res;
                        delete fetchSession;
                        delete con;
                        return false;  //Session expired

                    }

                }


                //No active session or session has expired. Start a new session (regular login)
                sql::PreparedStatement *insertSession;
                insertSession = con->prepareStatement("INSERT INTO user_sessions (user_id, session_token, expires_at, ip_address) VALUES ((SELECT user_id FROM users WHERE username = ?), ?, ?, ? )");
                insertSession->setString(1, username);
                insertSession->setString(2, sessionToken);
                insertSession->setDateTime(3, expiresAtStr);
                insertSession->setString(4, ip_address);

                insertSession->executeUpdate();
                con->commit();  // Commit the new session to the database.

                // Cleanup resources after successful session creation.
                delete insertSession;
                delete fetchSession;
                delete res;
                delete con;
                return true;


            }catch(sql::SQLException &error){
                // Handle any database exceptions by rolling back the transaction.
                cerr << "Session Error: " << error.what() << endl;
                if (con) {
                    con->rollback();  // Rollback if something goes wrong
                }
                delete con;
                return false;
            }

        }


        bool UserManager::loginUser(const string& username, const string& password) {

            // Establish a connection to the database.
            DBConnection db;
            sql::Connection *con = db.connect();
            if(!con) return false;      // Return false if the connection fails.

            try{

                // Disable auto-commit to handle transactions manually.
                con->setAutoCommit(false);

                // Prepare a statement to fetch the user's information.
                sql::PreparedStatement *fetchUser;
                fetchUser = con->prepareStatement("SELECT user_id, password_hash, password_reset_required FROM users WHERE username = ? OR email = ?");
                fetchUser->setString(1, username);
                fetchUser->setString(2, username);

                // Execute the query to retrieve user data.
                sql::ResultSet *res = fetchUser->executeQuery();

                // Check if the user exists in the database.
                if(!res->next()) {
                    cerr << "Invalid Username or Password." << endl;   
                    delete res;
                    delete fetchUser;
                    delete con;
                    return false;
                } 

                // Retrieve the user ID and stored password hash from the result.
                int user_id = res->getInt("user_id");
                string storedHash = res->getString("password_hash");

                    // Hash the input password and perform a constant-time comparison with the stored hash.
                    string hashedInputPassword = HashPassword(password);   
                    if(!constantTimeComparison(hashedInputPassword, storedHash)) {
                        cerr << "Invalid Username or Password." << endl;
                        delete res;
                        delete fetchUser;
                        delete con;
                        return false;
                    }   

                    // Check and track the user's session after successful authentication.
                     if(trackUserSessions(username)) {
                        cout << "Login Successful." << endl;
                     
                    } else {
                        cerr << "Unknown Error during Login." << endl;
                        delete res;
                        delete fetchUser;
                        delete con;
                        return false;
                    }

                    // Commit the transaction after successful login and session management.
                    con->commit();

                    // Cleanup resources after the operation.
                    delete res;
                    delete fetchUser;
                    delete con;
                    return true;     // Return true for a successful login.


            } catch(sql::SQLException &error) {
                // Handle SQL exceptions by rolling back the transaction.
                cerr << "Error during Login: " << error.what() << endl;
                if(con){
                    con->rollback();     // Rollback if any error occurs.
                }
                delete con;
                return false;
            }
            
        }


        bool UserManager::deleteUser(const string& username) {

            // Establish a connection to the database.
            DBConnection db;
            sql::Connection *con = db.connect();
            if (!con) return false;     // Return false if the connection fails.

            try{

                 // Disable auto-commit to handle transactions manually.
                con->setAutoCommit(false);

                // Prepare a statement to fetch the user's ID based on the provided username.
                sql::PreparedStatement *fetchUser;
                fetchUser = con->prepareStatement("SELECT user_id FROM users WHERE username = ?");
                fetchUser->setString(1, username);  // Set the username in the query.

                // Execute the query to check if the user exists.
                sql::ResultSet *res = fetchUser->executeQuery();

                if(res->next()) {

                    // User found. Retrieve the user's ID.
                    int user_id = res->getInt("user_id");

                    // Prepare a statement to delete the user based on the retrieved user_id.
                    sql::PreparedStatement *del_user;
                    del_user = con->prepareStatement("DELETE FROM users WHERE user_id = ?");
                    del_user->setInt(1, user_id);   // Set the user_id in the delete query.

                    // Execute the delete operation.
                    del_user->executeUpdate();

                    // Commit the transaction after successfully deleting the user.
                    con->commit();

                    // Cleanup resources after the operation.
                    delete del_user;
                    delete fetchUser;
                    delete res;
                    delete con;
                    return true;    // Return true if the user is successfully deleted.
                    
                } else {
                    // Handle the case where the user is not found in the database.
                    cerr << "User not found." << username << endl;
                    // Cleanup resources if the user does not exist.
                    delete fetchUser;
                    delete res;
                    delete con;
                    return false;   // Return false if the user is not found.

                }
    

            } catch (sql::SQLException &error) {
                // Handle SQL exceptions by rolling back the transaction in case of an error.
                cerr << "Error during deleting user: " << error.what() << endl;
                if(con) {
                    con->rollback();    // Rollback the transaction if an error occurs.
                }
                delete con;
                return false;

            }
        
        }
  
    bool UserManager::forgotPassword(const string& email) {

        // Establish a connection to the database.
        DBConnection db;
        sql::Connection *con = db.connect();
        if (!con) return false;     // Return false if the connection fails.

        // Generate a password reset token.
        TokenGenerator tokenGen;
        string resetToken = tokenGen.generateSessionToken();

        // Step 3: Calculate expiration time. The code generate in UTC time but my Database is in CET, so this setup gives me 1 hour expiration time.
        time_t now = time(nullptr);
        time_t expiresAt = now + 3  * 60 * 60;  //Sessions expiration: 1 hours later in this case, but you can change it depending on need.
        struct tm expirationTime;
        #ifdef _WIN32
            gmtime_s(&expirationTime, &expiresAt); // Thread-safe on Windows
        #else
            gmtime_r(&expiresAt, &expirationTime); // Thread-safe on Unix-like systems
        #endif

        char expiresAtStr[20];
            if (strftime(expiresAtStr, sizeof(expiresAtStr), "%Y-%m-%d %H:%M:%S", &expirationTime) == 0) {
                cerr << "Failed to format expiration time." << endl;
                return false;
            }
 
        try{

            // Disable auto-commit for transactional integrity.
            con->setAutoCommit(false);

            // Fetch the user by email to validate if the user exists.
            sql::PreparedStatement *fetchUser;
            fetchUser = con->prepareStatement("SELECT user_id FROM users WHERE email = ?");
            fetchUser->setString(1, email);

            sql::ResultSet *res = fetchUser->executeQuery();

             if (!res->next()) {
                // Handle the case where the user is not found in the database.
                cerr << "User not found." << endl;
                // Cleanup resources if the user does not exist.
                delete fetchUser;
                delete res;
                delete con;
                return false;
            }

            // User found, retrieve the user ID.
            int user_id = res->getInt("user_id");

            // Insert the password reset token and expiration time into the `password_resets` table.
            sql::PreparedStatement *insertReset;
            insertReset = con->prepareStatement("INSERT INTO password_resets (user_id, reset_token, expires_at) VALUES (?, ?, ?)");
            insertReset->setInt(1, user_id);
            insertReset->setString(2, resetToken);
            insertReset->setDateTime(3, expiresAtStr);

            insertReset->executeUpdate();

            // Mark the user in the `users` table as requiring a password reset.
            sql::PreparedStatement *markUserForReset;
            markUserForReset = con->prepareStatement("UPDATE users SET password_reset_required = TRUE WHERE user_id = ?");
            markUserForReset->setInt(1, user_id);

            markUserForReset->executeUpdate();

            // Commit the transaction after successful execution.
            con->commit();

            // Send the password reset email with the reset token to the user's email.
            sendPasswordResetEmail(email, resetToken);

            // Cleanup resources after the operation.
            delete insertReset;
            delete markUserForReset;
            delete fetchUser;
            delete res;
            delete con;
            return true;    // Return true if the password reset process was successful.


        } catch(sql::SQLException &error) {
                //Handle SQL exceptions by rolling back the transaction in case of an error.
                cerr << "Error in resetting password." << error.what() << endl;
                if(con) {
                    con->rollback();       // Rollback the transaction if an error occurs.
                }
                delete con;
                return false;
        }   

    }


    bool UserManager::passwordResetRequired(const string& email, const string& newPassword, const string& confirmNewPassword) {

        // Establish a connection to the database.
        DBConnection db;
        sql::Connection *con = db.connect();
        if (!con) return false;     // Return false if connection fails.

        // Fetch the user by email to check if they exist.
        sql::PreparedStatement *fetchUser;
        fetchUser = con->prepareStatement("SELECT user_id FROM users WHERE email = ?");
        fetchUser->setString(1, email);

        sql::ResultSet *res = fetchUser->executeQuery();

        if(!res->next()) {
            // If no user is found, display an error message.
            cerr << "Invalid email." << endl;
            delete res;
            delete fetchUser;
            delete con;
            return false;   // Return false if the email is not found.
            }

        int user_id = res->getInt("user_id");   // Retrieve the user's ID.


        try{

            // Disable auto-commit for transactional integrity.
            con->setAutoCommit(false);

            // Validate that the password fields are not empty.
            if (newPassword.empty() || confirmNewPassword.empty()) {
                cerr << "Password fields cannot be empty." << endl;
                return false;
                }

            // Ensure the new password matches the confirmation.
            if(newPassword != confirmNewPassword) {
                cerr << "Passwords do not match. Try again." << endl;
                delete res;
                delete fetchUser;
                delete con;
                return false;   // Return false if the passwords don't match.
                }

            // Hash the new password before updating it in the database.
            string hashedNewPassword = HashPassword(newPassword);

            // Update the user's password and reset the password reset flag in the database.
            sql::PreparedStatement *updatePassword;
            updatePassword = con->prepareStatement("UPDATE users SET password_hash = ?, password_reset_required = FALSE WHERE user_id = ?");
            updatePassword->setString(1, hashedNewPassword);
            updatePassword->setInt(2, user_id);

            updatePassword->executeUpdate();

            // Commit the transaction to persist the changes.
            con->commit();

            cout << "Password successfully updated. Login again with your new credentials." << endl;

            // Provide a pause mechanism based on the operating system.
            #ifdef _WIN32
                system("pause");  // Only on Windows
            #else
                cin.get();  // For other platforms
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear the input buffer
                cout << "Press Enter to continue...";
            #endif

            // Cleanup resources.
            delete res;
            delete fetchUser;
            delete con;
            delete updatePassword;
            return true;    // Return true if the password reset is successful.


        }catch(sql::SQLException &error) {
             // Handle any SQL exceptions by rolling back the transaction and displaying the error.
            cerr << "Error in resetting password." << error.what() << endl;
                if(con) {
                    con->rollback();    // Rollback the transaction if an error occurs.
                }
                delete con;
                return false;
            }   

    }


    bool UserManager::logOutUser(const string& username) {

        // Establish a connection to the database.
        DBConnection db;
        sql::Connection *con = db.connect();
        if (!con) return false;     // Return false if connection fails.

        try{

            // Disable auto-commit to ensure transaction integrity.
            con->setAutoCommit(false);

            // Fetch the active session for the specified user.
            sql::PreparedStatement *fetchSession;
            fetchSession = con->prepareStatement("SELECT session_token FROM user_sessions WHERE user_id = (SELECT user_id FROM users WHERE username = ?) AND end_session_at IS NULL");
            fetchSession->setString(1, username);

            sql::ResultSet *res = fetchSession->executeQuery();

            // Check if an active session exists for the user.
            if(!res->next()) {
                cerr << "No session found for: " << username << endl;
                // Clean up resources before returning false.
                delete fetchSession;
                delete res;
                delete con;
                return false;   // Return false if no active session is found.
            }   

            // Extract the session token from the result set.
            string sessionToken = res->getString("session_token");

            // End the session by setting the `end_session_at` field to the current timestamp.
            sql::PreparedStatement *endSession;
            endSession = con->prepareStatement("UPDATE user_sessions SET end_session_at = CURRENT_TIMESTAMP WHERE session_token = ?");
            endSession->setString(1, sessionToken);

            // Execute the update and commit the transaction.
            endSession->executeUpdate();
            con->commit();      // Commit changes to ensure the session is properly closed.

            cout << "Logout successfull." << endl;

            // Clean up all resources.
            delete fetchSession;
            delete res;
            delete endSession;
            delete con;
            return true;        // Return true to indicate successful logout.


        } catch(sql::SQLException &error) {
            // Handle SQL exceptions by rolling back the transaction and logging the error.
            cerr << "Error during logout: " << error.what() << endl;
            if (con) {
                con->rollback();    // Rollback the transaction if an error occurs.
            }
            delete con;
            return false;
        }
    }



        // Logs out the session based on session token
        void SessionMonitor::logOutSession(const string& sessionToken) {

            // Establish a database connection.
            DBConnection db;
            sql::Connection* con = db.connect();
            if (!con) return;   // Return if the connection fails.


            try {
                // Prepare an SQL statement to update the session's `end_session_at` field.
                sql::PreparedStatement* endSession = con->prepareStatement(
                    "UPDATE user_sessions SET end_session_at = CURRENT_TIMESTAMP WHERE session_token = ?"
                );
                endSession->setString(1, sessionToken);

                // Execute the update to log out the session.
                endSession->executeUpdate();

                cout << "Session ended for token: " << sessionToken << endl;

                // Clean up resources.
                delete endSession;
                delete con;

            } catch (sql::SQLException& error) {
                // Handle SQL exceptions and rollback if necessary.
                cerr << "Error during session logout: " << error.what() << endl;
                if(con) {
                    con->rollback();    // Rollback the transaction in case of failure.
                }
                delete con;
            }
        }


        void SessionMonitor::start() {
            monitorThread = thread(&SessionMonitor::monitorSessions, this);
        }

  
        void SessionMonitor::stop() {
            stopMonitoring = true;
            if (monitorThread.joinable()) {
                monitorThread.join();   // Wait for the monitoring thread to finish.
            }
        }


        void SessionMonitor::monitorSessions() {
            while (!stopMonitoring) {       // Continue monitoring until the stop flag is set.

                // Establish a database connection.
                DBConnection db;
                sql::Connection* con = db.connect();
                if (!con) {
                    cerr << "Failed to connect to DB for session monitoring." << endl;
                    return; // Exit if connection fails.
                }
  
                try {

                    // Prepare an SQL statement to fetch expired sessions that are not yet ended.
                    sql::PreparedStatement* fetchExpiredSessions = con->prepareStatement("SELECT session_token, username FROM user_sessions JOIN users ON user_sessions.user_id = users.user_id WHERE expires_at < CURRENT_TIMESTAMP AND end_session_at IS NULL");

                    // Execute the query to get the list of expired sessions.
                    sql::ResultSet* res = fetchExpiredSessions->executeQuery();

                    vector<string> expiredSessions;     ///< List of session tokens for expired sessions.
                    vector<string> usernames;           ///< Corresponding usernames for expired sessions.

                    // Collect all expired sessions.
                    while (res->next()) {
                        expiredSessions.push_back(res->getString("session_token"));
                        usernames.push_back(res->getString("username"));
                    }

                    // Log out all expired sessions
                    for (size_t i = 0; i < expiredSessions.size(); ++i) {
                        cout << "Ending session for user: " << usernames[i] << endl;
                        logOutSession(expiredSessions[i]);
                    }

                    // Clean up resources.
                    delete res;
                    delete fetchExpiredSessions;
                    delete con;

                } catch (sql::SQLException& error) {
                    // Handle SQL exceptions and rollback if necessary.
                    cerr << "Error during session monitoring: " << error.what() << endl;
                    if(con) {
                        con->rollback();    // Rollback the transaction in case of failure.
                    }
                    delete con;
                }

                // Sleep for a minute before the next check
                this_thread::sleep_for(chrono::minutes(1));
            }
        }

