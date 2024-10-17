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
            qDebug() << "Failed to start server";
            return;
        }
        qDebug() << "Server started on port 8081";
    }

public slots:
    void newConnection() {
        QTcpSocket *socket = nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &Server::readyRead);
        connect(socket, &QTcpSocket::disconnected, this, &Server::disconnected);

        // id|pos

        std::string res = "id+pos!"+std::to_string(id)+"|"+vec2_to_str(positions[id-1]);
        id+=1;

        socket->write(QString::fromStdString(res).toUtf8());
        socket->flush();
        socket->waitForBytesWritten();
    }

    void readyRead() {
        QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
        QByteArray msg = socket->readAll();
        qDebug() << "Received message: " << msg;
        std::string req = msg.toStdString();
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
    int id = 1;
    QTcpServer* server;

    std::string vec2_to_str(Vector2 v) {
        return std::to_string(v.x) + "," + std::to_string(v.y);
    }
};

#endif