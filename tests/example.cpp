#define CATCH_CONFIG_RUNNER
#include <fakeit.hpp>
#include "candevice/candevice.h"

TEST_CASE("Simple test", "[example]") {
    CanDevice cd;
    cd.init("peakcan", "can0");
    cd.start();
}

int main(int argc, char *argv[])
{
    return Catch::Session().run(argc, argv);
}
