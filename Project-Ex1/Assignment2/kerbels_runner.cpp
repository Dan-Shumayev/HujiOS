#include "osm.h"
#include <iostream>
#include <fstream>
#include <map>
#include <unistd.h>
#include <sstream>
#include <vector>

enum MeasurementType
{
    Instruction, FunctionCall, SysCall
};

static const std::map<MeasurementType, const char*> MEASUREMENT_TYPE_TO_STRING {
    {Instruction, "Instruction"},
    {FunctionCall, "FunctionCall"},
    {SysCall, "SysCall"}
};

static int HOST_NAME_LEN = 64;

int main(int argc, const char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Expected 1 argument(current environment)" << std::endl;
        return EXIT_FAILURE;
    }

    // not really needed, used to further identify the environment the test was run at.
    std::string machine = argv[1];
    char hostName[HOST_NAME_LEN];
    if (gethostname(hostName, HOST_NAME_LEN))
    {
        std::cerr << "Failed to obtain host name" << std::endl;
        return EXIT_FAILURE;
    }
    std::ofstream csvFile("osm_ex1_measurements.csv", std::ios::app);
    if (csvFile.tellp() == 0)
    {
        csvFile << "Machine,Host,MeasurementType,Iterations,TimeNs" << std::endl;
    }

    auto measure = [&](const MeasurementType& type, int numIterations) {
        double result = -1;
        switch (type)
        {
        case Instruction:
            result = osm_operation_time(numIterations);
            break;
        case FunctionCall:
            result = osm_function_time(numIterations);
            break;
        case SysCall:
            result = osm_syscall_time(numIterations);
            break;
        }
        std::cout << "Measuring " << MEASUREMENT_TYPE_TO_STRING.at(type) << " with " << numIterations << " iterations\n";
        csvFile << machine << "," << hostName << "," << MEASUREMENT_TYPE_TO_STRING.at(type)
                << "," << numIterations << "," << result << std::endl;
    };

    std::vector<int> iterSizes  {10, 1000, 1000000, 1000000 , 1000000000};

    for (int numIterations: iterSizes)
    {
        measure(Instruction, numIterations);
        measure(FunctionCall, numIterations);
        if (numIterations != iterSizes[iterSizes.size()-1])
        {
            measure(SysCall, numIterations);
        }
    }

    std::cout << "Done" << std::endl;
    return EXIT_SUCCESS;
}
