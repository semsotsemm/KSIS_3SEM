#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    setlocale(LC_ALL, "Russian");
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(1280);
    inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr);

    if (connect(s, (sockaddr*)&serv, sizeof(serv)) != 0) {
        cerr << "Не удалось подключиться\n";
        return 1;
    }
    cout << "Подключено к серверу. Доступные команды:\n"
        << "VIEW <буква>\nADD <ФИО>;<группа>;<стипендия>;<оценки>\n"
        << "EDIT <ФИО>;<новая группа>;<новая стипендия>;<оценки>\n"
        << "DEL <ФИО>\nLIST\nEXIT\n";

    string line;
    char buf[2048];
    while (true) {
        cout << "> ";
        if (!getline(cin, line))
        {
            break;
        }
        line += "\n";
        send(s, line.c_str(), (int)line.size(), 0);
        if (line.find("EXIT") == 0) 
        {
            break;
        }

        int n = recv(s, buf, sizeof(buf) - 1, 0);
        if (n <= 0) break;
        buf[n] = 0;
        cout << buf;
    }

    closesocket(s);
    WSACleanup();
    return 0;
}
