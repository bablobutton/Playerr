#include "MainWindow.hpp"
#include "AudioDecoder.hpp"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Размерчик
    resize(400, 300);
    
    // Кнопочка
    m_button = new QPushButton("BABLO", this);
    m_button->setGeometry(150, 120, 100, 30);
    
    // Подключение сигнала к слоту
    connect(m_button, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
}

MainWindow::~MainWindow()
{

}

void MainWindow::onButtonClicked()
{
    QMessageBox::information(this, "Сообщение", "BABLO BABLO!");
}