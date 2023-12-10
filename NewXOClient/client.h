#include <iostream>
#include <fstream>
//#include <sys/socket.h>
//#include <arpa/inet.h>
//#include <unistd.h>

#include <cstring>
#include <string>
#include <thread>
#include <chrono>
#include <future>
#include <ctime>
#pragma comment(lib, "ws2_32.lib")
#define __STDC_WANT_LIB_EXT1__ 1
#define _CRT_SECURE_NO_WARNINGS 1
#include <winsock2.h>
#pragma warning(disable: 4996)
//#include <netinet/in.h>
//#include <unistd.h>
#include <map>
#include <stdio.h>
#include <vector>

int time_for_move = 1000000; //Время на ход
int port; //Порт
bool gamer = true; //Состояние текущего хода
bool move; //Состояние хода определенного игрока
std::ofstream llog("log_clients.txt", std::ios::app); //Файл с логами

//Игровое поле
char game_map[3][3] = {{' ', ' ', ' '},
                       {' ', ' ', ' '},
                       {' ', ' ', ' '}};
