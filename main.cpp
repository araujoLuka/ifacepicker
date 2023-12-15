/*
 * ifacepicker - A C++ program for easy listing and selection of network interfaces and IP addresses using 'ip a'.
 * Lucas Araujo - 2023-12-15
 *
 * Purpose:
 *   `ifacepicker` simplifies the process of selecting a network interface or IP address, aiding in scripting scenarios.
 *   It enhances visibility across interfaces, making it useful for various tasks, such as configuring Wake-on-LAN.
 *
 * Compilation: g++ main.cpp -o ifacepicker
 */

#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// Function to display the help message
void showHelp(const std::string& programName) {
    std::ostringstream helpMessage;
    helpMessage << "Usage: " << programName << " [-h|--help]" << std::endl;
    helpMessage << "\nList and easily select network interfaces, displaying their respective IP addresses." << std::endl;
    helpMessage << "\nOutput:" << std::endl;
    helpMessage << "  IFACE=<interface-name>" << std::endl;
    helpMessage << "  IPADDR=<configured-ip>" << std::endl;
    helpMessage << "\nArguments:" << std::endl;
    helpMessage << "  -h, --help   Show this help message" << std::endl;

    std::cout << helpMessage.str();
}

int main(int argc, char* argv[]) {
    // Extract the program name from the full path
    std::string programName = argv[0];
    size_t lastSlash = programName.find_last_of('/');
    if (lastSlash != std::string::npos) {
        programName = programName.substr(lastSlash + 1);
    }

    // Check for help option
    if (argc == 2) {
        std::string arg = argv[1];
        if (arg == "-h" || arg == "--help") {
            showHelp(programName);
            return 0;
        }
    }

    // Commands that can be used
    const static char* COMMAND_IP{"ip a"};

    /* Open a pipe for the 'ip a' command
     * - std::unique_ptr<FILE, decltype(&pclose)>: Uses a unique pointer to manage the FILE* returned by popen,
     *   specifying that the pclose function should be used to close the resource when the unique pointer goes out of
     *   scope.
     *
     * decltype(&pclose): Returns the type of the pointer to the pclose function. In the context of std::unique_ptr, this
     *                     informs the compiler which function to use to delete the resource when the unique pointer is
     *                     destroyed.
     */
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(COMMAND_IP, "r"), pclose);

    // Check if the pipe was opened correctly
    if (!pipe) {
        std::cerr << "Error opening pipe for command: " << COMMAND_IP << std::endl;
        return 1;
    }

    // List to store pairs (interface name, IP address)
    std::vector<std::pair<std::string, std::string>> interfaceList;

    char buffer[128];           // Buffer to store the line read from the pipe
    std::string interfaceName;  // Interface name
    std::string ipAddress;      // Interface IP address

    // Read the pipe line by line
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        std::string line{buffer};
        std::istringstream iss{line};

        // Search for a line with the format: #: [interface name]: <...
        size_t pos{line.find(": <")};
        if (pos == std::string::npos) {
            // Format not found, ignore the line
            continue;
        }

        // Get the interface name, ignoring the index (first 3 characters)
        interfaceName = line.substr(3, pos - 3);

        // Search the following lines for the interface IP address
        // If a line starts with the current index followed by ":",
        // it means the interface has no configured IP address
        // (skip to the next interface)
        while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
            line = buffer;
            pos = line.find(": <");
            if (pos != std::string::npos) { // Found a new interface
                /*
                 * Save the current interface in the list indicating that it has no configured IP address.
                 * - std::emplace_back: Directly constructs a pair (interface name, IP address) at the end of the vector,
                 *   avoiding the need to create temporary variables before insertion.
                 */
                interfaceList.emplace_back(interfaceName, "<no ip address>");

                // Get the name of the new interface, ignoring the index (first 3 characters)
                interfaceName = line.substr(3, pos - 3);
                continue;
            }

            // Check if the line contains an IP address
            pos = line.find("inet ");
            if (pos != std::string::npos) {
                // Skip the string "inet " and the space
                pos += 5;
                std::string ipAddress{line.substr(pos, line.find('/', pos) - pos)};
                interfaceList.emplace_back(interfaceName, ipAddress);

                // Clear the variables to validate the end of the pipe
                interfaceName.clear();
                ipAddress.clear();
                break;
            }
        }
    }

    if (interfaceName.length() > 0) {
        // Save the last interface in the list indicating that it has no configured IP address
        interfaceList.emplace_back(interfaceName, "<no ip address>");
    }

    // Display the list of interfaces and IP addresses
    std::cout << "List of Interfaces and IP Addresses:" << std::endl;
    for (std::size_t i = 0; i < interfaceList.size(); ++i) {
        const auto& entry = interfaceList[i];
        std::cout << i + 1 << " - Interface: " << entry.first << ", IP: " << entry.second << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Choose an interface: ";
    std::size_t interfaceIndex;
    std::cin >> interfaceIndex;
    --interfaceIndex; // Adjust the index for the vector of interfaces

    // Check if the interface index is valid
    if (interfaceIndex < 0 || interfaceIndex >= interfaceList.size()) {
        std::cerr << "Invalid interface index!" << std::endl;
        return 1;
    }

    // Display the selected interface
    const auto& selectedInterface = interfaceList[interfaceIndex];
    std::cout << "IFACE=" << selectedInterface.first << std::endl;
    std::cout << "IPADDR=" << (selectedInterface.second.empty() ? "<no ip address>" : selectedInterface.second) << std::endl;

    return 0;
}
