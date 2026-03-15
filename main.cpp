#include <iostream>
#include <filesystem>

bool isValidPath(std::string path) {

    if (path == "hello") {
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