//
//  server.hpp
//  igde
//
//  Created by Jason Graalum on 5/9/18.
//  Copyright © 2018 Jason Graalum. All rights reserved.
//

#ifndef igde_server_hpp
#define igde_server_hpp
#include <iostream>
#include <sstream>
#include <string>
#include <list>

#include "igde_common.hpp"
#include "igde_message.hpp"

#define MAX_CLIENTS 10

class IGDE_Server
{
public:
    IGDE_Server();
    IGDE_Server(int);
    int start();
private:
    
    // Server Details
    char server_hostname[MAX_HOSTNAME_LEN];
    struct addrinfo *server_addr;

    int server_portno;
    int server_socket_fd;
    //struct hostent *server_ent;
    
    // Session Structure to track active sessions
    struct session {
        int client_id;
        int state;
        int max_editors;
        std::string filename;
        std::string session_name;
        int portno;
    };
    std::list<session *> session_list;
    
    // Client Connection Struct Array
    struct client
    {
        int type;
        int state;
        int fd;
        struct sockaddr_storage * addr;
    };
    
    struct client clients[MAX_CLIENTS];

    // Called from start()
    int open_server_socket();
    int listen_loop(int);
    
    // Inside listen_loop()
    int process_message(int, IGDE_Message *);
    int process_owner_message(int, IGDE_Message *);
    int process_editor_message(int, IGDE_Message *);

    // Methods called from process_message()
    int add_client(int, struct sockaddr_in *);
    int remove_client(int);
    int connect_owner(int, struct sockaddr_in *);
    int disconnect_owner(int);
    bool auth_new_connection(int);
    void set_session_timer(session);
    int ping_session(session);
    
    int add_session(int, IGDE_Message * msg);
    int remove_session(int, IGDE_Message * msg);
    int send_edit_sessions(int);
    int add_editor(session);
    int remove_editor(session);

    

};

#endif /* common_hpp */
