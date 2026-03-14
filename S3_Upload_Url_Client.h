#ifndef S3_UPLOAD_URL_CLIENT_H
#define S3_UPLOAD_URL_CLIENT_H
#pragma once

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include "AR_ASSERT.h"

class S3_Upload_Client : public QObject
{
    Q_OBJECT

public:
    explicit S3_Upload_Client(const QString& lambda_url, QObject* parent = nullptr)
        : QObject(parent)
        , _lambda_url(lambda_url)
    {
        AR_ASSERT(!_lambda_url.isEmpty());
    }

    void request_upload_url(const QString& app_name, const QString& instance_id)
    {
        AR_ASSERT(!_lambda_url.isEmpty());
        AR_ASSERT(!app_name.isEmpty());
        AR_ASSERT(!instance_id.isEmpty());

        if (_lambda_url.isEmpty() || app_name.isEmpty() || instance_id.isEmpty()) {
            emit request_failed("invalid arguments");
            return;
        }

        QJsonObject request_obj;
        request_obj["action"] = "request_upload_url";
        request_obj["app_name"] = app_name;
        request_obj["instance_id"] = instance_id;

        const QByteArray request_body =
            QJsonDocument(request_obj).toJson(QJsonDocument::Compact);

        QNetworkRequest request{ QUrl{ _lambda_url } };
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QNetworkReply* reply = _network_access_manager.post(request, request_body);
        AR_ASSERT(reply != nullptr);

        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            const QByteArray response_body = reply->readAll();
            const int http_status =
                reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            if (reply->error() != QNetworkReply::NoError) {
                const QString error_text =
                    QString("request failed: %1, http_status=%2, body=%3")
                        .arg(reply->errorString())
                        .arg(http_status)
                        .arg(QString::fromUtf8(response_body));

                reply->deleteLater();
                emit request_failed(error_text);
                return;
            }

            QJsonParseError parse_error{};
            const QJsonDocument doc = QJsonDocument::fromJson(response_body, &parse_error);
            if (parse_error.error != QJsonParseError::NoError || !doc.isObject()) {
                reply->deleteLater();
                emit request_failed("bad json from lambda");
                return;
            }

            const QJsonObject response_obj = doc.object();
            const QString upload_url = response_obj.value("url").toString();
            const QString object_key = response_obj.value("object_key").toString();

            if (upload_url.isEmpty() || object_key.isEmpty()) {
                reply->deleteLater();
                emit request_failed("lambda response missing url or object_key");
                return;
            }

            _upload_url = upload_url;
            _object_key = object_key;

            reply->deleteLater();
            emit upload_url_ready(_upload_url, _object_key);
        });
    }

    void upload_text(const QByteArray& text_data)
    {
        AR_ASSERT(!_upload_url.isEmpty());
        AR_ASSERT(!text_data.isEmpty());

        QNetworkRequest request{ QUrl{ _upload_url } };
        request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");

        QNetworkReply* reply = _network_access_manager.put(request, text_data);
        AR_ASSERT(reply != nullptr);

        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            if (reply->error() != QNetworkReply::NoError) {
                emit upload_failed(reply->errorString());
                reply->deleteLater();
                return;
            }

            emit upload_complete(_object_key);
            reply->deleteLater();
        });
    }

    const QString& upload_url() const
    {
        return _upload_url;
    }

    const QString& object_key() const
    {
        return _object_key;
    }

signals:
    void upload_url_ready(const QString& upload_url, const QString& object_key);
    void request_failed(const QString& error_text);
    void upload_complete(const QString& object_key);
    void upload_failed(const QString& error_text);

private:
    QString _lambda_url;
    QString _upload_url;
    QString _object_key;
    QNetworkAccessManager _network_access_manager;
};
#endif // S3_UPLOAD_URL_CLIENT_H
