#include "server.h"

//Запись в логи
void logWriter(std::string buffer){
    time_t now = time(nullptr);
    std::string localTime = ctime(&now);
    llog << "[" << localTime << " " << buffer << std::endl;   
}

//Очистка игрового поля
void clearMap(){
    for (int i = 0; i < 3; ++i){
        for (int k = 0; k < 3; ++k){
            game_map[i][k] = ' ';
        }
    }
}

//Определение победителя
int winner(){
    int sum_diag = 0;
    int sum_diag_2 = 0; 
    for (int i = 0; i < 3; ++i){
        int sum_v = 0;
        int sum_h = 0;
        sum_diag += game_map[i][i];
        sum_diag_2 += game_map[2 - i][i];
        for (int j = 0; j < 3; ++j){
            sum_v += game_map[j][i];
            sum_h += game_map[i][j];
        }
        if (sum_v == 3 * 'X' || sum_h == 3 * 'X'){ return 1; }
        if (sum_v == 3 * 'O' || sum_h == 3 * 'O'){ return -1; }
    }
    if (sum_diag == 3 * 'X' || sum_diag_2 == 3 * 'X'){ return 1; }
    if (sum_diag == 3 * 'O' || sum_diag_2 == 3 * 'O'){ return -1; }
    return 0;
}

//Заполнение игрового поля по координате, которую ввел пользователь
bool ticTacToe(int coord){
    char symbol;
    if (gamer) { symbol = 'X'; }
    else { symbol = 'O'; }

    if (game_map[(coord - 1) / 3][(coord - 1) % 3] == ' '){ 
        game_map[(coord - 1) / 3][(coord - 1) % 3] = symbol; 
        gamer = !gamer;
    }
    else { return false; }
    return true;
}

//Проверка на то, существует ли клиент, который пытается подключиться
bool checkNewClientExist(std::string Nickname, std::string Password){
    std::ifstream config("config.txt");
    while (config.peek() != EOF){
        std::string str;
        std::getline(config, str);
        if (str == (Nickname + ' ' + Password)){
            config.close();
            return true; 
        }
    }
    config.close();
    return false;
}

//Проверка на то, не подключается ли тот же пользователь еще раз
bool checkNewClientNotOld(std::string Nickname){
    for (auto it = clients.begin(); it != clients.end(); ++it){
        if (it->second == Nickname){ return false; }
    }
    return true;
}

//Заполнение буфера по состояниям игрового поля
void fillBuffer(char* buffer){
    for (int i = 0, j = 0; i < 3; ++i){
        for (int k = 0; k < 3; ++k, ++j){
            buffer[j] = game_map[i][k];
        }
    }
}

//Обработка сообщений с клиента
void clientHandler(SOCKET socket) {
    char buffer[1024];
    int valread;
    while (true) {
        memset(buffer, 0, sizeof(buffer));      
        valread = recv(socket, buffer, 1024,NULL);

         //Клиент отключился
        if (valread == 0) {
            std::cout << "\033[0;31m" << "Client " << clients[socket] << " disconnected!" << "\033[0m" << std::endl;
            logWriter("Client " + clients[socket] + " disconnected!");
            clients.erase(socket);
            queue.erase(socket); 
            break;
        }

        //Превышено время хода
        if (strcmp(buffer, "-1") == 0){
            
            std::string text;
            if (queue[socket]) {
                text = "O winner!";                
            }
            else {
                text = "X winner!";           
            }
            for (auto it = clients.begin(); it != clients.end(); ++it) {
                send(it->first, text.c_str(), text.length(), NULL);               
            }

            count_of_move = 0;
            gamer = true;
            clearMap();
            logWriter(text);
            break;
        }

        //Если ход другого игрока, то пропускаем
        if (queue[socket] != gamer) { 
            continue;
        }
        //Заполняем игровое поле по пришедшей координате
        if (ticTacToe(static_cast<int>(buffer[0] - '0'))) { ++count_of_move; }
        else { 
            send(socket, "Wrong input!\n", sizeof("Wrong input!\n"), NULL);
            logWriter("Wrong input by " + clients[socket]);
            continue;
        }
        
        fillBuffer(buffer);

        std::string text;

        if (winner() == 1){
           text = "X winner!";
        }
        else if (winner() == -1){
            text = "O winner!";
        }
        else if (count_of_move == 9){
            text = "Draw!";
        }
        else {
            for (auto it = clients.begin(); it != clients.end(); ++it) {
                send(it->first, buffer, strlen(buffer), NULL);
            }
            continue;
        }

        for (auto it = clients.begin(); it != clients.end(); ++it) {
            send(it->first, text.c_str(), text.length(), NULL);
        }   
        logWriter(text);
        count_of_move = 0;
        gamer = true;
        clearMap();       
    }
}

//Очистка логов клиента
void clearLogClients(){
    std::ofstream tmp("log_clients.txt");
    tmp.close();
}

//Установка порта
void getPort(){     
    std::ifstream config("config.txt");
    if (!config.is_open()){
        const char* error = "No config file!";
        logWriter(error);
        llog.close();
        std::cout << error << std::endl;
        exit(0);
    }
    std::string string;
    while (config.peek() != EOF){
        std::getline(config, string);
        if (string.find("port: ") != std::string::npos){
            std::string num;
            for (int i = 6; i < string.length(); ++i){
                num += string[i];
            }
            try {
                port = std::stoi(num);
                logWriter("Installed port: " + num);
            } catch(std::invalid_argument const& ex){
                logWriter(ex.what());
                std::cout << ex.what() << std::endl;
                config.close();
                llog.close();
                exit(0);
            }
            break;
        }        
    }
    config.close();
}

//Запуск сервера
int openServer(){
    int server_fd;
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cout << "\nsocket failed\n";
        logWriter("socket failed");
        llog.close();
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    getPort();
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cout << "\nbind failed\n";
        logWriter("bind failed");
        llog.close();
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        std::cout << "\nlisten\n";
        logWriter("listen");
        llog.close();
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is running...\n";  
    logWriter("Server is running...");
    return server_fd;
}

//Определение первого хода
bool firstMove(){
    srand((int)time(0));
    return rand() % 2 == 1 ? true : false;
}

//Подключение клиентов
void connectionClients(SOCKET socket, bool &gamer_l){

    char username1[1024];
    char password1[1024];
    recv(socket, username1, sizeof(username1), NULL);
    recv(socket, password1, sizeof(password1), NULL);

    std::ifstream config("config.txt");
    std::string line;
    unsigned int move_time;
    std::unordered_map<std::string, std::string> users;
    std::getline(config, line);
    move_time = stoi(line);
    std::cout << move_time << std::endl;
    while (std::getline(config, line)) {
        size_t pos = line.find(" ");
        std::string username = line.substr(0, pos);
        line.erase(0, pos + 1);
        users[username] = line;
    }
    config.close();
    auto search1 = users.find(username1);
    if (search1 != users.end() and search1->second == password1) {
        send(socket, "Success connect!\n", sizeof("Success connect!\n"), 0);
    } 
    else {
        send(socket, "Unknown user!\n", sizeof("Unknown user!\n"), 0);
        logWriter("Unknown user try to connect to the server!");
        return;
    }
    std::cout << "\033[0;32mClient " << username1 << " connected!\n\033[0m";
    logWriter(std::string(username1) + " success connected to the server!");        

    clients.insert(std::pair<SOCKET, std::string>(socket, std::string(username1)));
    queue.insert(std::pair<SOCKET, bool>(socket, gamer_l));

    if (clients.size() == 1){   
        send(socket, "Wait other player!\n", sizeof("Wait other player!\n"), NULL);
        logWriter(std::string(username1) + " waiting other player!");
    }

    if (clients.size() == 2) {
        clearMap();
        std::string first_move;

        for (auto it = clients.begin(); it != clients.end(); ++it) {
            if (queue[it->first] == true){
                first_move = "First move is " + clients[it->first];
                logWriter(first_move);
            }
        }

        for (auto it = clients.begin(); it != clients.end(); ++it) {
            send(it->first, ("Game statred! " + first_move).c_str(), first_move.length() + sizeof("Game statred! "), NULL);
        }

        Sleep(1000);


    }
    gamer_l = !gamer_l;            
}

int main() {
    clearLogClients();
    WSAData wsaData;
    WORD DLLVersion = MAKEWORD(2, 1);
    if (WSAStartup(DLLVersion, &wsaData) != 0) {
        std::cout << "Error" << std::endl << std::endl;
        exit(1);
    }
    SOCKADDR_IN addr;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(1111);
    addr.sin_family = AF_INET;
    SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
    bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
    listen(sListen, SOMAXCONN);
    SOCKET FConnection;
    SOCKET SConnection;
    int sizeofaddr = sizeof(addr);
    std::cout << "Wating for a clients.";
    FConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);
    std::cout << "First accept";
    SConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);
    std::cout << "Second accept";
    SOCKET Players[2] = { FConnection, SConnection };
    
    bool gamer_l = firstMove();
    
    connectionClients(FConnection, gamer_l);
    connectionClients(SConnection, gamer_l);
    for (auto it = clients.begin(); it != clients.end(); ++it) {
        std::cout << queue[it->first] << " " << (queue[it->first] ? "1" : "0") << std::endl;
        if (queue[it->first]) {
            send(it->first, "1", sizeof("1"), NULL);
        }
        else {
            send(it->first, "0", sizeof("0"), NULL);
        }

    }

    //for (auto it = clients.begin(); it != clients.end(); ++it) {
    //    std::thread clientThread(clientHandler, it->first);
    //    clientThread.detach();
    //}
	std::thread FThread(clientHandler, FConnection);
    std::thread SThread(clientHandler, SConnection);
    FThread.join();
    SThread.join();
	
    return 0;
}
