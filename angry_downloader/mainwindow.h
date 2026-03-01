#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QPixmap> // <--- ADD THIS for image handling

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
    void on_btnFileDestClicked();
    void on_thumbProcessFinished(int exitCode,QProcess::ExitStatus exitStatus);

private:
    Ui::MainWindow *ui;
    QProcess* ytProcess;
    QString m_saveFolder;
    QProcess* thumbProcess;
    QString m_tempThumbPath;
};
#endif // MAINWINDOW_H
