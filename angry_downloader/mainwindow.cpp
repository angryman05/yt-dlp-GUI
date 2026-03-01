#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "QFileDialog"
#include <QMessageBox>
#include <QStringList>
#include <QRegularExpression>
#include <QProcessEnvironment>
#include <QSettings>
#include <QDir>
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // --- LOAD SAVED SETTINGS ON STARTUP ---
    QSettings settings("MyCoolApps", "YtDlpGUI");
    m_saveFolder = settings.value("lastFolder", "").toString();

    if (!m_saveFolder.isEmpty()) {
        ui->lblPath->setText(m_saveFolder);
    } else {
        ui->lblPath->setText("No folder selected");
    }

    ytProcess = new QProcess(this);

    // --- SET UP THUMBNAIL PROCESS ---
    thumbProcess = new QProcess(this);

    // We will save the temp image as "yt_temp_thumb.jpg" in your OS's temp folder
    m_tempThumbPath = QDir::tempPath() + "/yt_temp_thumb.jpg";

    connect(thumbProcess,
            QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            &MainWindow::on_thumbProcessFinished);
    // --------------------------------

    connect(ytProcess,
            QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            &MainWindow::on_processFinished);

    connect(ui->btnConvert,
            &QPushButton::clicked,
            this,
            &MainWindow::on_btnConvertClicked);

    connect(ytProcess,
            &QProcess::readyReadStandardOutput,
            this,
            &MainWindow::on_processReadyRead);

    connect(ui->btnFileDest,
            &QPushButton::clicked,
            this,
            &MainWindow::on_btnFileDestClicked
            );

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnFileDestClicked()
{
    // Step 2: Open a dialog to choose the save folder
    QString saveFolder = QFileDialog::getExistingDirectory(this, "Select Save Folder");

    if (!saveFolder.isEmpty()) {
        m_saveFolder = saveFolder;
        ui->lblPath->setText(saveFolder);

        // --- SAVE TO SETTINGS ---
        QSettings settings("MyCoolApps", "YtDlpGUI");
        settings.setValue("lastFolder", m_saveFolder);
    }

}

void MainWindow::on_btnConvertClicked()
{
    // Step 1: Get text from the text box
    QString url = ui->txtYtLink->toPlainText().trimmed();


    if (url.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a URL first!");
        return;
    }

    // --- ADD THIS SAFETY CHECK ---
    if (m_saveFolder.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select a save folder first!");
        return;
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
    QString outputPath = m_saveFolder + "/%(title)s.%(ext)s";
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

    // --- FETCH THE THUMBNAIL ---
    // Clear the old thumbnail and show a loading message
    ui->lblThumbnail->clear();
    ui->lblThumbnail->setText("Loading thumbnail...");
    ui->lblTitle->clear();
    ui->lblTitle->setText("Loading title");

    QStringList thumbArgs;

    // 1. Tell yt-dlp to get the thumbnail, make it a JPG, and skip the video
    thumbArgs << "--write-thumbnail" << "--convert-thumbnails" << "jpg" << "--skip-download";

    // 2. Make it quiet, print the title, force overwrite, and ADD --no-simulate
    // so it actually saves the picture while printing the text!
    thumbArgs << "-q" << "--no-warnings" << "--print" << "title" << "--force-overwrites" << "--no-simulate";

    // 3. Output path
    thumbArgs << "-o" << QDir::tempPath() + "/yt_temp_thumb";

    thumbArgs << "--ffmpeg-location" << "/usr/bin/ffmpeg";
    thumbArgs << url;

    // Use the exact same clean environment we created earlier
    thumbProcess->setProcessEnvironment(env);

    QFile::remove(m_tempThumbPath);

    // Start fetching the thumbnail!
    thumbProcess->start("yt-dlp", thumbArgs);
    // ---------------------------

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

void MainWindow::on_thumbProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {

        // --- GRAB THE TITLE ---
        // Read the standard output, which now perfectly contains our title!
        QString videoTitle = QString(thumbProcess->readAllStandardOutput()).trimmed();

        // If it found a title, put it on the label
        if (!videoTitle.isEmpty()) {
            ui->lblTitle->setText(videoTitle);
        } else {
            ui->lblTitle->setText("Unknown Title");
        }
        // ----------------------

        // Load the image from the temp folder
        QPixmap thumbnail(m_tempThumbPath);

        if (!thumbnail.isNull()) {
            // Scale the image to perfectly fit the label you drew in Qt Designer,
            // keeping the correct aspect ratio and smoothing the pixels!
            ui->lblThumbnail->setPixmap(thumbnail.scaled(ui->lblThumbnail->size(),
                                                         Qt::KeepAspectRatio,
                                                         Qt::SmoothTransformation));
        } else {
            ui->lblThumbnail->setText("Image downloaded but invalid.");
        }
    } else {
        ui->lblThumbnail->setText("Failed to load thumbnail.");
    }
}
