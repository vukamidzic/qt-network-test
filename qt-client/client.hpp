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
        connect(socket, &QTcpSocket::readyRead, this, &Client::receiveResponse);
        InitWindow(800,600,"Client");
        SetTargetFPS(60);

        if (!connectToHost("127.0.0.1")) {
            // QMessageBox::critical(nullptr,"Error","Failed to connect to server");
            QApplication::quit();
        }

        if (!sendRequest("id?")) {
            // QMessageBox::critical(nullptr,"Error","Failed to send request(ID)");
            QApplication::quit();
        }
        qDebug() << "Client ID: " << id;

        if (!sendRequest("pos?")) {
            // QMessageBox::critical(nullptr,"Error","Failed to send request(POS)");
            QApplication::quit();
        }

        QTimer* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &Client::gameLoop);
        timer->start(16.667);
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
        if (STARTS_WITH(server_res, "id!")) {
            id = std::stoi(server_res.substr(server_res.find('!',0)+1));
        }
        if (STARTS_WITH(server_res, "pos!")) {
            auto vec_pos = server_res.find('!',0);
            auto v = server_res.substr(vec_pos+1);
            auto delim_pos = v.find(',',0);
            float x = std::stof(server_res.substr(0, delim_pos));
            float y = std::stof(server_res.substr(delim_pos+1));
            position = Vector2 {x,y};
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

    bool sendRequest(QString msg) {
        QByteArray barr = msg.toUtf8();
        
        if (socket->state() == QTcpSocket::ConnectedState) {
            qDebug() << "Sending message:" << msg;
            socket->write(barr);
            socket->flush();
            return socket->waitForBytesWritten();
        }
        else 
            return false;
    }

    void movePlayer() {
        if (IsKeyDown(KEY_LEFT)) position.x--;
        if (IsKeyDown(KEY_RIGHT)) position.x++;
        if (IsKeyDown(KEY_UP)) position.y--;
        if (IsKeyDown(KEY_DOWN)) position.y++;
    }
};

#endif