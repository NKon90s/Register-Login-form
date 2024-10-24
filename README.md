# User Authentication and Session Management

## Overview

This project provides a user authentication system that includes functionalities for user registration, login, logout, and session management. It also implements automatic session logout for expired sessions and manages password resets. The system is designed to be secure, scalable, and efficient.

Not that this project is for educational purposes only. 

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
); '''

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
); '''


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
); '''

### Alternations done to User Table 

Additional column added to monitor password resets:

```sql
ALTER TABLE users ADD COLUMN password_reset_required BOOLEAN DEFAULT FALSE; '''

### Alterations done to User Sessions Table

```sql
ALTER TABLE user_sessions ADD COLUMN end_session_at TIMESTAMP DEFAULT NULL; '''

## Installation

-Clone the repository

'''bash
git clone https://https://github.com/NKon90s/Register_Login_form
cd Register_Login_form '''

-Set up the database
    - Create a new database using your preferred SQL database management system (MySQL, SQLite, PostgreSQL etc.)
    -This project is done via MySQL
    -Run the provided SQL schema to create necessary tables. 

-If it is necessary use APIs to connect you code with your SQL database. In this case I used MySQL Connector C++ 9.0.
-Compile the code 
    -Make sure you have the required libraries for database connectivity
    -Compile the C++ files using your preferred compiler or with a compiler that is compatible with your API. I used MSVC for MySQL Connector. 


## Usage

-Register a User: Use the registration function to create a new user account.
-Log In: Authenticate users with the login function.
-Log Out: Users can log out, which will mark their session as ended.
-Session Monitoring: The SessionMonitor class runs in the background to automatically log out expired sessions.
-Password Reset: Initiate a password reset flow by generating a reset token.
