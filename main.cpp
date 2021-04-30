#include <iostream>
#include "./server/Server.h"
#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>

int main() {
    std::cout << "Starting" << std::endl;
    Server s(8080);
    s.start();
    return 0;
}
