//
//  main.cpp
//  igde
//
//  Created by Jason Graalum on 5/9/18.
//  Copyright © 2018 Jason Graalum. All rights reserved.
//

#include "igde_common.hpp"
#include "igde_server.hpp"
#include "igde_owner.hpp"
#include "igde_control.hpp"

int main(int argc, const char * argv[]) {
  
    if(argc < 3)
    {
        std::cout << "Wrong number of args" << std::endl;
        return(-1);
    }
    std::cout << "Args = " << argv[1] << " : " << argv[2] << std::endl;
    
    if(std::strncmp(argv[1],"-s",2) == 0)
    {
        int srv_portno = atoi(argv[2]);
        
        std::cout << "Starting server on port " << srv_portno << std::endl;
        IGDE_Server server = IGDE_Server(srv_portno);
        server.start();
    
    }
    else if(std::strcmp(argv[1],"-o")==0)
    {
        
        int control_portno = atoi(argv[2]);

        std::cout << "Starting owner on port " << control_portno << std::endl;
        IGDE_Owner owner = IGDE_Owner(control_portno);
        owner.start();
    }
    else if(std::strcmp(argv[1],"-c")==0)
    {
    
        int owner_portno = atoi(argv[2]);
        
        std::cout << "Starting control on port " << owner_portno << std::endl;
        IGDE_Control control = IGDE_Control(owner_portno, argv[3]);
        control.start();
    
    }
    
    return 1;
}

