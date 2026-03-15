#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

bool isValidPath(const std::string& pathStr) {
    fs::path p(pathStr);

    if (fs::exists(p)) {
        return true;
    }

    return false;
}

int main() {

    const auto lang = "C++";
    std::cout << "Hello and welcome to " << lang << "!\n";

    while (true) {
        std::string input;
        std::cin >> input;
        std:: cout << input << "\n";
        std::cout << isValidPath(input) << "\n";
    }

    return 0;
}