#include "MainWindow.hpp"
#include "AudioDecoder.hpp"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>
#include <memory>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    auto central = std::make_unique<QWidget>(this);
    auto layout = std::make_unique<QVBoxLayout>(central.get());
    
    m_openButton = new QPushButton(tr("Open Audio File"), this);
    layout->addWidget(m_openButton);
    connect(m_openButton, &QPushButton::clicked, this, &MainWindow::onOpenFile);

    setCentralWidget(central.release());

    resize(400, 200);
}

MainWindow::~MainWindow() {}

// Затестим tr() - переводит строку под язык пользователя

void MainWindow::onOpenFile() {
    const QString filter = tr("Audio Files (*.mp3 *.wav *.flac);;All Files (*)");
    const QString filePath = QFileDialog::getOpenFileName(this, tr("Select Audio File"), QDir::homePath(), filter);
    if (!filePath.isEmpty()) {
        showAudioInfo(filePath);
    }
}

void MainWindow::showAudioInfo(const QString &filePath) {
    AudioDecoder decoder;
    if (!decoder.open(filePath)) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to open audio file."));
        return;
    }

    QString info;
    info += tr("File: %1\n").arg(filePath);
    info += tr("Sample Rate: %1 Hz\n").arg(decoder.sampleRate());
    info += tr("Channels: %1\n").arg(decoder.channels());
    info += tr("Bytes Per Sample: %1\n").arg(decoder.bytesPerSample());
    info += tr("Bitrate: %1 bps\n").arg(decoder.bitrate());
    info += tr("Duration: %1 s\n").arg(decoder.durationUs() / 1e6, 0, 'f', 2);
    info += tr("Sample Format: %1").arg(decoder.sampleFormatName());

    QMessageBox::information(this, tr("Audio File Info"), info);
}
