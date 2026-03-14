#include "Presigned_Url_Text_File_Uploader.h"

#include <QFile>
#include "AR_ASSERT.h"

/**
 * @brief Constructs an uploader.
 * @param parent Qt parent object.
 */
Presigned_Url_Text_File_Uploader::Presigned_Url_Text_File_Uploader(QObject* parent)
    : QObject(parent)
{
}

void Presigned_Url_Text_File_Uploader::upload_text_file(
    const QString& local_file_path,
    const QString& upload_url,
    const QString& object_key)
{
    AR_ASSERT(!local_file_path.isEmpty());
    AR_ASSERT(!upload_url.isEmpty());

    if (local_file_path.isEmpty()) {
        emit upload_failed("local_file_path is empty");
        return;
    }

    if (upload_url.isEmpty()) {
        emit upload_failed("upload_url is empty");
        return;
    }

    QFile file(local_file_path);
    if (!file.exists()) {
        emit upload_failed(QString("file does not exist: %1").arg(local_file_path));
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit upload_failed(QString("failed to open file: %1").arg(local_file_path));
        return;
    }

    const QByteArray text_data = file.readAll();
    file.close();

    if (text_data.isEmpty()) {
        emit upload_failed(QString("file is empty: %1").arg(local_file_path));
        return;
    }

    _object_key = object_key;

    QNetworkRequest request{ QUrl{ upload_url } };
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");

    QNetworkReply* reply = _network_access_manager.put(request, text_data);
    AR_ASSERT(reply != nullptr);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handle_upload_finished(reply);
    });
}

void Presigned_Url_Text_File_Uploader::handle_upload_finished(QNetworkReply* reply)
{
    AR_ASSERT(reply != nullptr);
    if (reply == nullptr) {
        emit upload_failed("null reply during upload");
        return;
    }

    const QByteArray response_body = reply->readAll();
    const int http_status =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (reply->error() != QNetworkReply::NoError) {
        const QString error_text =
            QString("upload failed: %1, http_status=%2, body=%3")
                .arg(reply->errorString())
                .arg(http_status)
                .arg(QString::fromUtf8(response_body));
        reply->deleteLater();
        emit upload_failed(error_text);
        return;
    }

    reply->deleteLater();
    emit upload_complete(_object_key);
}