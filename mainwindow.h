#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include "fileprocess.h"
#include "QThread"
#include <QProgressBar>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QTimer>
//пока так, потом добавил чек боксы и т.д

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onBrowseInputClicked();//обзор исходной папки
    void onBrowseOutClicked();//обзор папки сохранения
    void onStartClick();// старт работы программы
    void onStopClick();//стоп работы программы
    void onPauseClick();//закончить работы программы
    void updateProgress(int percent);
    void updateStatus(const QString &message);
    void onFileProcessed(const QString &fileName);
    void onProcessingFinished();
    void onTimerTimeout();

private:
    Ui::MainWindow *ui;

    //указатели
    QLineEdit *inEdit;
    QLineEdit *outEdit;
    QLineEdit *maskaEdit;
    QLineEdit *keyEdit;

    QPushButton *StartButton;
    QPushButton *stopButton;
    QPushButton *PauseButton;
    QPushButton *InputButton;
    QPushButton *OutPutButton;

    QProgressBar *progressBar;
    QPlainTextEdit *plainTextEdit;

    QCheckBox *deleteOriginalCheck;

    QThread *m_thread;
    QTimer *m_timer;
    FileProcess *m_processor;
};
#endif // MAINWINDOW_H
