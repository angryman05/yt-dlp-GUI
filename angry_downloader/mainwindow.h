#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>

QT_BEGIN_NAMESPACE

namespace Ui {class MainWindow;}

QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnConvertClicked();
    void on_processFinished(int exitCode,QProcess::ExitStatus exitStatus);
    void on_processReadyRead();

private:
    Ui::MainWindow *ui;
    QProcess* ytProcess;
};
#endif // MAINWINDOW_H
