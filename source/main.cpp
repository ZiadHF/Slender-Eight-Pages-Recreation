#include <Miniaudio.h>
#include <flags/flags.h>

#include <application.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <json/json.hpp>

#include "states/death-state.hpp"
#include "states/menu-state.hpp"
#include "states/play-state.hpp"
#include "states/settings-state.hpp"
#include "states/win-state.hpp"
#include "common/debug-utils.hpp"

// Helper function to find config file in multiple locations
std::string findConfigFile(const std::string& specified_path, const char* argv0) {
    // If a path was explicitly specified via command line, use it
    if (!specified_path.empty() && std::filesystem::exists(specified_path)) {
        return specified_path;
    }
    
    // List of paths to search (in order of priority)
    std::vector<std::string> search_paths = {
        "config/app.jsonc",           // Current working directory
        "../config/app.jsonc",        // One level up (for bin/ structure)
    };
    
    // Also try relative to the executable's location
    std::filesystem::path exe_path = std::filesystem::path(argv0).parent_path();
    if (!exe_path.empty()) {
        search_paths.push_back((exe_path / "config/app.jsonc").string());
        search_paths.push_back((exe_path / "../config/app.jsonc").string());
    }
    
    // Search for the config file
    for (const auto& path : search_paths) {
        if (std::filesystem::exists(path)) {
            // Try to set working directory to where assets are found
            try {
                std::filesystem::path config_path = std::filesystem::absolute(path);
                std::filesystem::path project_root = config_path.parent_path().parent_path();
                if (!project_root.empty() && std::filesystem::exists(project_root) && 
                    std::filesystem::is_directory(project_root) &&
                    std::filesystem::exists(project_root / "assets")) {
                    std::filesystem::current_path(project_root);
                    return "config/app.jsonc";  // Return relative path after changing dir
                }
            } catch (const std::filesystem::filesystem_error& e) {
                // If we can't change directory, just use the path as-is
                std::cerr << "Warning: Could not set working directory: " << e.what() << std::endl;
            }
            return path;
        }
    }
    
    // Return the originally specified path (will fail with helpful error message)
    return specified_path.empty() ? "config/app.jsonc" : specified_path;
}

int main(int argc, char** argv) {
    flags::args args(argc, argv);  // Parse the command line arguments
    // config_path is the path to the json file containing the application
    // configuration Default: "config/app.jsonc"
    std::string specified_config = args.get<std::string>("c", "");
    std::string config_path = findConfigFile(specified_config, argv[0]);
    
    // run_for_frames is how many frames to run the application before
    // automatically closing This is useful for testing multiple configurations
    // in a batch Default: 0 where the application runs indefinitely until
    // manually closed
    int run_for_frames = args.get<int>("f", 0);

    // Open the config file and exit if failed
    std::ifstream file_in(config_path);
    if (!file_in) {
        std::cerr << "Couldn't open file: " << config_path << std::endl;
        return -1;
    }
    // Read the file into a json object then close the file
    nlohmann::json app_config =
        nlohmann::json::parse(file_in, nullptr, true, true);
    file_in.close();

    // Set debug mode from config (default: false)
    our::setDebugMode(app_config.value("debug_mode", false));

    // Create the application
    our::Application app(app_config);

    // Register all the states of the project in the application
    app.registerState<Menustate>("menu");
    app.registerState<Playstate>("play");
    app.registerState<SettingsState>("settings");
    app.registerState<WinState>("win");
    app.registerState<DeathState>("death");

    // Then choose the state to run based on the option "start-scene" in the
    // config
    if (app_config.contains(std::string{"start-scene"})) {
        app.changeState(app_config["start-scene"].get<std::string>());
    }

    // Finally run the application
    // Here, the application loop will run till the terminatio condition is
    // statisfied
    return app.run(run_for_frames);
}