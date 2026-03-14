#ifndef PRESIGNED_URL_TEXT_FILE_DOWNLOADER_H
#define PRESIGNED_URL_TEXT_FILE_DOWNLOADER_H
#pragma once

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSaveFile>
#include <QString>
#include <QUrl>

/**
 * @brief Downloads a text file from S3 using a presigned GET URL.
 */
class Presigned_Url_Text_File_Downloader : public QObject
{
    Q_OBJECT

public:
    explicit Presigned_Url_Text_File_Downloader(QObject* parent = nullptr);

    /**
     * @brief Downloads a text file using a presigned S3 GET URL.
     * @param download_url Presigned GET URL.
     * @param output_file_path Local output file path.
     * @param object_key Optional S3 object key for reporting.
     */
    void download_text_file(
        const QString& download_url,
        const QString& output_file_path,
        const QString& object_key = {});

signals:
    /**
     * @brief Emitted when download completes.
     * @param output_file_path Local file written.
     * @param object_key Reported object key, if provided.
     */
    void download_complete(const QString& output_file_path, const QString& object_key);

    /**
     * @brief Emitted when download fails.
     * @param error_text Error description.
     */
    void download_failed(const QString& error_text);

private:
    void handle_download_finished(QNetworkReply* reply);

private:
    QString _output_file_path;
    QString _object_key;
    QNetworkAccessManager _network_access_manager;
};
#endif // PRESIGNED_URL_TEXT_FILE_DOWNLOADER_H
