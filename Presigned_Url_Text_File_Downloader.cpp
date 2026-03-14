#include "Presigned_Url_Text_File_Downloader.h"
#include "AR_ASSERT.h"

/**
 * @brief Constructs a downloader.
 * @param parent Qt parent object.
 */
Presigned_Url_Text_File_Downloader::Presigned_Url_Text_File_Downloader(QObject* parent)
    : QObject(parent)
{
}

void Presigned_Url_Text_File_Downloader::download_text_file(
    const QString& download_url,
    const QString& output_file_path,
    const QString& object_key)
{
    AR_ASSERT(!download_url.isEmpty());
    AR_ASSERT(!output_file_path.isEmpty());

    if (download_url.isEmpty()) {
        emit download_failed("download_url is empty");
        return;
    }

    if (output_file_path.isEmpty()) {
        emit download_failed("output_file_path is empty");
        return;
    }

    _output_file_path = output_file_path;
    _object_key = object_key;

    QNetworkRequest request{ QUrl{ download_url } };

    QNetworkReply* reply = _network_access_manager.get(request);
    AR_ASSERT(reply != nullptr);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handle_download_finished(reply);
    });
}

void Presigned_Url_Text_File_Downloader::handle_download_finished(QNetworkReply* reply)
{
    AR_ASSERT(reply != nullptr);
    if (reply == nullptr) {
        emit download_failed("null reply during download");
        return;
    }

    const QByteArray response_body = reply->readAll();
    const int http_status =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (reply->error() != QNetworkReply::NoError) {
        const QString error_text =
            QString("download failed: %1, http_status=%2, body=%3")
                .arg(reply->errorString())
                .arg(http_status)
                .arg(QString::fromUtf8(response_body));
        reply->deleteLater();
        emit download_failed(error_text);
        return;
    }

    QSaveFile file(_output_file_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        reply->deleteLater();
        emit download_failed(QString("failed to open output file: %1").arg(_output_file_path));
        return;
    }

    const qint64 bytes_written = file.write(response_body);
    if (bytes_written != response_body.size()) {
        file.cancelWriting();
        reply->deleteLater();
        emit download_failed(QString("failed to write output file: %1").arg(_output_file_path));
        return;
    }

    if (!file.commit()) {
        reply->deleteLater();
        emit download_failed(QString("failed to commit output file: %1").arg(_output_file_path));
        return;
    }

    reply->deleteLater();
    emit download_complete(_output_file_path, _object_key);
}