#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QString>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onOpenFile();


private:
    void showAudioInfo(const QString &filePath);

    QPushButton *m_openButton;
    QPushButton *m_infoButton;
};