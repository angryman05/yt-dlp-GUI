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
    // ------------------------------------

    // --- LOAD SAVED SETTINGS ON STARTUP ---
    QSettings settings("MyCoolApps", "YtDlpGUI");
    m_saveFolder = settings.value("lastFolder", "").toString();

    if (!m_saveFolder.isEmpty()) {
        ui->lblPath->setText(m_saveFolder);
    } else {
        ui->lblPath->setText("No folder selected");
    }

    // --- LOAD HISTORY ON STARTUP ---
    loadHistory();
    updateHistoryUI();

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

    // --- NEW PLAYLIST LOGIC ---
    QString outputPath;

    if (ui->chkPlaylist->isChecked()) {
        // Yes Playlist: Create a folder with the playlist name, and number the files!
        arguments << "--yes-playlist";
        outputPath = m_saveFolder + "/%(playlist_title)s/%(playlist_index)s - %(title)s.%(ext)s";
    } else {
        // No Playlist: Just download the single video, even if it's part of a playlist link
        arguments << "--no-playlist";
        outputPath = m_saveFolder + "/%(title)s.%(ext)s";
    }

    arguments << "-o" << outputPath;

    // Format selection logic
    if (format == "mp3" || format == "wav" || format == "m4a" || format == "flac" || format == "ogg") {
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

        // --- ADD THESE TWO LINES ---
        // Grab the title we fetched earlier from the label
        QString videoTitle = ui->lblTitle->text();

        if (ui->chkPlaylist->isChecked()) {
            videoTitle = "🗂️ [Playlist] " + videoTitle;
        }

        // Send it to our new save function!
        saveToHistory(videoTitle, m_saveFolder);
        // ---------------------------

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

void MainWindow::saveToHistory(QString title, QString savedPath)
{
    // 1. Create a permanent copy of the thumbnail
    // We name the image using the current exact millisecond so it's always unique!
    QString uniqueName = QString::number(QDateTime::currentMSecsSinceEpoch()) + ".jpg";
    QString permanentThumbPath = m_appDataFolder + "/" + uniqueName;

    // Copy the temporary image to the permanent cache folder
    QFile::copy(m_tempThumbPath, permanentThumbPath);

    // 2. Add the new items to the FRONT of our lists (Index 0)
    m_historyTitles.prepend(title);
    m_historyPaths.prepend(savedPath);
    m_historyThumbs.prepend(permanentThumbPath);

    // 3. Enforce the 5-item limit (FIFO Queue)
    if (m_historyTitles.size() > 5) {
        // Delete the oldest physical image file from your hard drive to save space
        QFile::remove(m_historyThumbs.last());

        // Remove the oldest entries from the back of the lists
        m_historyTitles.removeLast();
        m_historyPaths.removeLast();
        m_historyThumbs.removeLast();
    }

    // 4. Save the lists to the operating system using QSettings!
    QSettings settings("MyCoolApps", "YtDlpGUI");
    settings.setValue("historyTitles", m_historyTitles);
    settings.setValue("historyPaths", m_historyPaths);
    settings.setValue("historyThumbs", m_historyThumbs);

    // 5. Tell the UI to redraw the History tab
    updateHistoryUI();
}

void MainWindow::loadHistory()
{
    QSettings settings("MyCoolApps", "YtDlpGUI");

    // Grab the lists from the OS. If they don't exist yet, it just returns an empty list!
    m_historyTitles = settings.value("historyTitles").toStringList();
    m_historyPaths = settings.value("historyPaths").toStringList();
    m_historyThumbs = settings.value("historyThumbs").toStringList();
}

void MainWindow::updateHistoryUI()
{
    // 1. Get the vertical layout you created inside the scroll area
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(ui->scrollAreaWidgetContents->layout());

    // Safety fallback: if you forgot to add the layout in Qt Designer, this creates it automatically!
    if (!mainLayout) {
        mainLayout = new QVBoxLayout(ui->scrollAreaWidgetContents);
        ui->scrollAreaWidgetContents->setLayout(mainLayout);
    }

    // 2. Clear out the old UI items so we don't duplicate the list every time we download a new video!
    QLayoutItem* child;
    while ((child = mainLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget(); // Delete the actual row
        }
        delete child; // Delete the layout pointer
    }

    for (int i = 0; i < m_historyTitles.size(); ++i) {

        QWidget* rowWidget = new QWidget(ui->scrollAreaWidgetContents);

        // --- NEW FIX 1: Lock the row height so it doesn't stretch! ---
        rowWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        rowWidget->setMinimumHeight(110);
        // -------------------------------------------------------------

        QHBoxLayout* rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(10, 10, 10, 10); // Add a little padding around the edges

        // --- DRAW THUMBNAIL ---
        QLabel* thumbLabel = new QLabel(rowWidget);
        thumbLabel->setFixedSize(160, 90);
        QPixmap pix(m_historyThumbs[i]);
        if (!pix.isNull()) {
            thumbLabel->setPixmap(pix.scaled(160, 90, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            thumbLabel->setText("Image Missing");
            thumbLabel->setAlignment(Qt::AlignCenter);
        }

        // --- DRAW TEXT ---
        QVBoxLayout* textLayout = new QVBoxLayout();

        QLabel* titleLabel = new QLabel("<b>" + m_historyTitles[i] + "</b>", rowWidget);
        QLabel* pathLabel = new QLabel("<i>Saved to: " + m_historyPaths[i] + "</i>", rowWidget);

        titleLabel->setWordWrap(true);

        // --- NEW FIX 2: Force the text to align perfectly to the left! ---
        titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        pathLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        // -----------------------------------------------------------------

        textLayout->addWidget(titleLabel);
        textLayout->addWidget(pathLabel);
        textLayout->addStretch();

        // --- NEW FIX 3: Snap it all together and anchor the thumbnail to the top! ---
        rowLayout->addWidget(thumbLabel, 0, Qt::AlignTop);
        rowLayout->addLayout(textLayout);
        // ----------------------------------------------------------------------------

        // Add the finished row to the master layout
        mainLayout->addWidget(rowWidget);
    }
    // Add a spacer at the very bottom so all the rows are pushed nicely to the top of the window
    mainLayout->addStretch();
}

void MainWindow::on_btnClearHistoryClicked()
{
    // 1. Ask the user if they are sure!
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Clear History", "Are you sure you want to delete your download history?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) {
        return; // Cancel the wipe
    }

    // 2. Delete the physical thumbnail images from your hard drive
    for (int i = 0; i < m_historyThumbs.size(); ++i) {
        QFile::remove(m_historyThumbs[i]);
    }

    // 3. Empty the C++ lists
    m_historyTitles.clear();
    m_historyPaths.clear();
    m_historyThumbs.clear();

    // 4. Wipe the saved memory in the OS
    QSettings settings("MyCoolApps", "YtDlpGUI");
    settings.remove("historyTitles");
    settings.remove("historyPaths");
    settings.remove("historyThumbs");

    // 5. Redraw the UI (it will draw an empty list!)
    updateHistoryUI();
}
