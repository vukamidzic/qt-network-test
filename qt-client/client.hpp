#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QApplication>
#include <QTimer>
#include <QTcpSocket>
#include <QMessageBox>
#include <raylib.h>

#define STARTS_WITH(s,pre) ((s).find((pre),0)==0)

class Client : public QObject {
    Q_OBJECT
public:
    Client() {
        socket = new QTcpSocket(this);
        id = 0;
        connect(socket, &QTcpSocket::readyRead, this, &Client::receiveResponse);
        InitWindow(800,600,"Client");
        SetTargetFPS(60);

        if (!connectToHost("127.0.0.1")) {
            qDebug() << "Failed to connect to host, " << socket->errorString();
            QApplication::quit();
        }

        QTimer* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &Client::gameLoop);
        timer->start(15);
    }

    ~Client() {
        CloseWindow();
    }
public slots:
    void gameLoop() {
        if (WindowShouldClose()) {
            QApplication::quit();
            return;
        }

        movePlayer();

        BeginDrawing();
        ClearBackground(GetColor(0x181818FF));
        DrawRectangleV(position, Vector2{50,50}, RAYWHITE);
        EndDrawing();
    }

    void receiveResponse() {
        QByteArray msg = socket->readAll();
        std::string server_res = msg.toStdString();
        qDebug() << "Server response: " << QString::fromStdString(server_res);
        if (STARTS_WITH(server_res, "id+pos!")) {
            auto body = server_res.substr(7);
            qDebug() << "Message body: " << QString::fromStdString(body);

            auto delim_pos = body.find('|',0);
            this->id = std::stoi(body.substr(0, delim_pos))-1;
            auto v_str = body.substr(delim_pos+1);
            auto v_delim_pos = v_str.find(',',0);

            this->position.x = std::stof(v_str.substr(0,v_delim_pos));
            this->position.y = std::stof(v_str.substr(v_delim_pos+1)); 
        }
    }

private: 
    int id; 
    QTcpSocket* socket;
    Vector2 position;

    bool connectToHost(QString host) {
        socket->connectToHost(host, 8081);
        return socket->waitForConnected(); // wait connection;
    }

    void movePlayer() {
        if (IsKeyDown(KEY_LEFT)) position.x--;
        if (IsKeyDown(KEY_RIGHT)) position.x++;
        if (IsKeyDown(KEY_UP)) position.y--;
        if (IsKeyDown(KEY_DOWN)) position.y++;
    }
};

#endif