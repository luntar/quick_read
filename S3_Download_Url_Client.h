#ifndef S3_DOWNLOAD_URL_CLIENT_H
#define S3_DOWNLOAD_URL_CLIENT_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>
#include "AR_ASSERT.h"


class S3_Download_Url_Client : public QObject
{
    Q_OBJECT

public:
    explicit S3_Download_Url_Client(const QString& lambda_url, QObject* parent = nullptr)
        : QObject(parent)
        , _lambda_url(lambda_url)
    {
        AR_ASSERT(!_lambda_url.isEmpty());
    }

    void request_download_url(const QString& object_key)
    {
        AR_ASSERT(!_lambda_url.isEmpty());
        AR_ASSERT(!object_key.isEmpty());

        if (_lambda_url.isEmpty() || object_key.isEmpty()) {
            emit request_failed("invalid arguments");
            return;
        }

        QJsonObject request_obj;
        request_obj["action"] = "request_download_url";
        request_obj["object_key"] = object_key;

        const QByteArray request_body =
            QJsonDocument(request_obj).toJson(QJsonDocument::Compact);

        QNetworkRequest request{ QUrl{ _lambda_url } };
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QNetworkReply* reply = _network_access_manager.post(request, request_body);
        AR_ASSERT(reply != nullptr);

        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            const QByteArray response_body = reply->readAll();

            if (reply->error() != QNetworkReply::NoError) {
                emit request_failed(reply->errorString());
                reply->deleteLater();
                return;
            }

            QJsonParseError parse_error{};
            const QJsonDocument doc = QJsonDocument::fromJson(response_body, &parse_error);
            if (parse_error.error != QJsonParseError::NoError || !doc.isObject()) {
                emit request_failed("bad json from lambda");
                reply->deleteLater();
                return;
            }

            const QJsonObject obj = doc.object();
            const QString download_url = obj.value("url").toString();
            const QString returned_object_key = obj.value("object_key").toString();

            if (download_url.isEmpty()) {
                emit request_failed("missing url");
                reply->deleteLater();
                return;
            }

            reply->deleteLater();
            emit download_url_ready(download_url, returned_object_key);
        });
    }

signals:
    void download_url_ready(const QString& download_url, const QString& object_key);
    void request_failed(const QString& error_text);

private:
    QString _lambda_url;
    QNetworkAccessManager _network_access_manager;
};
#endif // S3_DOWNLOAD_URL_CLIENT_H
