#include "fileprocess.h"
#include "QFile"
#include "QDir"
#include <QThread>
#include <QStringList>

FileProcess::FileProcess(QObject *parent)
    : QObject{parent}
{

}

void FileProcess::setParams(const QString &sourcePath,
                   const QString &destPath,
                   const QString &mask,
                   const QByteArray &key,
                            bool deleteOriginal)
{
    m_sourcePath = sourcePath;
    m_destPath = destPath;
    m_mask = mask;
    m_key = key;
    m_deleteOriginal = deleteOriginal;
}

void FileProcess::stop()
{
    m_isStopped = true;
}

void FileProcess::pause()
{
    m_isPause = !m_isPause;
    if(m_isPause)
    {
        emit statusUpdate("II пауза");
    }
    else
    {
        emit statusUpdate("Продолжение");
    }
}
void FileProcess::process()
{
    emit statusUpdate("Поиск файлов");
    QDir dir(m_sourcePath);
    if(!dir.exists())
    {
        emit statusUpdate("Такой папки не существует");
        emit finished();
        return;
    }

    QStringList files = dir.entryList(QStringList () << m_mask, QDir::Files);
    int totalFiles = files.size();
    int processedFiles = 0;

    if(files.isEmpty())
    {
        emit statusUpdate("Нет файлов для обработки");
        emit finished();
        return;
    }

    QDir destDir(m_destPath);
        if(!destDir.exists())
    {
        emit statusUpdate("Не получилось создать папку для результатов");
            emit finished();
        return;
    }
        emit statusUpdate("📂 Найдено файлов: " + QString::number(totalFiles));

        for(const QString &fileName : files)
        {
            if(m_isStopped)
            {
                emit statusUpdate("Остановлено пользователем");
                break;
            }

            while(m_isPause)
            {
                QThread::msleep(100);
                if(m_isStopped) break;
            }
            if(m_isStopped)break;

            QString inputFile = m_sourcePath + "/" + fileName;
            QString outputFile = m_destPath + "/" + fileName;

            emit statusUpdate("обработка" + fileName);
            emit fileProcessed(fileName);

            if(processSingleFile(inputFile, outputFile))
            {
                processedFiles++;

                // ===== УДАЛЕНИЕ ИСХОДНОГО ФАЙЛА =====
                if(m_deleteOriginal)
                {
                    if(QFile::remove(inputFile))
                    {
                        qDebug() << "  🗑️ Исходный файл удалён:" << inputFile;
                    }
                    else
                    {
                        qDebug() << "  ❌ Не удалось удалить:" << inputFile;
                    }
                }
            }
            else
            {
                emit statusUpdate("Ошибка при обработке" + fileName);
            }



            int percent = (processedFiles * 100) / totalFiles;
            emit progressUpdate(percent);
            qDebug() << "Обработан файл:" << fileName << ", осталось:" << (files.size() - processedFiles);
        }

        emit statusUpdate("Готово. Обработано: " + QString::number(processedFiles));
        emit progressUpdate(100);
        emit finished();

}

bool FileProcess::processSingleFile(const QString &inputPath, const QString &outputPath)
{
    QFile inFile(inputPath);
    QFile outFile(outputPath);

    if(!inFile.open(QIODevice::ReadOnly))
    {
        return false;
    }
    if(!outFile.open(QIODevice::WriteOnly))
    {
        inFile.close();
        return false;
    }
    const int bufferSize = 1024 * 1024;
    QByteArray buffer;

    while (!inFile.atEnd())
    {
        //проверка остановки/паузы
        if(m_isStopped)
        {
            outFile.close();
            inFile.close();
            return false;
        }
        while(m_isPause)
        {
            QThread::msleep(100);
            if(m_isStopped)
            {
                outFile.close();
                inFile.close();
                return false;
            }
        }
        buffer = inFile.read(bufferSize);

        //применение XOR
        if(!m_key.isEmpty())
        {
            int keySize = m_key.size();
            for(int i = 0; i < buffer.size(); i++)
            {
                buffer[i] = buffer[i] ^ m_key[i % keySize];
            }
        }
        outFile.write(buffer);
    }
    inFile.close();
    outFile.close();
    return true;
}



