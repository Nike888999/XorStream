#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QFileDialog"
#include "QMessageBox"
#include "fileprocess.h"
#include "QThread"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    inEdit = ui -> InEdit;
    StartButton = ui -> StartButton;
    stopButton = ui -> stopButton;
    PauseButton = ui ->PauseButton;
    InputButton = ui -> InputButton;
    progressBar = ui -> progressBar;
    plainTextEdit = ui -> plainTextEdit;
    OutPutButton = ui -> OutPutButton;
    deleteOriginalCheck = ui -> deleteOriginalCheck;


    m_thread = new QThread(this);
    m_processor = new FileProcess();
    m_processor->moveToThread(m_thread);
    m_timer = new QTimer(this);

    //подключение сигнала к слотам
    connect(InputButton, &QPushButton::clicked, this, &MainWindow::onBrowseInputClicked);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::onStopClick);
    connect(PauseButton, &QPushButton::clicked, this, &MainWindow::onPauseClick);
    connect(m_thread, &QThread::started, m_processor, &FileProcess::process);
    connect(m_processor, &FileProcess::finished, m_thread, &QThread::quit);
    connect(m_processor, &FileProcess::finished, this, &MainWindow::onProcessingFinished);
    connect(m_processor, &FileProcess::progressUpdate, this, &MainWindow::updateProgress);
    connect(m_processor, &FileProcess::statusUpdate, this, &MainWindow::updateStatus);
    connect(m_processor, &FileProcess::fileProcessed, this, &MainWindow::onFileProcessed);
    connect(OutPutButton, &QPushButton::clicked, this, &MainWindow::onBrowseOutClicked);
    connect(StartButton, &QPushButton::clicked, this, &MainWindow::onStartClick);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::onTimerTimeout);
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onBrowseInputClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Выбирете папку");
    if(!dir.isEmpty())
    {
        inEdit->setText(dir);
    }
}

void MainWindow::onBrowseOutClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Выберите папку для сохранения");
    if(!dir.isEmpty())
    {
        ui->OutEdit->setText(dir);
    }
}

void MainWindow::onStartClick()
{
    QString sourcePath = ui ->InEdit->text();
    QString destPath = ui -> OutEdit->text();
    QString mask = ui -> maskaEdit -> text();
    QString key = ui -> keyEdit -> text();



    if(sourcePath.isEmpty()||destPath.isEmpty()||mask.isEmpty()||key.isEmpty())
    {
        QMessageBox::warning(this,"Ошибка", "Заполните все поля");
        return;
    }
    if(key.length()!= 16)
    {
        QMessageBox::warning(this, "Ошибка", "Ключ не верный");
    }
    QByteArray keyBytes = QByteArray::fromHex(key.toUtf8());


    // Запускаем поток
    m_thread->start();

    // Блокируем кнопки
    ui->StartButton->setEnabled(true);
    ui->PauseButton->setEnabled(true);
    ui->stopButton->setEnabled(true);
    bool deleteOriginal = ui -> deleteOriginalCheck ->isChecked();
    m_processor->setParams(sourcePath, destPath, mask, keyBytes, deleteOriginal);

}
void MainWindow::onStopClick()
{
    if(m_processor)
    {
        m_processor->stop();   // Устанавливаем флаг остановки
        ui->plainTextEdit->appendPlainText("⏹ Остановка...");

        // Разблокируем кнопки (они разблокируются в onProcessingFinished, но на всякий случай)
        ui->StartButton->setEnabled(true);
        ui->PauseButton->setEnabled(true);
        ui->stopButton->setEnabled(false);

        // Останавливаем таймер, если он был запущен
        if(m_timer->isActive())
        {
            m_timer->stop();
            ui->plainTextEdit->appendPlainText("⏹ Таймер остановлен");
        }
    }
}
void MainWindow::onPauseClick()
{
    if(m_processor)
    {
        m_processor->pause();   // Переключаем флаг паузы

        // Меняем текст кнопки
        if(ui->PauseButton->text() == "⏸ Пауза")
        {
            ui->PauseButton->setText("▶ Продолжить");
            ui->plainTextEdit->appendPlainText("⏸ Пауза");
        }
        else
        {
            ui->PauseButton->setText("⏸ Пауза");
            ui->plainTextEdit->appendPlainText("▶ Продолжено");
        }
    }
}

void MainWindow::updateProgress(int percent)
{
    // ui->progressBar->setValue(percent);
    // qDebug() << "📊 Прогресс обновлён:" << percent << "%";
    // ui->progressBar->setValue(percent);

    qDebug() << "📊 Прогресс обновлён:" << percent << "%";
    if (ui->progressBar) {
        ui->progressBar->setValue(percent);
        ui->progressBar->update();      // ← принудительное обновление
        ui->progressBar->repaint();     // ← перерисовка
        qDebug() << "   Значение прогресс-бара установлено в:" << ui->progressBar->value();
    } else {
        qDebug() << "❌ progressBar is nullptr!";
    }
}

void MainWindow::updateStatus(const QString &message)
{
    ui->statusLabel->setText(message);
    ui->plainTextEdit->appendPlainText("[Статус] " + message);
}

void MainWindow::onFileProcessed(const QString &fileName)
{
    ui->plainTextEdit->appendPlainText("[Файл] " + fileName + " обработан");
}

void MainWindow::onProcessingFinished()
{
    ui->StartButton->setEnabled(true);
    ui->PauseButton->setEnabled(true);
    ui->stopButton->setEnabled(true);
    ui->progressBar->setValue(0);
    //m_timer->stop();
    QString mode = ui->modeComboBox ->currentText();
    if(mode == "По таймеру")
    {
        int interval = ui->timerSpinBox->value(); // Читаем значение из поля "Интервал (сек)"
        if(interval > 0) // Если интервал больше нуля
        {
            m_timer->start(interval * 1000); // Запускаем таймер (переводим секунды в миллисекунды)
            ui-> plainTextEdit-> appendPlainText("⏰ Таймер запущен на" + QString::number(interval) + "сек");
        }
    }
    qDebug() << "Режим из комбобокса:" << mode;
    qDebug() << "Текст сравнения: 'По таймеру'";
    qDebug() << "Совпадают?" << (mode == "По таймеру");

    QMessageBox::information(this, "Готово", "Обработка завершена!");
}
void MainWindow::onTimerTimeout()
{
    qDebug()<<"Таймер сработал";
    onStartClick();
}

