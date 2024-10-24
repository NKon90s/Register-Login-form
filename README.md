# User Authentication and Session Management

## Overview

This project provides a user authentication system that includes functionalities for user registration, login, logout, and session management. It also implements automatic session logout for expired sessions and manages password resets. The system is designed to be secure, scalable, and efficient.

**Note** that this project is for educational purposes only. 

## Features

- User registration and management
- Secure password hashing
- Session management with automatic expiration
- Password reset functionality
- Logging out of expired sessions

## Database Schema

The project utilizes three main tables to manage user data, sessions, and password resets. Below is the detailed schema for each table:

### 1. Users Table

Stores user information including username, email, and hashed password.

```sql
CREATE TABLE users (
    user_id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    first_name VARCHAR(50) NOT NULL,
    last_name VARCHAR(50) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);
```

### 2. Users Session Table

Tracks user sessions, including session tokens and expiration times.

```sql
CREATE TABLE user_sessions(
    session_id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT,
    session_token VARCHAR(255) NOT NULL,
    expires_at TIMESTAMP NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    ip_address VARCHAR(45),
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
);
```

### 3. Password Resets Table

Manages password reset requests, including reset tokens and expiration times.

```sql
CREATE TABLE password_resets (
    reset_id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT,
    reset_token VARCHAR(255) NOT NULL,
    expires_at TIMESTAMP NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
);
```

### Alternations done to User Table 

Additional column added to monitor password resets:

```sql
ALTER TABLE users ADD COLUMN password_reset_required BOOLEAN DEFAULT FALSE;
```

### Alterations done to User Sessions Table

Additional column added to monitor session endings:

```sql
ALTER TABLE user_sessions ADD COLUMN end_session_at TIMESTAMP DEFAULT NULL;
```

## Installation

- **Clone the repository**

```bash
git clone https://https://github.com/NKon90s/Register_Login_form
cd Register_Login_form
```

- **Set up the database**
    - Create a new database using your preferred SQL Database Management System (MySQL, SQLite, PostgreSQL etc.)
    - This project is done via MySQL and C++
    - Run the provided SQL schema to create necessary tables. 
    - If it is necessary use APIs to connect you code with your SQL database. In this case I used MySQL Connector C++ 9.0.

- **Compile the code**
    - Make sure you have the required libraries for database connectivity
    - Compile the C++ files using your preferred compiler or with a compiler that is compatible with your API. This project applied MSVC compiler for MySQL Connector. 

- **Compiling with MSVC**
  - **1.Open the Developer Command Prompt for Visual Studio**
    - You can find this in the Start Menu under Visual Studio folder.
    
  - **2.Navigate to Your Project Directory**
    - Use the  `cd` command to change directories where you saved this project.

    ```bash
    cd path\to\this\project
    ```
  - **3.Compile the C++ files**

    ```bash
    c1 /EHsc /Fe:YourProgramName.exe main.cpp user_auth.cpp
    ```
  - **4.Run the Compiled Program**

    ```bash
    YourProgramName.exe
    ```

## Known Issues

If you are using MySQL Connector C++, then please note that the API is optimized for MSVC Compiler. It works best with that. 
In case of MySQL Connector C++ 9.0 the gcc compiler does not recognizes certain neccesary libraries needed to create a connection to the 
SQL server. Also if you are using MySQL connector, then include the mysqlcppconn.dll file in your library. It didn't work without this step for me.


## Usage

- **Register a User:** Use the registration function to create a new user account.
- **Log In:** Authenticate users with the login function.
- **Log Out:** Users can log out, which will mark their session as ended.
- **Session Monitoring:** The SessionMonitor class runs in the background to automatically log out expired sessions.
- **Password Reset:** Initiate a password reset flow by generating a reset token.

## Contributing

If you would like to contribute to this project, please fork the repository and create a pull request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- The design of this system is inspired by basic C++ console applications.
- Thanks to the C++ community for valuable resources and tutorials.
- Also thanks to OpenAI for valuable help and suggestions.
