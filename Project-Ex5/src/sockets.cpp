#include "command.h"
#include <string>


int main(int argc, const char *argv[]) {
    std::string cmdLine;

    // TODO: solve problem with command like /bin/bash -c "sleep 3 | sleep 3"
    for (auto ix = 0; ix < argc; ++ix)
    {
        cmdLine += std::string(argv[ix]) + " ";
    }

    Command cmd(argc, cmdLine);

    return 0;
}