# Angry Downloader 🎥

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0) [![Framework: Qt 6](https://img.shields.io/badge/Framework-Qt_6-41CD52.svg)](https://www.qt.io/) [![Language: C++](https://img.shields.io/badge/Language-C++-00599C.svg)](https://isocpp.org/)

**[ 🇬🇧 English ](#angry-downloader-en)** | **[ 🇷🇴 Română ](#angry-downloader-ro)**

---

<a name="angry-downloader-en"></a>
## 🇬🇧 Angry Downloader (EN)

A fast, cross-platform graphical user interface (GUI) for `yt-dlp` and `FFmpeg`, built with C++ and Qt 6. Easily download high-quality videos and audio without touching the command line.

### 🎯 Key Features
* **Cross-Platform:** Works natively on Windows and Linux (Fedora, Ubuntu, etc.).
* **Powered by yt-dlp:** Supports downloading from YouTube and hundreds of other websites.
* **Automated Merging:** Uses FFmpeg under the hood to automatically stitch together high-quality video and audio streams.
* **Fully Portable:** No installation required. Just download the AppImage (Linux) or extract the ZIP (Windows) and run.

### 🛠️ Software Requirements & Dependencies
To run the pre-built releases, you don't need anything. To compile from source, you will need:
* **Framework:** Qt 6 (Tested with 6.10.2)
* **Build System:** CMake
* **Compiler:** MinGW (Windows) or GCC (Linux)
* **Backend Tools:** `yt-dlp` and `FFmpeg`

### 🔌 Project Structure & Tools (Paths)
To keep the repository lightweight, the massive binary tools for Windows are ignored by Git (using a `.gitkeep` file). If you are building from source, place them here:

| Tool | Windows Path | Linux Path |
| :--- | :--- | :--- |
| **yt-dlp** | `bundled_tools/windows/yt-dlp.exe` | Automatically bundled inside AppDir |
| **FFmpeg** | `bundled_tools/windows/ffmpeg.exe` | Automatically bundled inside AppDir |

*Note for Windows Devs: Download the [yt-dlp release](https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.exe) and the [FFmpeg essentials build](https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-essentials.zip) and drop the executables into the paths above before building in Qt Creator.*

### ⚠️ Known Issues & Build Notes
* **Linux AppImage Dependency Clashes:** Compiling the AppImage on bleeding-edge distros (like Fedora) with `linuxdeploy` can cause Segfaults due to bundled kernel-level libraries. 
    * *Solution:* We manually purge `libcap`, `libsystemd`, `libpcre2`, etc., from the `AppDir`, manually drop in the Qt `wayland`/`xcb` platform plugins, and compress it using raw `appimagetool`.
* **Missing Windows Folders:** Git does not track empty folders. 
    * *Solution:* A `.gitkeep` file is used inside the `bundled_tools/windows` directory to force the folder structure to sync for collaborators.

### 👨‍💻 Author
**angryman05**
---

<a name="angry-downloader-ro"></a>
## 🇷🇴 Angry Downloader (RO)

O interfață grafică (GUI) rapidă și cross-platform pentru `yt-dlp` și `FFmpeg`, construită cu C++ și Qt 6. Descarcă ușor videoclipuri și fișiere audio la calitate maximă, fără a folosi linia de comandă.

### 🎯 Funcționalități Cheie
* **Cross-Platform:** Funcționează nativ pe Windows și Linux (Fedora, Ubuntu, etc.).
* **Puterea yt-dlp:** Suportă descărcarea de pe YouTube și sute de alte site-uri.
* **Îmbinare Automată:** Folosește FFmpeg în fundal pentru a îmbina automat fluxurile video și audio de înaltă calitate.
* **Complet Portabil:** Nu necesită instalare. Doar descarcă AppImage-ul (Linux) sau extrage arhiva ZIP (Windows) și pornește aplicația.

### 🛠️ Cerințe Software & Dependințe
Pentru a rula versiunile pre-compilate, nu ai nevoie de nimic. Pentru a compila din codul sursă, necesită:
* **Framework:** Qt 6 (Testat cu 6.10.2)
* **Sistem de Build:** CMake
* **Compilator:** MinGW (Windows) sau GCC (Linux)
* **Unelte Backend:** `yt-dlp` și `FFmpeg`

### 🔌 Structura Proiectului & Unelte (Paths)
Pentru a păstra acest repository ușor, binarele masive pentru Windows sunt ignorate de Git (folosind un fișier `.gitkeep`). Dacă compilezi din sursă, plasează-le aici:

| Unealtă | Cale Windows (Path) | Cale Linux (Path) |
| :--- | :--- | :--- |
| **yt-dlp** | `bundled_tools/windows/yt-dlp.exe` | Împachetat automat în AppDir |
| **FFmpeg** | `bundled_tools/windows/ffmpeg.exe` | Împachetat automat în AppDir |

*Notă pentru dezvoltatorii Windows: Descarcă [yt-dlp](https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.exe) și arhiva [FFmpeg essentials](https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-essentials.zip) și copiază executabilele în folderele de mai sus înainte de a da Build în Qt Creator.*

### ⚠️ Probleme Cunoscute & Notițe de Compilare
* **Conflicte de Dependențe la Linux AppImage:** Compilarea AppImage-ului pe distribuții foarte noi (precum Fedora) cu `linuxdeploy` poate cauza Segfault-uri din cauza bibliotecilor de sistem împachetate greșit.
    * *Soluție:* Ștergem manual `libcap`, `libsystemd`, `libpcre2` din folderul `AppDir`, adăugăm manual plugin-urile Qt pentru `wayland`/`xcb` și comprimăm totul folosind `appimagetool` brut.
* **Foldere Lipsă pe Windows:** Git nu sincronizează foldere goale.
    * *Soluție:* Un fișier `.gitkeep` este folosit în folderul `bundled_tools/windows` pentru a forța păstrarea structurii de directoare pentru colaboratori.

### 👨‍💻 Autor
**angryman05**
