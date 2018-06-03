//
//  igde_control.hpp
//  igde
//
//  Created by Jason Graalum on 6/1/18.
//  Copyright © 2018 Jason Graalum. All rights reserved.
//

#ifndef idge_control_hpp
#define idge_control_hpp

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <thread>
#include <vector>

#include "igde_common.hpp"
#include "igde_message.hpp"

// Menu Items
#define O_MENU_CONNECT 1
#define O_MENU_DISCONNECT 2
#define O_MENU_START_SESSION 3
#define O_MENU_UPLOAD 4
#define O_MENU_DOWNLOAD 5
#define O_MENU_END_SESSION 6
#define O_MENU_LIST_EDITORS 7
#define O_MENU_EXIT 0

#define O_MENU_CONNECT_TXT "Connect to Server"
#define O_MENU_DISCONNECT_TXT "Disconnet from Server"
#define O_MENU_START_SESSION_TXT "Start Session"
#define O_MENU_UPLOAD_TXT "Upload Text File"
#define O_MENU_DOWNLOAD_TXT "Download Text File"
#define O_MENU_END_SESSION_TXT "End Session"
#define O_MENU_LIST_EDITORS_TXT "List Editors"
#define O_MENU_EXIT_TXT "Exit"

class IGDE_Control {
public:
    IGDE_Control();
    IGDE_Control(int, const char *);
    
    void start();
    
    // Menu Loop <- started as a thread
    void menu_loop();
    void display_menu();
    int open_menu_socket();
    
    void connect_to_server();
    void disconnect_from_server();
    void start_session();
    void end_session();
    void list_editors();
    void stop_owner();
    
private:
    // Owner connection Details
    char * owner_hostname;
    int owner_portno;
    int owner_socket_fd;
    struct sockaddr_in owner_addr;
 
    std::ifstream session_ifd;
    std::string session_filename;
    std::string session_name;
    int session_portno;
    int session_max_editors;
    int upload_file();

    std::string temp_dir = "/tmp/";

    int status;
    
    int connect_to_owner();
};

#endif /* idge_control_hpp */
