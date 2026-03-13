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
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDateTime>
#include <QtGlobal>
#include <QString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // --- SETUP PERMANENT CACHE FOLDER ---
    // This finds the standard Linux/Windows user data folder and creates a folder just for our app
    m_appDataFolder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/YtDlpGUI/Thumbnails";

    QDir dir;
    if (!dir.exists(m_appDataFolder)) {
        dir.mkpath(m_appDataFolder); // Create the folder if it doesn't exist!
    }



    QSettings settings("MyCoolApps", "YtDlpGUI");
    m_saveFolder = settings.value("lastFolder", "").toString();

    if (!m_saveFolder.isEmpty()) {
        ui->lblPath->setText(m_saveFolder);
    } else {
        ui->lblPath->setText("No folder selected");
    }


    loadHistory();
    updateHistoryUI();

    ytProcess = new QProcess(this);


    thumbProcess = new QProcess(this);


    m_tempThumbPath = QDir::tempPath() + "/yt_temp_thumb.jpg";

    connect(thumbProcess,
            QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            &MainWindow::on_thumbProcessFinished);

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

    connect(ui->btnClearHistory,
            &QPushButton::clicked,
            this,
            &MainWindow::on_btnClearHistoryClicked);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnFileDestClicked()
{

    QString saveFolder = QFileDialog::getExistingDirectory(this, "Select Save Folder");

    if (!saveFolder.isEmpty()) {
        m_saveFolder = saveFolder;
        ui->lblPath->setText(saveFolder);

        QSettings settings("MyCoolApps", "YtDlpGUI");
        settings.setValue("lastFolder", m_saveFolder);
    }

}

QString getFfmpegPath()
{
#ifdef Q_OS_WIN
    return QCoreApplication::applicationDirPath() + "/tools/ffmpeg.exe";
#elif defined(Q_OS_LINUX)
    return QCoreApplication::applicationDirPath() + "/tools/ffmpeg";
#else
    return "ffmpeg";
#endif
}

QString getYtDlpPath()
{
#ifdef Q_OS_WIN
    return QCoreApplication::applicationDirPath() + "/tools/yt-dlp.exe";
#elif defined(Q_OS_LINUX)
    return QCoreApplication::applicationDirPath() + "/tools/yt-dlp";
#else
    return "yt-dlp";
#endif
}

void MainWindow::on_btnConvertClicked()
{

    QString url = ui->txtYtLink->toPlainText().trimmed();


    if (url.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a URL first!");
        return;
    }

    if (m_saveFolder.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select a save folder first!");
        return;
    }


    QString format = ui->boxFormats->currentText().toLower();

    ui->btnConvert->setEnabled(false);
    ui->btnConvert->setText("Downloading...");

    QStringList arguments;


    QString outputPath;

    if (ui->chkPlaylist->isChecked()) {
        arguments << "--yes-playlist";
        outputPath = m_saveFolder + "/%(playlist_title)s/%(playlist_index)s - %(title)s.%(ext)s";
    } else {
        arguments << "--no-playlist";
        outputPath = m_saveFolder + "/%(title)s.%(ext)s";
    }

    arguments << "-o" << outputPath;

    // Format selection logic
    if (format == "mp3" || format == "wav" || format == "m4a" || format == "flac" || format == "ogg") {
        arguments << "-x" << "--audio-format" << format;
    } else {
        arguments << "--merge-output-format" << format;
    }

    ui->progressBar->setValue(0);

    arguments << "--ffmpeg-location" << getFfmpegPath();

    arguments << "--newline" << url;

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.remove("LD_LIBRARY_PATH");
    ytProcess->setProcessEnvironment(env);

    ui->lblThumbnail->clear();
    ui->lblThumbnail->setText("Loading thumbnail...");
    ui->lblTitle->clear();
    ui->lblTitle->setText("Loading title");

    QStringList thumbArgs;

    //thumbnail logic
    thumbArgs << "--write-thumbnail" << "--convert-thumbnails" << "jpg" << "--skip-download";

    thumbArgs << "-q" << "--no-warnings" << "--print" << "title" << "--force-overwrites" << "--no-simulate";

    thumbArgs << "-o" << QDir::tempPath() + "/yt_temp_thumb";

    thumbArgs << "--ffmpeg-location" << getFfmpegPath();
    thumbArgs << url;

    thumbProcess->setProcessEnvironment(env);

    QFile::remove(m_tempThumbPath);

    thumbProcess->start(getYtDlpPath(), thumbArgs);

    ytProcess->start(getYtDlpPath(), arguments);

}

void MainWindow::on_processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    ui->btnConvert->setEnabled(true);
    ui->btnConvert->setText("Download");

    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        QMessageBox::information(this, "Success", "Video downloaded successfully!");

        QString videoTitle = ui->lblTitle->text();

        if (ui->chkPlaylist->isChecked()) {
            videoTitle = "🗂️ [Playlist] " + videoTitle;
        }

        saveToHistory(videoTitle, m_saveFolder);


        ui->txtYtLink->clear();
    } else {
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
    QString output = QString(ytProcess->readAllStandardOutput());

    // Use Regex to find "[download]  XX.X%"
    // R"(...)" is a raw string so we don't have to double-escape slashes
    QRegularExpression re(R"(\[download\]\s+([\d\.]+)%)");

    QRegularExpressionMatch match = re.match(output);

    if (match.hasMatch()) {
        // Extract the number (e.g., "45.3"), convert to double, then to int
        double progressPercentage = match.captured(1).toDouble();

        // Update the progress bar
        ui->progressBar->setValue(static_cast<int>(progressPercentage));
    }
}

void MainWindow::on_thumbProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {

        // read the title
        QString videoTitle = QString(thumbProcess->readAllStandardOutput()).trimmed();

        if (!videoTitle.isEmpty()) {
            ui->lblTitle->setText(videoTitle);
        } else {
            ui->lblTitle->setText("Unknown Title");
        }

        // Load the image from the temp folder
        QPixmap thumbnail(m_tempThumbPath);

        if (!thumbnail.isNull()) {
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

void MainWindow::saveToHistory(QString title, QString savedPath)
{
    // Create a permanent copy of the thumbnail
    QString uniqueName = QString::number(QDateTime::currentMSecsSinceEpoch()) + ".jpg";
    QString permanentThumbPath = m_appDataFolder + "/" + uniqueName;

    // Copy the temporary image to the permanent cache folder
    QFile::copy(m_tempThumbPath, permanentThumbPath);

    //Add the new items to the front of the list
    m_historyTitles.prepend(title);
    m_historyPaths.prepend(savedPath);
    m_historyThumbs.prepend(permanentThumbPath);

    //enforce 5 video limit
    if (m_historyTitles.size() > 5) {
        // Delete the oldest physical image file from the hard drive to save space
        QFile::remove(m_historyThumbs.last());

        // Remove the oldest entries from the back of the lists
        m_historyTitles.removeLast();
        m_historyPaths.removeLast();
        m_historyThumbs.removeLast();
    }

    // save settings
    QSettings settings("MyCoolApps", "YtDlpGUI");
    settings.setValue("historyTitles", m_historyTitles);
    settings.setValue("historyPaths", m_historyPaths);
    settings.setValue("historyThumbs", m_historyThumbs);

    updateHistoryUI();
}

void MainWindow::loadHistory()
{
    QSettings settings("MyCoolApps", "YtDlpGUI");

    // Grab the lists from the OS. If they don't exist yet, it just returns an empty list
    m_historyTitles = settings.value("historyTitles").toStringList();
    m_historyPaths = settings.value("historyPaths").toStringList();
    m_historyThumbs = settings.value("historyThumbs").toStringList();
}

void MainWindow::updateHistoryUI()
{
    //Get the vertical layout you created inside the scroll area
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaWidgetContents->layout());

    // Safety fallback: if you forgot to add the layout in Qt Designer, this creates it automatically!
    if (!mainLayout) {
        mainLayout = new QVBoxLayout(ui->scrollAreaWidgetContents);
        ui->scrollAreaWidgetContents->setLayout(mainLayout);
    }

    //Clear out the old UI items so we don't duplicate the list every time we download a new video!
    QLayoutItem* child;
    while ((child = mainLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget(); // Delete the actual row
        }
        delete child; // Delete the layout pointer
    }

    for (int i = 0; i < m_historyTitles.size(); ++i) {

        QWidget* rowWidget = new QWidget(ui->scrollAreaWidgetContents);

        rowWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        rowWidget->setMinimumHeight(110);

        QHBoxLayout* rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(10, 10, 10, 10); // Add a little padding around the edges

        // draw thumbnail
        QLabel* thumbLabel = new QLabel(rowWidget);
        thumbLabel->setFixedSize(160, 90);
        QPixmap pix(m_historyThumbs[i]);
        if (!pix.isNull()) {
            thumbLabel->setPixmap(pix.scaled(160, 90, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            thumbLabel->setText("Image Missing");
            thumbLabel->setAlignment(Qt::AlignCenter);
        }

        // draw text
        QVBoxLayout* textLayout = new QVBoxLayout();

        QLabel* titleLabel = new QLabel("<b>" + m_historyTitles[i] + "</b>", rowWidget);
        QLabel* pathLabel = new QLabel("<i>Saved to: " + m_historyPaths[i] + "</i>", rowWidget);

        titleLabel->setWordWrap(true);


        titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        pathLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);


        textLayout->addWidget(titleLabel);
        textLayout->addWidget(pathLabel);
        textLayout->addStretch();


        rowLayout->addWidget(thumbLabel, 0, Qt::AlignTop);
        rowLayout->addLayout(textLayout);



        mainLayout->addWidget(rowWidget);
    }

    mainLayout->addStretch();
}

void MainWindow::on_btnClearHistoryClicked()
{
    //Ask user if they are sure
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Clear History", "Are you sure you want to delete your download history?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) {
        return; // Cancel the wipe
    }

    //Delete the physical thumbnail images from the hard drive
    for (int i = 0; i < m_historyThumbs.size(); ++i) {
        QFile::remove(m_historyThumbs[i]);
    }

    // Empty the C++ lists
    m_historyTitles.clear();
    m_historyPaths.clear();
    m_historyThumbs.clear();

    //Wipe the saved memory in the OS
    QSettings settings("MyCoolApps", "YtDlpGUI");
    settings.remove("historyTitles");
    settings.remove("historyPaths");
    settings.remove("historyThumbs");

    //Redraw the UI (draw empty list)
    updateHistoryUI();
}
