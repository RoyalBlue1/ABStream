// VulkanTest.cpp : Defines the entry point for the application.
//



#include "VulkanTest.h"




#include "spdlog/spdlog.h"
#include <fstream>
#include "st_app.h"
#include "st_settings_controller.h"


int main(int argc, char* argv[])
{
    st::StSettingsManager::Initialize(argc,argv);

    st::StApp app{};
    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
