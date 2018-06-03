//
//  editor.hpp
//  igde
//
//  Created by Jason Graalum on 5/9/18.
//  Copyright © 2018 Jason Graalum. All rights reserved.
//

#ifndef igde_editor_hpp
#define igde_editor_hpp

#include <iostream>

#include "igde_common.hpp"

class IGDE_Editor
{
public:
    IGDE_Editor();
    IGDE_Editor(int, const std::string& hostname);
    
private:
    int server_portno;
    int owner_portno;
    std::string hostname;
    
};

#endif /* editor_hpp */
