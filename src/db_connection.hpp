#ifndef DB_CONNECTION_HPP
#define DB_CONNECTION_HPP
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <iostream>



using namespace std;

/**
 * @brief Establishes a connection to a MySQL database.
 *
 * This method initializes a connection to a MySQL database using the MySQL C++ connector.
 * It utilizes the MySQL Driver to connect to a local database running on the default MySQL
 * port (3306) with the provided credentials. Once connected, it sets the schema to the 
 * specified database. If an exception occurs during the connection process, it is caught 
 * and an error message is displayed. In the event of an error, the function returns `nullptr`.
 *
 * @return A pointer to the active MySQL database connection (`sql::Connection`). 
 *         Returns `nullptr` if the connection fails.
 */


class DBConnection {

    public:

    //default constructor for DBConnection class
    DBConnection() {

    }

    //default destructor for DBConnection class
    ~DBConnection() {

    }

    sql::Connection *connect() {
        try {
            // Create a connection to MySQL database
            sql::mysql::MySQL_Driver *driver;
            sql::Connection *con;

            // Initialize MySQL Driver
            driver = sql::mysql::get_mysql_driver_instance();

            // Connect to the database (replace with your MySQL credentials) 127.0.0.1 is usually the placeholder IP for local host. 3306 is usually the default port.
            con = driver->connect("tcp://127.0.0.1:3306", "root", "Qwedsa_123");

            con->setSchema("user_authentication");  // Connect to the database you created
            return con;

        } catch (sql::SQLException &error) {

            cerr << "Error connecting to database: " << error.what() << endl;   // Handle any SQL-related exceptions by logging the error message
            return nullptr;                                                     // Return nullptr to indicate that the connection attempt failed
        }   

    }

};

#endif //DB_CONNECTION_HPP