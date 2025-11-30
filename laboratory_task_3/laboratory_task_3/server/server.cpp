#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <sstream>
#include <algorithm>
#include <cctype>

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

const int PORT = 1280;
mutex data_mutex;

struct Student {
    string fio;
    string group;
    double stipend;
    vector<int> grades;
};

vector<Student> students;

void init_students() {
    students = {
        {"Ivanov Ivan Ivanovich", "101", 1200, {5, 4, 5}},
        {"Petrov Petr Petrovich", "102", 1100, {3, 4, 4}},
        {"Sidorova Mariya Pavlovna", "103", 1500, {5, 5, 5}},
        {"Aleksandrov Nikolay Sergeevich", "104", 900, {4, 3, 4}},
        {"Kozlov Andrey Igorevich", "105", 1000, {5, 5, 4}}
    };
}

string student_to_string(const Student& s) {
    ostringstream out;
    out << s.fio << "; группа " << s.group
        << "; стипендия " << s.stipend << "; оценки: ";
    for (size_t i = 0; i < s.grades.size(); ++i) {
        out << s.grades[i];
        if (i + 1 < s.grades.size()) out << ",";
    }
    out << "\n";
    return out.str();
}

string handle_view(char letter) {
    ostringstream out;
    letter = toupper((unsigned char)letter);
    lock_guard<mutex> lock(data_mutex);
    for (auto& s : students) {
        char first = toupper((unsigned char)s.fio[0]);
        if (first == letter) out << student_to_string(s);
    }
    string result = out.str();
    if (result.empty()) return "Студентов на букву нет.\n";
    return result;
}

string handle_list() {
    ostringstream out;
    lock_guard<mutex> lock(data_mutex);
    for (auto& s : students) out << student_to_string(s);
    return out.str();
}

string handle_add(const string& args) {
    istringstream ss(args);
    string fio, group, stipend_str, grades_str;
    getline(ss, fio, ';');
    getline(ss, group, ';');
    getline(ss, stipend_str, ';');
    getline(ss, grades_str);

    Student st;
    st.fio = fio;
    st.group = group;
    st.stipend = atof(stipend_str.c_str());

    istringstream gs(grades_str);
    string token;
    while (getline(gs, token, ',')) st.grades.push_back(atoi(token.c_str()));

    lock_guard<mutex> lock(data_mutex);
    students.push_back(st);
    return "Добавлено.\n";
}

string handle_edit(const string& args) {
    istringstream ss(args);
    string fio, group, stipend_str, grades_str;
    getline(ss, fio, ';');
    getline(ss, group, ';');
    getline(ss, stipend_str, ';');
    getline(ss, grades_str);

    lock_guard<mutex> lock(data_mutex);
    for (auto& s : students) {
        if (s.fio == fio) {
            s.group = group;
            s.stipend = atof(stipend_str.c_str());
            s.grades.clear();
            istringstream gs(grades_str);
            string token;
            while (getline(gs, token, ',')) s.grades.push_back(atoi(token.c_str()));
            return "Изменено.\n";
        }
    }
    return "Не найден студент.\n";
}

string handle_delete(const string& fio) {
    lock_guard<mutex> lock(data_mutex);
    auto it = remove_if(students.begin(), students.end(),
        [&](const Student& s) { return s.fio == fio; });
    if (it != students.end()) {
        students.erase(it, students.end());
        return "Удалено.\n";
    }
    return "Не найден студент.\n";
}

void client_thread(SOCKET client) {
    char buf[1024];
    while (true) {
        int len = recv(client, buf, sizeof(buf) - 1, 0);
        if (len <= 0) break;
        buf[len] = 0;
        string msg(buf);
        msg.erase(remove(msg.begin(), msg.end(), '\r'), msg.end());
        msg.erase(remove(msg.begin(), msg.end(), '\n'), msg.end());

        string cmd, args;
        size_t pos = msg.find(' ');
        if (pos != string::npos) {
            cmd = msg.substr(0, pos);
            args = msg.substr(pos + 1);
        }
        else cmd = msg;

        string response;

        if (cmd == "VIEW" && !args.empty())
            response = handle_view(args[0]);
        else if (cmd == "ADD")
            response = handle_add(args);
        else if (cmd == "EDIT")
            response = handle_edit(args);
        else if (cmd == "DEL")
            response = handle_delete(args);
        else if (cmd == "LIST")
            response = handle_list();
        else if (cmd == "EXIT")
            break;
        else
            response = "Неизвестная команда.\n";

        send(client, response.c_str(), (int)response.size(), 0);
    }
    closesocket(client);
}



int main() {
    WSADATA wsa; WSAStartup(MAKEWORD(2, 2), &wsa);
    setlocale(LC_ALL, "Russian");
    init_students();
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(s, (sockaddr*)&addr, sizeof(addr));
    listen(s, 5);

    cout << "Сервер запущен на порту " << PORT << "...\n";
    while (true) {
        sockaddr_in cli{};
        socklen_t cl = sizeof(cli);
        SOCKET c = accept(s, (sockaddr*)&cli, &cl);
        thread(client_thread, c).detach();
    }

    WSACleanup();
    return 0;
}
