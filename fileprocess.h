#ifndef FILEPROCESS_H
#define FILEPROCESS_H

#include <QObject>
#include <QString>

class FileProcess : public QObject
{
    Q_OBJECT
public:
    explicit FileProcess(QObject *parent = nullptr);

    void setParams(const QString &sourcePath,
                   const QString &destPath,
                   const QString &mask,
                   const QByteArray &key,
                   bool deleteOriginal,
                   bool overwrite);
    void stop();
    void pause();

signals:
    void progressUpdate(int perсent); //прогресс
    void statusUpdate(const QString &message); //ссылка на сообщение о статусе
    void finished(); //конец
    void fileProcessed(const QString &fileName);//уведомление что файл закончен
    void fileProgress(int percent); //отображение прогресса в реалтайм

  public  slots:
    void process();
  private:
    QString m_sourcePath;
    QString m_destPath;
    QString m_mask;
    QByteArray m_key;


    bool m_isStopped = false;
    bool m_isPause = false;
    bool processSingleFile(const QString &inputPath, const QString &outputPath);
    bool m_deleteOriginal = false;
    bool m_overwrite = true;
};

#endif // FILEPROCESS_H
