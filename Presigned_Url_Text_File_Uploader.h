#ifndef PRESIGNED_URL_TEXT_FILE_UPLOADER_H
#define PRESIGNED_URL_TEXT_FILE_UPLOADER_H
#pragma once

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QUrl>


/**
 * @brief Uploads a local text file to S3 using a presigned PUT URL.
 */
class Presigned_Url_Text_File_Uploader : public QObject
{
    Q_OBJECT

public:
    explicit Presigned_Url_Text_File_Uploader(QObject* parent = nullptr);

    /**
     * @brief Uploads a local text file using a presigned S3 PUT URL.
     * @param local_file_path Local text file path.
     * @param upload_url Presigned PUT URL.
     * @param object_key Optional S3 object key for reporting.
     */
    void upload_text_file(
        const QString& local_file_path,
        const QString& upload_url,
        const QString& object_key = {});

signals:
    /**
     * @brief Emitted when upload completes.
     * @param object_key Reported object key, if provided.
     */
    void upload_complete(const QString& object_key);

    /**
     * @brief Emitted when upload fails.
     * @param error_text Error description.
     */
    void upload_failed(const QString& error_text);

private:
    void handle_upload_finished(QNetworkReply* reply);

private:
    QString _object_key;
    QNetworkAccessManager _network_access_manager;
};
#endif // PRESIGNED_URL_TEXT_FILE_UPLOADER_H
