#include "command.h"
#include <string>


int main(int argc, const char *argv[]) {
    std::string cmdLine;

    for (auto ix = 0; ix < argc; ++ix)
    {
        cmdLine += std::string(argv[ix]) + " ";
    }

    Command cmd(cmdLine);

    return 0;
}