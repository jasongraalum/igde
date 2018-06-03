//
//  igde_common.hpp
//  igde
//
//  Created by Jason Graalum on 5/9/18.
//  Copyright © 2018 Jason Graalum. All rights reserved.
//

#ifndef igde_common_hpp
#define igde_common_hpp

#define TEST std::cout<<"TEST"<<std::endl;
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/event.h>

#include <errno.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "igde_protocol.h"

#define MAX_SERVER_CONNECTIONS 10
#define MAX_EDIT_SESSIONS 128
#define MAX_EDITORS 8
#define MAX_USERNAME_LEN 64
#define MAX_PASSWORD_LEN 64


#define MAX_HOSTNAME_LEN 128
#define MSG_HEADER_LEN 32
#define MAX_SEND_ATTEMPTS 3

void setstdinecho(bool);
void error(const char *);
void info_msg(const char *);
void warning_msg(const char *);
void error(std::string);
void info_msg(std::string);
void warning_msg(std::string);
void get_creds(char [], char[]);
std::string int_to_hex(int , int);
int stringify(char *out_str, char * arr, ...);

class IGDE_Editor;
class IGDE_Session;
class IGDE_Server;
class IGDE_Owner;
class IGDE_Message;

#endif /* igde_common_hpp */

