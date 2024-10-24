#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <regex> 
#include "user_auth.hpp"

#ifdef max // Check if 'max' is defined
#undef max // Undefine it if it is a macro
#endif

using namespace std;

SessionMonitor sessionMonitor;      //Create the session monitor class globally

/*Checks if the email is in valid email format*/
bool isValidEmail(const string& email) {
    const regex pattern(R"((\w+)(\.{1}\w+)*@(\w+)(\.\w+)+)");
    return regex_match(email, pattern);
}

/*Checks if a passoword is at least 8 characters long, contains at lest one upper, one lower and one numeric characters*/
bool isValidPassword(const string& password) {
    const regex passwordPattern("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d).{8,}$"); 
    return regex_match(password, passwordPattern);
}


/**
 * Clears the terminal screen based on the operating system.
 * 
 * If the code is running on a Windows system, it will use the "cls" command.
 * If running on a Unix-like system (Linux, macOS, etc.), it will use the "clear" command.
 */

void clearScreen() {
#ifdef _WIN32
    system("cls"); // Use the "cls" command to clear the screen on Windows
#else
    system("clear"); // Use the "clear" command to clear the screen on Unix-like systems
#endif
}


/**
 * Pauses the program execution until the user presses Enter.
 * 
 * This function first clears the input buffer to discard any remaining characters,
 * then waits for the user to press the Enter key.
 */ 

void pause() {
#ifdef _WIN32
    system("pause");  // Only on Windows
#else
    cin.get();  // For other platforms
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Press Enter to continue...";
#endif
}

/*
 * Function: postLoginOptions
 * 
 * This function provides a menu with post-login options for the user, such as logging out, deleting their account, or returning to the main menu.
 * It runs in a loop, allowing the user to repeatedly make a selection until they either log out, delete their account, or exit to the main menu.
 *
 * Parameters:
 *  - user: Reference to the UserManager class that manages user actions (e.g., log out, delete account).
 *  - username: The username of the logged-in user.
 */

void postLoginOptions(UserManager& user, const string& username) {

    // Clear the screen before showing the menu
    clearScreen();

    // Loop to keep displaying the menu until the user chooses to log out or delete their account
    while(true) {
         
        clearScreen();  // Clear the screen for each menu display
        int choice;

        // Display the post-login options menu
        cout << "===========================================================================" << endl;
        cout << "IF IT WAS A NORMAL APPLICATION BY LOGING IN, HERE YOU WOULD SEE CONTENT. " << endl;
        cout << "===========================================================================" << endl;

        cout << "But for now here are your options: " << endl;
        cout << "============================================" << endl;
        cout << "[1] Logout" << endl;
        cout << "[2] Delete Account" << endl;
        cout << "[0] Return to Main Menu" << endl;
        cout << "============================================" << endl;

        cout << "Enter you choice." << endl;
        cin >> choice;

        // Handle invalid input
        if(cin.fail()){
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "invalid input. Please enter a valid number" << endl;
            pause();
            continue;   // Go back to display the menu again
        }
        pause();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        // Switch case to handle user input
        switch(choice) {

            // Case 1: Logout the user
            case 1:
                if(user.logOutUser(username)) {
                    cout << "Logout successfull!" << endl;
                } else {
                    cerr << "Error during logout." << endl;
                }

                pause();
                return;

            // Case 2: Delete the user account
            case 2:
                char confirm;
                // Loop to confirm the deletion of the user account
                do{
                    cout << "Are you sure you want to delete your account ? (Y/N)" << endl;
                    cin >> confirm;
                    confirm = tolower(confirm);

                    if(confirm == 'y') {
                        if(user.deleteUser(username)) {
                            cout << "Account deleted successfully." << endl;
                            pause();
                            return;
                        }
                    }

                    if(confirm == 'n') {
                        cout << "Account deleting cancelled." << endl;
                        pause();
                        break;
                    } 
                    else {
                        cout << "Invalid option. Please enter 'Y' or 'N'." << endl;
                    }
                } while(confirm != 'y' || confirm != 'n');      // Repeat until valid input is provided

            // Case 0: Return to the main menu
            case 0:
                return;
            // Default case: Handle invalid input
            default:
                cout << "Invalid choice. Please try again." << endl;
                pause();
                continue;
        }
    }
}

/*
 * Function: main
 * 
 * This is the entry point of the program. It starts the session monitor, provides options for user registration, login, 
 * password reset, and allows for account management actions like logging out or deleting the account after login.
 * The loop continues until the user decides to exit the program.
 */

int main() {

    //Starting the session monitor process
    sessionMonitor.start();

    clearScreen();  // Clear the console before showing the introduction message

    cout << endl;
    cout <<" * New here? REGISTER! (or sign in if you are already a user) * " << endl;
    pause(); //Wait for user to press Enter

    // Main loop that displays the menu options until the user decides to exit
    while(true) {

        clearScreen();  

        UserManager user;   // Create an instance of UserManager to handle user action
        int choice;

        // Display the main menu with options
        cout << endl;
        cout << "============================================" << endl;
        cout << "PLEASE CHOOSE AND OPTION" << endl;
        cout << "============================================" << endl;
        cout << "[1] Register" << endl;
        cout << "[2] Login" << endl;
        cout << "[3] Forgot Password" << endl;
        cout << "[0] Exit" << endl;
        cout << "============================================" << endl;

        cout << "Enter your choice: ";
        cin >> choice;

        // Handle invalid input (non-numeric values)
        if(cin.fail()){
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "invalid input. Please enter a valid number." << endl;
            pause();
            continue;       // Return to the start of the loop to display the menu again
        }
        pause();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        // Define variables used for registration, login, or password reset
        string username, first_name, last_name, email, password, confirmPassword, newPassword, confirmNewPassword;

        // Switch case to handle user actions based on their menu choice
        switch(choice){

            // Case 1: Register a new user
            case 1:

                clearScreen();

                do {
                cout << "Username(max 50 characters): ";
                getline(cin, username);
                if(username.length() > 50) {
                    cerr << "Username is too long. Enter a username no longer than 50 characters." << endl;
                }
                }while(username.length() > 50);
                cout << endl;


                do{
                cout << "First Name(max 50 characters): ";
                getline(cin, first_name);
                if(username.length() > 50) {
                    cerr << "First Name is too long. Enter a First Name no longer than 50 characters." << endl;
                }
                }while(first_name.length() > 50);
                cout << endl;


                do{
                cout << "Last Name(max 50 characters): ";
                getline(cin, last_name);
                if(last_name.length() > 50) {
                    cerr << "Last Name is too long. Enter a Last Name no longer than 50 characters." << endl;
                }
                }while(last_name.length() > 50);
                cout << endl;


                while(true) {
                do {
                cout << "Email(max 100 characters): ";
                getline(cin, email);
                if(email.length() > 100) {
                    cerr << "Email is too long. Enter an email no longer than 100 characters." << endl;
                }
                }while(email.length() > 100);
                cout << endl;
                    if(!isValidEmail(email)) {
                        cerr << "Invalid email format" << endl;
                    } else break;
                }


                while(true) {
                do{
                cout << "Password: ";
                getline(cin, password);
                if(password.length() > 20) {
                    cerr << "Password must be maximum 20 characters long." << endl;
                }
                }while(password.length() > 20); 
                cout << endl;
                if(!isValidPassword(password)) {
                    cerr << "Password must be at least 8 characters long, but no longer than 20 characters. Must contain at least one upper case, one lower case and one numeric characters." << endl;
                    }else break;
                }

                cout << "Confirm Password: ";
                getline(cin, confirmPassword);
                cout << endl;

                 // Call the registerUser function to complete the registration process
                user.registerUser(username, first_name, last_name, email, password, confirmPassword);
                pause();
                break;

             // Case 2: User login
            case 2:

                clearScreen();

                cout << "Username/email: ";
                getline(cin, username);
                cout << endl;

                cout << "Password: ";
                getline(cin, password);   
                cout << endl;

                // Attempt to log in the user and display post-login options if successful
                if(user.loginUser(username, password)) {

                    postLoginOptions(user, username);

                } else {
                    cout << "Login failed. Invalid username or password" << endl;
                }

                pause();
                break;

            // Case 3: Forgot password process
            case 3:

                clearScreen();

                while(true) {
                cout << "Type in your email, where we can send the reset link: ";
                getline(cin, email);
                cout << endl;
                 if(!isValidEmail(email)) {
                        cerr << "Invalid email format" << endl;
                    } else break;
                }

                // Trigger the forgotPassword process and handle password reset
                if(user.forgotPassword(email)) {
                    cout << "*** You are required to change password *** " << endl;

                    while(true){
                    do{
                    cout << "Enter new password: ";
                    getline(cin, newPassword);
                    if(newPassword.length() > 20) {
                        cerr << "Password must be maximum 20 characters long." << endl;
                    }
                    }while(newPassword.length() > 20);
                    if(!isValidPassword(newPassword)){
                        cerr << "Password must be at least 8 characters long, but no longer than 20 characters. Must contain at least one upper case, one lower case and one numeric characters." << endl;
                        }else break;
                    }


                    cout << "Confirm new password: ";
                    getline(cin, confirmNewPassword);

                    user.passwordResetRequired(email, newPassword, confirmNewPassword);

                } else {
                    cerr << "Error. Email address not found." << endl;
                }
                pause();
                break;

            // Case 0: Exit the program
            case 0:

                clearScreen();
                cout << endl;
                cout << "Exiting program. Good bye!" << endl;
                //stop monitoring sessions
                sessionMonitor.stop();
                return 0;

            // Default case: Handle invalid menu selection
            default:

                cout << "Invalid choice. Please try again." << endl;
                pause();
                continue;

        }

    }

    return 0;   // Return 0 to indicate successful execution of the program
}