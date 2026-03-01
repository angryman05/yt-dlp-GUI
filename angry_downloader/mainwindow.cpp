#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "QFileDialog"
#include <QMessageBox>
#include <QStringList>
#include <QRegularExpression>
#include <QProcessEnvironment>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    ytProcess = new QProcess(this);

    connect(ytProcess,
            QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            &MainWindow::on_processFinished);

    connect(ui->btnConvert,
            &QPushButton::clicked,
            this,
            &MainWindow::on_btnConvertClicked);

    connect(ytProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::on_processReadyRead);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnConvertClicked()
{
    // Step 1: Get text from the text box
    QString url = ui->txtYtLink->toPlainText().trimmed();

    if (url.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a URL first!");
        return;
    }

    // Step 2: Open a dialog to choose the save folder
    QString saveFolder = QFileDialog::getExistingDirectory(this, "Select Save Folder");

    if (saveFolder.isEmpty()) {
        return; // User canceled the dialog
    }

    // Step 3: Get the format from the combo box
    // Make sure 'boxFormats' matches the objectName of your QComboBox
    QString format = ui->boxFormats->currentText().toLower();

    // Disable the button to prevent multiple clicks
    ui->btnConvert->setEnabled(false);
    ui->btnConvert->setText("Downloading...");

    // Step 4: Set up the arguments for yt-dlp
    QStringList arguments;

    // Output template
    QString outputPath = saveFolder + "/%(title)s.%(ext)s";
    arguments << "-o" << outputPath;

    // Format selection logic
    if (format == "mp3" || format == "wav" || format == "m4a" || format == "flac") {
        // If it's an audio format, we must extract the audio
        arguments << "-x" << "--audio-format" << format;
    } else {
        // If it's a video format, tell it to merge/remux into that container
        arguments << "--merge-output-format" << format;
    }


    // Reset progress bar
    ui->progressBar->setValue(0);

    arguments << "--ffmpeg-location" << "/usr/bin/ffmpeg";

    // Add the URL and force newlines for easy reading
    arguments << "--newline" << url; // <--- ADD "--newline" HERE


    // --- ADD THESE 3 LINES ---
    // Grab the system environment and strip out Qt's customized library paths
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.remove("LD_LIBRARY_PATH");
    ytProcess->setProcessEnvironment(env);
    // -------------------------

    // Start the process!
    ytProcess->start("yt-dlp", arguments);

    // Start the process!
    ytProcess->start("yt-dlp", arguments);
}

void MainWindow::on_processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // Reset the UI once the background process finishes
    ui->btnConvert->setEnabled(true);
    ui->btnConvert->setText("Download");

    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        QMessageBox::information(this, "Success", "Video downloaded successfully!");
        ui->txtYtLink->clear();
    } else {
        // Grab the actual error message from yt-dlp
        QString errorDetails = QString(ytProcess->readAllStandardError()).trimmed();

        // If stderr is empty, grab stdout just in case
        if (errorDetails.isEmpty()) {
            errorDetails = QString(ytProcess->readAllStandardOutput()).trimmed();
        }

        QMessageBox::critical(this, "Error", "The download failed. Details:\n\n" + errorDetails);
    }

    ui->progressBar->setValue(0);
}


void MainWindow::on_processReadyRead()
{
    // Read whatever new text yt-dlp just outputted
    QString output = QString(ytProcess->readAllStandardOutput());

    // Use Regex to find "[download]  XX.X%"
    // R"(...)" is a raw string so we don't have to double-escape slashes
    QRegularExpression re(R"(\[download\]\s+([\d\.]+)%)");

    // Search the output
    QRegularExpressionMatch match = re.match(output);

    if (match.hasMatch()) {
        // Extract the number (e.g., "45.3"), convert to double, then to int
        double progressPercentage = match.captured(1).toDouble();

        // Update the progress bar!
        ui->progressBar->setValue(static_cast<int>(progressPercentage));
    }
}
