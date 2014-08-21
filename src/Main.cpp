/* Virus_Interaction simulator
 * 
 * Written by Rory Dungan as part of the Programming Throwdown Showdown
 * (C) 2013
 */

#include "Simulation.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT) {
#else
int main(int argc, char* argv[]) {
#endif
    Simulation app;
    try {
        app.go();
    } catch( Ogre::Exception& e ) {
#ifdef _WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        std::cerr << "An exception has occured: " <<
            e.getFullDescription().c_str() << std::endl;
#endif
    }

    return 0;
}

#ifdef __cplusplus
}
#endif