#include "client_controller.h"
#include <sstream>

#define MAX_DIGITS_UINT16 5
#define MAX_NUMB_UINT16 65535
#define UINT32_SIZE 4 
#define ERROR 1
#define SUCCESS 0

int ClientController::checkLine(std::string &line) {
    if ((line == "AYUDA") || (line == "RENDIRSE")) {
        return SUCCESS;
    }

    for (std::size_t i = 0; i < line.size(); i++) {
        if (!std::isdigit(line[i])) {
            return ERROR;
        }
    }

    if ((line.size() > MAX_DIGITS_UINT16) || (line.size() == 0)) {
        return ERROR;
    }

    int n = std::stoi(line);
    
    if ((n < 0) || (n > MAX_NUMB_UINT16)) {
        return ERROR;
    }

    return SUCCESS;
}

void ClientController::sendLine(std::string &line) {
    if (line == "AYUDA") {
        char msg[1] = {'h'};
        this->connector.sendMsg(msg, 1);
    } else if (line == "RENDIRSE") {
        char msg[1] = {'s'};
        this->connector.sendMsg(msg, 1);
    } else {
        char msg[3];
        msg[0] = 'n';
        uint16_t num = htons((uint16_t)std::stoi(line));
        msg[1] = ((char *) &num)[0];
        msg[2] = ((char *) &num)[1];
        this->connector.sendMsg(msg, 3);
    }
}

void ClientController::receiveResult(char** result) {
    char tamBytes[UINT32_SIZE];
    this->connector.rcvMsg(tamBytes, UINT32_SIZE);

    uint32_t tam = (tamBytes[3] << 24) + (tamBytes[2] << 16) + 
                   (tamBytes[1] << 8) + tamBytes[0];

    tam = ntohl(tam);
    
    char *msg = (char*) malloc((tam + 1) * sizeof(char));
    this->connector.rcvMsg(msg, tam);
    msg[tam] = '\0';

    (*result) = msg;
    /*CAMBIAR IMPLEMENTACION DAR ORIENTACION A OBJETOS ENCAPSULAR MALLOC*/
    /*AGREGAR UN IF MSG == "PERDISTE" O "GANASTE" -> ENDED=TRUE*/
}

ClientController::ClientController(ClientConnector &aConnector) : 
                                        connector(aConnector), ended(false) {}

ClientController::~ClientController() {}

void ClientController::run() {
    std::string line;
    getline(std::cin, line);
    
    if (this->checkLine(line) == ERROR) {
        char e[] = "Error: comando invalido. Escriba AYUDA para obtener ayuda";
        std::cout << e << std::endl;
        return;
    }

    this->sendLine(line);
    
    char* result;
    this->receiveResult(&result);

    if ((!strcmp(result, "Ganaste")) || (!strcmp(result, "Perdiste"))) {
        this->ended = true;
    }

    std::cout << result << std::endl;
    free(result);
    /*CAMBIAR IMPLEMENTACION DAR ORIENTACION A OBJETOS ENCAPSULAR FREE*/
}

bool ClientController::end() {
    return this->ended;
}
