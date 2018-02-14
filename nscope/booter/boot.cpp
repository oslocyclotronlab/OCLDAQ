#include "SystemBooter.h"
#include "Configuration.h"

#include <memory>

using namespace DAQ::DDAS;

int main() {

    SystemBooter booter;
    auto pConfig = Configuration::generate(FIRMWARE_FILE, "cfgPixie16.txt");
    booter.boot(*pConfig, SystemBooter::FullBoot);

    return 0;
}
