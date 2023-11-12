#include "LogoFunctions.h"

#include <windows.h>
#include <iostream>

// For logo ;p
int getTerminalWidth() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.dwSize.X;
}

void bigLogo() {
    std::cout << R"(


          JJJJJJJJJJJ                                                                                     WWWWWWWW                           WWWWWWWW
          J:::::::::J                                                                                     W::::::W                           W::::::W
          J:::::::::J                                                                                     W::::::W                           W::::::W
          JJ:::::::JJ                                                                                     W::::::W                           W::::::W
            J:::::J    eeeeeeeeeeee    rrrrr   rrrrrrrrr   zzzzzzzzzzzzzzzzzyyyyyyy           yyyyyyy      W:::::W           WWWWW           W:::::W
            J:::::J  ee::::::::::::ee  r::::rrr:::::::::r  z:::::::::::::::z y:::::y         y:::::y        W:::::W         W:::::W         W:::::W
            J:::::J e::::::eeeee:::::eer:::::::::::::::::r z::::::::::::::z   y:::::y       y:::::y          W:::::W       W:::::::W       W:::::W
            J:::::Je::::::e     e:::::err::::::rrrrr::::::rzzzzzzzz::::::z     y:::::y     y:::::y            W:::::W     W:::::::::W     W:::::W
            J:::::Je:::::::eeeee::::::e r:::::r     r:::::r      z::::::z       y:::::y   y:::::y              W:::::W   W:::::W:::::W   W:::::W
JJJJJJJ     J:::::Je:::::::::::::::::e  r:::::r     rrrrrrr     z::::::z         y:::::y y:::::y                W:::::W W:::::W W:::::W W:::::W
J:::::J     J:::::Je::::::eeeeeeeeeee   r:::::r                z::::::z           y:::::y:::::y                  W:::::W:::::W   W:::::W:::::W
J::::::J   J::::::Je:::::::e            r:::::r               z::::::z             y:::::::::y                    W:::::::::W     W:::::::::W
J:::::::JJJ:::::::Je::::::::e           r:::::r              z::::::zzzzzzzz        y:::::::y                      W:::::::W       W:::::::W
 JJ:::::::::::::JJ  e::::::::eeeeeeee   r:::::r             z::::::::::::::z         y:::::y                        W:::::W         W:::::W
   JJ:::::::::JJ     ee:::::::::::::e   r:::::r            z:::::::::::::::z        y:::::y                          W:::W           W:::W
     JJJJJJJJJ         eeeeeeeeeeeeee   rrrrrrr            zzzzzzzzzzzzzzzzz       y:::::y                            WWW             WWW
                                                                                  y:::::y
                                                                                 y:::::y
                                                                                y:::::y
                                                                               y:::::y
                                                                              yyyyyyy

                                                                                                                                                      )"
              << std::endl;
}

void mediumLogo() {
    std::cout << "                                                                                     \n";
    std::cout << "         ,---._                                                                      \n";
    std::cout << "       .-- -.' \\                                                               .---. \n";
    std::cout << "       |    |   :                                                             /. ./| \n";
    std::cout << "       :    ;   |            __  ,-.       ,----,                         .--'.  ' ; \n";
    std::cout << "       :        |          ,' ,'/ /|     .'   .`|                        /__./ \\ : | \n";
    std::cout << "       |    :   :   ,---.  '  | |' |  .'   .'  .'      .--,          .--'.  '   \\  . \n";
    std::cout << "       :           /     \\ |  |   ,',---, '   ./     /_ ./|         /___/ \\ |    ' ' \n";
    std::cout << "       |    ;   | /    /  |'  :  /  ;   | .'  /   , ' , ' :         ;   \\  \\;      : \n";
    std::cout << "   ___ l         .    ' / ||  | '   `---' /  ;--,/___/ \\: |          \\   ;  `      | \n";
    std::cout << " /    /\\    J   :'   ;   /|;  : |     /  /  / .`| .  \\  ' |           .   \\    .\\  ; \n";
    std::cout << "/  ../  `..-    ,'   |  / ||  , ;   ./__;     .'   \\  ;   :            \\   \\   ' \\ | \n";
    std::cout << "\\    \\         ; |   :    | ---'    ;   |  .'       \\  \\  ;             :   '  |--\"  \n";
    std::cout << " \\    \\      ,'   \\   \\  /          `---'            :  \\  \\             \\   \\ ;     \n";
    std::cout << "  \"---....--'      `----'                             \\  ' ;              '---\"      \n";
    std::cout << "                                                       `--`                          \n" << std::endl;
}

void smallLogo() {
    std::cout << "   ___                       _    _ \n";
    std::cout << "  |_  |                     | |  | |\n";
    std::cout << "    | | ___ _ __ _____   _  | |  | |\n";
    std::cout << "    | |/ _ \\ '__|_  / | | | | |\\| |\n";
    std::cout << "/\\__/ /  __/ |   / /| |_| | \\  /\\  /\n";
    std::cout << "\\____/ \\___|_|  /___|\\__, |  \\/  \\/ \n";
    std::cout << "                      __/ |         \n";
    std::cout << "                     |___/          \n" << std::endl;
}