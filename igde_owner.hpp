//
//  owner.hpp
//  igde
//
//  Created by Jason Graalum on 5/9/18.
//  Copyright © 2018 Jason Graalum. All rights reserved.
//

#ifndef igde_owner_hpp
#define igde_owner_hpp

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <thread>
#include <vector>

#include "igde_common.hpp"
#include "igde_message.hpp"

class IGDE_Owner
{
private:
    //
    int state;
    
    // Server connection details for owner as client
    std::string server_hostname;
    struct sockaddr_in server_addr;

    int server_portno;
    int server_socket_fd;

    // Owner "Server"" details(for Sessions)
    std::string owner_hostname;
    struct addrinfo *owner_addr;

    
    int control_portno;
    int control_socket_fd;

    
    //struct hostent *server;
    
    IGDE_Message * get_msg(int);
    
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    
    struct editor {
        int type;
        int state;
        int auth_state;
        int fd;
        struct sockaddr_storage * addr;
        std::string name;
    };
    
    std::list<editor *> editors;
    
    struct control {
        int type;
        int state;
        int auth_state;
        int fd;
        struct sockaddr_storage * addr;
        std::string name;
    };
    
    control controller;
    
public:
    IGDE_Owner();
    IGDE_Owner(int);

    void start();

    int open_control_socket();

    //TCP Loop Commands
    void message_loop(int);
    struct kevent ev_set;
    struct kevent owner_event_set;
    
    int owner_queue;
    int process_srv_msg(IGDE_Message *);
    int process_editor_msg(std::list<editor *>::iterator, IGDE_Message *);
    int process_control_msg(IGDE_Message *);
    
    // Control
    int connect_control(int, struct sockaddr_in *);
    int disconnect_control();

    // Server
    int connect_to_server(std::string, int);
    int disconnect_from_server();

    // Session
    // Start session
    int start_session(std::string );
    std::string session_name;

    int session_portno;
    int session_socket_fd;
    
    std::string session_filename;
    std::string session_filepath;
    int session_max_editors;
    
    std::ofstream session_ofd;
    int open_session_file();
    int open_session_socket();
    int upload_chunk(std::string);
    int finalize_upload(std::string);
    long close_session_ofd();
    std::string temp_dir = "/tmp/";

    std::ifstream editted_ifd;
    int download_file();
    
    // Editors
    int connect_editor(int, struct sockaddr_in * );
    int disconnect_editor(int);
    
    // End session
    int end_session();
    
    //int list_sessions();
    std::string list_editors();
    //void end_all();
    void stop_owner();


};
#endif /* igde_owner_hpp */
