#ifndef SERVER_H
#define SERVER_H

#include <raylib.h>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMessageBox>

#define STARTS_WITH(s, pre) ((s).find((pre),0)==0)

class Server : public QTcpServer {
    Q_OBJECT
public:
    Server(QObject *parent = nullptr) : QTcpServer(parent) {
        connect(this, &QTcpServer::newConnection, this, &Server::newConnection);
        if (!listen(QHostAddress::Any, 8081)) {
            QMessageBox::critical(nullptr, "Error", "Failed to start server");
            return;
        }
        qDebug() << "Server started on port 8081";
    }

public slots:
    void newConnection() {
        QTcpSocket *socket = nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &Server::readyRead);
        connect(socket, &QTcpSocket::disconnected, this, &Server::disconnected);
    }

    void readyRead() {
        QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
        QByteArray msg = socket->readAll();
        qDebug() << "Received message: " << msg;
        std::string req = msg.toStdString();
        if (STARTS_WITH(req, "id?")) {
            std::string res = "id!"+std::to_string(id++);
            qDebug() << "Sending to client: " << QString::fromStdString(res);
            socket->write(res.c_str());
            socket->flush();
            socket->waitForBytesWritten(1000);
        }
        if (STARTS_WITH(req, "pos?")) {
            int client_id = std::stoi(req.substr(req.find('?',0)+1))-1;
            qDebug() << "Sender ID: " << client_id;
            std::string res = "pos!"+vec2_to_str(positions[client_id]);
            qDebug() << "Sending to client: " << QString::fromStdString(res);
            socket->write(res.c_str());
            socket->flush();
            socket->waitForBytesWritten(1000);
        }
    }

    void disconnected() {
        QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
        qDebug() << "Client disconnected";
        socket->deleteLater();
    }
private:
    Vector2 positions[2] = {
        Vector2{400.0f, 300.0f},
        Vector2{100.0f, 50.0f}
    };
    short id = 1;
    QTcpServer* server;

    std::string vec2_to_str(Vector2 v) {
        return std::to_string(v.x) + "," + std::to_string(v.y);
    }
};

#endif