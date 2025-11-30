/* * UDP-Сервер (Последовательный)
 * Исправлены все предупреждения (inet_pton и size_t)
 */

#pragma comment(lib, "ws2_32.lib") 

#include <winsock2.h>
#include <ws2tcpip.h>   // <-- Нужен для inet_pton
#include <iostream>
#include <cstring>

using namespace std;

#define PORT 1024       
#define BUFFER_SIZE 256 

int main() {
    // 1. Инициализация WinSock
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        cout << "WSAStartup failed with error: " << err << endl;
        return 1;
    }

    cout << "UDP Server started..." << endl;

    // 2. Создание сокета
    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cout << "Socket creation failed with error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // 3. Привязка сокета (bind)
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);

    // Привязываем сервер только к localhost (127.0.0.1)
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    //
    // Или, если хотите слушать на ВСЕХ адресах, используйте эту строку:
    // serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "Bind failed with error: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server is listening on 127.0.0.1:" << PORT << endl;

    // 4. Цикл обработки запросов
    char buffer[BUFFER_SIZE];
    sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    while (true) {
        cout << "\nWaiting for data..." << endl;

        // 5. Получение данных (recvfrom)
        // Указываем МАКСИМАЛЬНЫЙ размер буфера, который можем принять
        int bytesReceived = recvfrom(serverSocket, buffer, BUFFER_SIZE - 1, 0,
            (struct sockaddr*)&clientAddr, &clientAddrLen);

        if (bytesReceived == SOCKET_ERROR) {
            cout << "recvfrom failed with error: " << WSAGetLastError() << endl;
            continue;
        }

        buffer[bytesReceived] = '\0';
        cout << "Received " << bytesReceived << " bytes from client: " << buffer << endl;

        // 6. Обработка данных (Вариант 13)
        for (int i = 0; i < bytesReceived - 1; i += 2) {
            char temp = buffer[i];
            buffer[i] = buffer[i + 1];
            buffer[i + 1] = temp;
        }

        cout << "Processed string: " << buffer << endl;

        // 7. Отправка данных обратно клиенту (sendto)
        // 
        //  ↓↓↓ ИСПРАВЛЕНИЕ ПРЕДУПРЕЖДЕНИЯ (size_t -> int) ↓↓↓
        //
        int bytesSent = sendto(serverSocket, buffer, (int)strlen(buffer), 0,
            (struct sockaddr*)&clientAddr, clientAddrLen);

        if (bytesSent == SOCKET_ERROR) {
            cout << "sendto failed with error: " << WSAGetLastError() << endl;
        }
        else {
            cout << "Sent " << bytesSent << " bytes back to client." << endl;
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}