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

        std::string res = "id+pos!"+std::to_string(id)+"|"+vec2_to_str(positions[id-1]);
        id+=1;

        socket->write(QString::fromStdString(res).toUtf8());
        socket->flush();
        socket->waitForBytesWritten();
    }

    void readyRead() {
        QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
        QByteArray msg = socket->readAll();
        QString msg_str = QString::fromUtf8(msg);
        qDebug() << "Received message: " << msg;
        
        if (msg_str.startsWith("update")) {
            QString args = msg_str.split('|')[1];
            QStringList parts = args.split(',');
            int id = parts[0].toInt();
            positions[id].x = parts[1].toFloat();
            positions[id].y = parts[2].toFloat();

            qDebug() << "Updated position: " << QString::fromStdString(vec2_to_str(positions[id]));
        }
        if (msg_str.startsWith("others?")) {
            int id = msg_str.split('|')[1].toInt();

            QString res = "others!|";
            QStringList ls;
            for (int i = 0; i < 3; i++) {
                if (i != id) {
                    ls.append(QString::fromStdString(std::to_string(positions[i].x)+","+std::to_string(positions[i].y)));
                }
            }

            socket->write((res+ls.join(',')).toUtf8());
            socket->flush();
        }
    }

    void disconnected() {
        QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
        qDebug() << "Client disconnected";
        socket->deleteLater();
    }
private:
    Vector2 positions[3] = {
        Vector2{400.0f, 300.0f},
        Vector2{100.0f, 50.0f},
        Vector2{200.0f, 700.0f}
    };
    int id = 1;
    QTcpServer* server;

    std::string vec2_to_str(Vector2 v) {
        return std::to_string(v.x) + "," + std::to_string(v.y);
    }
};

#endif