# Angry Downloader 🎥

A fast, cross-platform graphical user interface (GUI) for `yt-dlp` and `FFmpeg`, built with C++ and Qt 6. Easily download high-quality videos and audio without touching the command line.

[🇷🇴 Citește în Limba Română](#-limba-română-romanian)

## ✨ Features
* **Cross-Platform:** Works perfectly on Windows and Linux.
* **Powered by yt-dlp:** Supports downloading from YouTube and hundreds of other supported sites.
* **Automated Merging:** Uses FFmpeg under the hood to automatically stitch together high-quality video and audio streams.
* **Portable:** No installation required. Just download the AppImage (Linux) or extract the ZIP (Windows) and run.

## 🚀 Download & Install
Head over to the **[Releases](../../releases/latest)** page to download the latest version for your operating system:
* **Windows:** Download the `.zip` file, extract it, and double-click `angry_downloader.exe`.
* **Linux (Fedora/Ubuntu/etc):** Download the `.AppImage` file, make it executable (`chmod +x`), and double-click to run.

## 🛠️ Building from Source

### Prerequisites
* Qt 6 (Tested with 6.10.2)
* CMake
* C++ Compiler (MinGW for Windows, GCC for Linux)

### Building on Windows
To keep this repository lightweight, the large Windows binaries for yt-dlp and FFmpeg are not tracked by Git. To compile this project on Windows, you must add them manually:

1. Clone the repository.
2. Download the latest [yt-dlp.exe](https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.exe) and place it inside `bundled_tools/windows/`.
3. Download the [FFmpeg Essentials build](https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-essentials.zip). Open the zip, go to the `bin/` folder, and extract `ffmpeg.exe` into `bundled_tools/windows/`.
4. Open the project in Qt Creator, select your MinGW kit, and hit **Build**.

### Building on Linux
1. Clone the repository.
2. Ensure you have the required Qt 6 development packages installed.
3. Open the project in Qt Creator, select your GCC kit, and hit **Build**.
*(Note: The Linux AppImage release is bundled with the necessary tools automatically during the deployment process).*

---

# 🇷🇴 Limba Română (Romanian)

# Angry Downloader 🎥

O interfață grafică (GUI) rapidă și cross-platform pentru `yt-dlp` și `FFmpeg`, construită cu C++ și Qt 6. Descarcă ușor videoclipuri și fișiere audio la calitate maximă, fără a folosi linia de comandă.

## ✨ Funcționalități
* **Cross-Platform:** Funcționează perfect pe Windows și Linux.
* **Puterea yt-dlp:** Suportă descărcarea de pe YouTube și sute de alte site-uri.
* **Îmbinare Automată:** Folosește FFmpeg în fundal pentru a îmbina automat fluxurile video și audio de înaltă calitate.
* **Portabil:** Nu necesită instalare. Doar descarcă AppImage-ul (Linux) sau extrage arhiva ZIP (Windows) și pornește aplicația.

## 🚀 Descărcare și Instalare
Accesează pagina de **[Releases](../../releases/latest)** pentru a descărca ultima versiune pentru sistemul tău de operare:
* **Windows:** Descarcă fișierul `.zip`, extrage-l și rulează `angry_downloader.exe`.
* **Linux (Fedora/Ubuntu/etc):** Descarcă fișierul `.AppImage`, oferă-i permisiuni de execuție (`chmod +x`) și rulează-l.

## 🛠️ Compilare din Codul Sursă (Build)

### Cerințe
* Qt 6 (Testat cu versiunea 6.10.2)
* CMake
* Compilator C++ (MinGW pentru Windows, GCC pentru Linux)

### Compilare pe Windows
Pentru a păstra acest repository ușor de descărcat, fișierele binare mari pentru Windows (yt-dlp și FFmpeg) nu sunt stocate în Git. Pentru a compila acest proiect pe Windows, trebuie să le adaugi manual:

1. Clonează repository-ul.
2. Descarcă ultimul [yt-dlp.exe](https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.exe) și pune-l în folderul `bundled_tools/windows/`.
3. Descarcă [FFmpeg Essentials build](https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-essentials.zip). Deschide arhiva zip, intră în folderul `bin/` și extrage `ffmpeg.exe` în `bundled_tools/windows/`.
4. Deschide proiectul în Qt Creator, selectează kit-ul MinGW și apasă **Build**.

### Compilare pe Linux
1. Clonează repository-ul.
2. Asigură-te că ai instalate pachetele de dezvoltare necesare pentru Qt 6.
3. Deschide proiectul în Qt Creator, selectează kit-ul GCC și apasă **Build**.
*(Notă: Versiunea AppImage pentru Linux include deja automat uneltele necesare în timpul procesului de deployment).*
