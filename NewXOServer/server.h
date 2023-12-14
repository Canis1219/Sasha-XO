#include <iostream>
#include <fstream>
#pragma comment(lib, "ws2_32.lib")
#define __STDC_WANT_LIB_EXT1__ 1
#define _CRT_SECURE_NO_WARNINGS 1
#include <winsock2.h>
#pragma warning(disable: 4996)
//#include <netinet/in.h>
//#include <unistd.h>
#include <cstring>
#include <thread>
#include <map>
#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "windows.h"
std::map<SOCKET, std::string> clients; //Вектор с клиентами в сети
std::map<SOCKET, bool> queue; //Вектор со значениями ходов для каждого клиента в сети
int count_of_move = 0;  //Количество ходов
int port = 0; 
struct sockaddr_in address; 
int addrlen = sizeof(address);
std::ifstream config("config.txt");
std::string line;
unsigned int move_time;
std::unordered_map<std::string, std::string> users;
std::ofstream llog;

//Игровое поле
char game_map[3][3] = {{' ', ' ', ' '},
                       {' ', ' ', ' '},
                       {' ', ' ', ' '}};
                       
bool gamer = true; //Состояние текущего хода
