#include "Registry.h"

#include <iostream>
#include <filesystem>
#include <memory>
namespace fs = std::filesystem;


const std::string ASSETS_DIR = "C:/Users/Bella/CLionProjects/AssetRegistry/assets";

bool isValidPath(const fs::path& p) {
    return fs::exists(p) && !fs::is_directory(p);
}

int main() {

    auto registry = std::make_unique<Registry>();

    std::cout << "Assets dir: " << ASSETS_DIR << "\n";

    while (true) {
        std::cout << "Which asset would you like to load?: ";
        std::string input;
        std::cin >> input;

        fs::path p(ASSETS_DIR);
        p += "/" + input;

        if (isValidPath(p)) {
            std::cout << "File '" << input << "' EXISTS at path " << p << "\n";
            const bool success = registry->Load(p);
            std::cout << "Load succeeded: " << success << "\n";
        } else {
            std::cout << "File '" << input << "' does not exist at path " << p << "\n";
        }
    }

    return 0;
}