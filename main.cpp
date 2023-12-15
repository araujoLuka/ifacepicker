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
    // Extrai o nome do programa a partir do caminho completo
    std::string programName = argv[0];
    size_t lastSlash = programName.find_last_of('/');
    if (lastSlash != std::string::npos) {
        programName = programName.substr(lastSlash + 1);
    }

    // Verifica se há opção de ajuda
    if (argc == 2) {
        std::string arg = argv[1];
        if (arg == "-h" || arg == "--help") {
            showHelp(programName);
            return 0;
        }
    }

    // Comandos que podem ser usados
    const static char* COMMAND_IP{"ip a"};

    /* Abrindo um pipe para o comando 'ip a'
     * - std::unique_ptr<FILE, decltype(&pclose)>: Usa um ponteiro exclusivo para gerenciar o FILE* retornado por popen,
     *   especificando que a função pclose deve ser usada para fechar o recurso quando o ponteiro exclusivo sai de
     * escopo.
     *
     * decltype(&pclose): Retorna o tipo do ponteiro para a função pclose. No contexto do std::unique_ptr, isso informa
     * ao compilador qual função usar para deletar o recurso quando o ponteiro exclusivo é destruído.
     */
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(COMMAND_IP, "r"), pclose);

    // Verifica se o pipe foi aberto corretamente
    if (!pipe) {
        std::cerr << "Erro ao abrir o pipe para o comando: " << COMMAND_IP << std::endl;
        return 1;
    }

    // Lista para armazenar pares (nome da interface, endereço IP)
    std::vector<std::pair<std::string, std::string>> interfaceList;

    char buffer[128];           // Buffer para armazenar a linha lida do pipe
    std::string interfaceName;  // Nome da interface
    std::string ipAddress;      // Endereço IP da interface

    // Lê o pipe linha por linha
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        std::string line{buffer};
        std::istringstream iss{line};

        // Busca uma linha que tenha o formato: #: [nome da interface]: <...
        size_t pos{line.find(": <")};
        if (pos == std::string::npos) {
            // Não encontrou o formato esperado, ignora a linha
            continue;
        }

        // Obtém o nome da interface, ignorando o indice (3 primeiros caracteres)
        interfaceName = line.substr(3, pos - 3);

        // Busca nas linhas seguintes o endereço IP da interface
        // Caso encontre uma linha que comece com o indice atual seguido de ":",
        // significa que a interface não possui endereço IP configurado
        // (pula para a próxima interface)
        while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
            line = buffer;
            pos = line.find(": <");
            if (pos != std::string::npos) { // Encontrou uma nova interface
                /*
                 * Salva a interface atual na lista indicando que não possui endereço IP configurado.
                 * - std::emplace_back: Constrói diretamente um par (nome da interface, endereço IP) no final do vetor,
                 *   evitando a necessidade de criar variáveis temporárias antes da inserção.
                 */
                interfaceList.emplace_back(interfaceName, "<no ip address>");

                // Obtém o nome da nova interface, ignorando o indice (3 primeiros caracteres)
                interfaceName = line.substr(3, pos - 3);
                continue;
            }

            // Verifica se a linha contém um endereço IP
            pos = line.find("inet ");
            if (pos != std::string::npos) {
                // Pula a string "inet " e o espaço em branco
                pos += 5;
                std::string ipAddress{line.substr(pos, line.find('/', pos) - pos)};
                interfaceList.emplace_back(interfaceName, ipAddress);

                // Limpa as variáveis para validar o fim do pipe
                interfaceName.clear();
                ipAddress.clear();
                break;
            }
        }
    }

    if (interfaceName.length() > 0) {
        // Salva a última interface na lista indicando que não possui endereço IP configurado
        interfaceList.emplace_back(interfaceName, "<no ip address>");
    }

    // Exibe a lista de interfaces e endereços IP
    std::cout << "Lista de Interfaces e Endereços IP:" << std::endl;
    for (std::size_t i = 0; i < interfaceList.size(); ++i) {
        const auto& entry = interfaceList[i];
        std::cout << i + 1 << " - Interface: " << entry.first << ", IP: " << entry.second << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Escolha uma interface: ";
    std::size_t interfaceIndex;
    std::cin >> interfaceIndex;
    --interfaceIndex; // Ajusta o índice para o vetor de interfaces

    // Verifica se o índice da interface é válido
    if (interfaceIndex < 0 || interfaceIndex >= interfaceList.size()) {
        std::cerr << "Índice de interface inválido!" << std::endl;
        return 1;
    }

    // Exibe a interface selecionada 
    const auto& selectedInterface = interfaceList[interfaceIndex];
    std::cout << "IFACE=" << selectedInterface.first << std::endl;
    std::cout << "IPADDR=" << (selectedInterface.second.empty() ? "<no ip address>" : selectedInterface.second) << std::endl;

    return 0;
}
