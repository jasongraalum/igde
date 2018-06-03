//
//  igde_common.cpp
//  igde
//
//  Created by Jason Graalum on 5/9/18.
//  Copyright © 2018 Jason Graalum. All rights reserved.
//

#include <iomanip>
#include <sstream>
#include "igde_common.hpp"

#define INFO 1
#define WARNING 1

//
// Originally from Stack Overflow:
// https://stackoverflow.com/questions/1413445/reading-a-password-from-stdcin
//
void setstdinecho(bool enable)
{
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    if( !enable )
        tty.c_lflag &= ~ECHO;
    else
        tty.c_lflag |= ECHO;
    (void) tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}


void get_creds(char * username, char * password)
{
    printf("Username: ");
    std::cin.get(username,MAX_USERNAME_LEN,'\n');
    
    printf("Password: ");
    setstdinecho(false);
    std::cin.get(password,MAX_PASSWORD_LEN,'\n');
    setstdinecho(true);
}

// From https://stackoverflow.com/questions/5100718/integer-to-hex-string-in-c#5100745
//

std::string int_to_hex( int i, int width)
{
    std::stringstream hex_out;
    hex_out << std::setfill ('0') << std::setw(width)
    << std::hex << i;
    return hex_out.str();
}
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void warning_msg(const char *msg)
{
    if(WARNING)
    std::cout << "Warning: " << msg << std::endl;
}


void info_msg(const char *msg)
{
    if(INFO)
    std::cout <<"Info: " << msg << std::endl;
}
void error(std::string msg)
{
    error(msg.c_str());
}

void warning_msg(std::string msg)
{
    warning_msg(msg.c_str());
}


void info_msg(std::string msg)
{
    info_msg(msg.c_str());
}

