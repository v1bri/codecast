#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string s;
    while (std::getline(std::cin, s)) {
        std::cout << s;
        if (std::cin.peek() != std::char_traits<char>::eof()) {
            std::cout << std::endl;
        }
    }
    return 0;
}
