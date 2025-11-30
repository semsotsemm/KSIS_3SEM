/* * UDP-Клиент
 * Исправлены все предупреждения (inet_pton и size_t)
 */

#pragma comment(lib, "ws2_32.lib") 

#include <winsock2.h>
#include <ws2tcpip.h>   // <-- 1. ДОБАВЛЕН ЭТОТ ЗАГОЛОВОК для inet_pton
#include <iostream>
#include <string> 
#include <cstring>

using namespace std;

#define SERVER_IP "127.0.0.1" // IP-адрес сервера (localhost)
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

    // 2. Создание сокета
    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cout << "Socket creation failed with error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // 3. Настройка адреса сервера
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);

    //
    //  ↓↓↓ 2. ИСПРАВЛЕНИЕ ПРЕДУПРЕЖДЕНИЯ (inet_addr -> inet_pton) ↓↓↓
    //
    // Было: serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    int serverAddrLen = sizeof(serverAddr);

    // 4. Ввод данных
    char sendBuffer[BUFFER_SIZE];
    cout << "Enter the string, please: ";
    cin.getline(sendBuffer, BUFFER_SIZE, '\n');

    // 5. Отправка данных (sendto)
    //
    //  ↓↓↓ 3. ИСПРАВЛЕНИЕ ПРЕДУПРЕЖДЕНИЯ (size_t -> int) ↓↓↓
    //
    int bytesSent = sendto(clientSocket, sendBuffer, (int)strlen(sendBuffer), 0,
        (struct sockaddr*)&serverAddr, serverAddrLen);

    if (bytesSent == SOCKET_ERROR) {
        cout << "sendto failed with error: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "Sent " << bytesSent << " bytes to server." << endl;

    // 6. Получение ответа (recvfrom)
    char recvBuffer[BUFFER_SIZE];
    // Указываем МАКСИМАЛЬНЫЙ размер буфера, который можем принять
    int bytesReceived = recvfrom(clientSocket, recvBuffer, BUFFER_SIZE - 1, 0,
        (struct sockaddr*)&serverAddr, &serverAddrLen);

    if (bytesReceived == SOCKET_ERROR) {
        cout << "recvfrom failed with error: " << WSAGetLastError() << endl;
    }
    else {
        recvBuffer[bytesReceived] = '\0';

        // 7. Вывод результата
        cout << "Server response: " << recvBuffer << endl;
    }

    // 8. Очистка
    closesocket(clientSocket);
    WSACleanup();

    system("pause");
    return 0;
}