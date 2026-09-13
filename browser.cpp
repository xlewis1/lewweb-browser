#include <QApplication>
#include <QCompleter>
#include <QCommandLineParser>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMainWindow>
#include <QHBoxLayout>
#include <QLabel>
#include <QTabBar>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>
#include <QShortcut>
#include <QStandardPaths>
#include <QTabWidget>
#include <QUrl>
#include <QResizeEvent>
#include <QProcess>
#include <QColor>
#include <QPalette>

#include <QWebEngineDownloadRequest>
#include <QWebEngineFullScreenRequest>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineView>

#include <functional>
#include <utility>


constexpr const char *LEWWEB_VERSION = "1.0.0";
constexpr const char *SERVER_NAME = "lewweb-browser";


static void printHelp();
static void printVersion();


struct BrowserConfig
{
    QString windowBorder = "default";
    QString theme = "light";

    QString browserColour = "#9b59b6";
    QString borderColour = "#9b59b6";

    QString backgroundColour;
    QString textColour;

    QString homepage = "https://duckduckgo.com/";
    QString searchEngine = "duckduckgo";

    int zoom = 100;
};


static QString profilePath()
{
    QString base =
        QStandardPaths::writableLocation(
            QStandardPaths::AppDataLocation
        );

    QDir dir(base);

    if (!dir.exists())
        dir.mkpath(".");

    QString path =
        dir.filePath("profile");

    QDir profile(path);

    if (!profile.exists())
        profile.mkpath(".");

    return path;
}


static QString bookmarksPath()
{
    return QDir(profilePath()).filePath(
        "bookmarks.json"
    );
}


static QJsonObject loadBookmarks()
{
    QFile file(bookmarksPath());

    if (!file.open(QIODevice::ReadOnly))
        return {};

    QJsonParseError error;

    QJsonDocument document =
        QJsonDocument::fromJson(
            file.readAll(),
            &error
        );

    if (error.error != QJsonParseError::NoError)
        return {};

    return document.object();
}


static bool saveBookmarks(
    const QJsonObject &bookmarks
)
{
    QFile file(bookmarksPath());

    if (!file.open(
        QIODevice::WriteOnly |
        QIODevice::Truncate
    ))
    {
        return false;
    }

    file.write(
        QJsonDocument(bookmarks)
            .toJson(QJsonDocument::Indented)
    );

    return true;
}


static bool addBookmark(
    const QString &name,
    const QString &url
)
{
    if (
        name.trimmed().isEmpty() ||
        url.trimmed().isEmpty()
    )
    {
        return false;
    }

    QJsonObject bookmarks =
        loadBookmarks();

    bookmarks[name] = url;

    return saveBookmarks(bookmarks);
}


static bool removeBookmark(
    const QString &name
)
{
    QJsonObject bookmarks =
        loadBookmarks();

    if (!bookmarks.contains(name))
        return false;

    bookmarks.remove(name);

    return saveBookmarks(bookmarks);
}


static QString bookmarkUrl(
    const QString &name
)
{
    return loadBookmarks()
        .value(name)
        .toString();
}


static QString referencePath()
{
    const QString fileName =
        "lewweb-reference.html";

    QString path =
        QDir::current().filePath(fileName);

    if (QFile::exists(path))
        return path;

    path =
        QDir(
            QCoreApplication::applicationDirPath()
        ).filePath(fileName);

    if (QFile::exists(path))
        return path;

    path =
        "/usr/local/share/lewweb/" +
        fileName;

    if (QFile::exists(path))
        return path;

    return {};
}


static bool runPinterestDownloader(
    const QString &url,
    const QString &directory
)
{
    if (
        url.trimmed().isEmpty() ||
        directory.trimmed().isEmpty()
    )
    {
        return false;
    }

    QDir dir(directory);

    if (!dir.exists())
    {
        if (!dir.mkpath("."))
            return false;
    }

    QProcess *process =
        new QProcess(qApp);

    process->setProgram("lew-dlp");

    process->setArguments({
        url,
        "-P",
        directory
    });

    process->setProcessChannelMode(
        QProcess::ForwardedChannels
    );

    QObject::connect(
        process,
        &QProcess::finished,
        process,
        &QObject::deleteLater
    );

    process->start();

    return true;
}


static BrowserConfig loadConfig(
    const QString &path
)
{
    BrowserConfig config;

    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning().noquote()
            << "LewWeb: could not open config:"
            << path;

        return config;
    }

    QJsonParseError error;

    QJsonDocument document =
        QJsonDocument::fromJson(
            file.readAll(),
            &error
        );

    if (error.error != QJsonParseError::NoError)
    {
        qWarning().noquote()
            << "LewWeb: invalid config:"
            << error.errorString();

        return config;
    }

    if (!document.isObject())
    {
        qWarning().noquote()
            << "LewWeb: config must contain a JSON object.";

        return config;
    }

    QJsonObject object =
        document.object();

    if (object.contains("window_border"))
        config.windowBorder =
            object["window_border"].toString(
                config.windowBorder
            ).toLower();

    if (object.contains("theme"))
        config.theme =
            object["theme"].toString(
                config.theme
            ).toLower();

    if (object.contains("browser_colour"))
        config.browserColour =
            object["browser_colour"].toString(
                config.browserColour
            );

    if (object.contains("border_colour"))
        config.borderColour =
            object["border_colour"].toString(
                config.borderColour
            );

    if (object.contains("background_colour"))
        config.backgroundColour =
            object["background_colour"].toString();

    if (object.contains("text_colour"))
        config.textColour =
            object["text_colour"].toString();

    if (object.contains("homepage"))
        config.homepage =
            object["homepage"].toString(
                config.homepage
            );

    if (object.contains("search_engine"))
        config.searchEngine =
            object["search_engine"].toString(
                config.searchEngine
            ).toLower();

    if (object.contains("zoom"))
        config.zoom =
            object["zoom"].toInt(
                config.zoom
            );

    if (
        config.windowBorder != "default" &&
        config.windowBorder != "borderless"
    )
    {
        qWarning().noquote()
            << "LewWeb: invalid window_border:"
            << config.windowBorder
            << "(using default)";

        config.windowBorder = "default";
    }

    if (
        config.theme != "light" &&
        config.theme != "dark"
    )
    {
        qWarning().noquote()
            << "LewWeb: invalid theme:"
            << config.theme
            << "(using light)";

        config.theme = "light";
    }

    if (!QColor(config.browserColour).isValid())
    {
        qWarning().noquote()
            << "LewWeb: invalid browser_colour:"
            << config.browserColour;

        config.browserColour = "#9b59b6";
    }

    if (!QColor(config.borderColour).isValid())
    {
        qWarning().noquote()
            << "LewWeb: invalid border_colour:"
            << config.borderColour;

        config.borderColour = "#9b59b6";
    }

    if (
        !config.backgroundColour.isEmpty() &&
        !QColor(config.backgroundColour).isValid()
    )
    {
        qWarning().noquote()
            << "LewWeb: invalid background_colour:"
            << config.backgroundColour;

        config.backgroundColour.clear();
    }

    if (
        !config.textColour.isEmpty() &&
        !QColor(config.textColour).isValid()
    )
    {
        qWarning().noquote()
            << "LewWeb: invalid text_colour:"
            << config.textColour;

        config.textColour.clear();
    }

    if (config.zoom < 25)
        config.zoom = 25;

    if (config.zoom > 500)
        config.zoom = 500;

    return config;
}


class BrowserWindow;


class BrowserPage : public QWebEnginePage
{
public:
    explicit BrowserPage(
        QWebEngineProfile *profile,
        std::function<QWebEnginePage *()> createPage,
        QObject *parent = nullptr
    )
        : QWebEnginePage(profile, parent),
          createPage_(std::move(createPage))
    {
    }


protected:
    QWebEnginePage *createWindow(
        WebWindowType type
    ) override
    {
        Q_UNUSED(type);

        if (createPage_)
            return createPage_();

        return nullptr;
    }


private:
    std::function<QWebEnginePage *()> createPage_;
};


class BrowserTab : public QWebEngineView
{
public:
    explicit BrowserTab(
        QWebEngineProfile *profile,
        std::function<QWebEnginePage *()> createPage,
        int zoomPercentage,
        QWidget *parent = nullptr
    )
        : QWebEngineView(parent)
    {
        setPage(
            new BrowserPage(
                profile,
                std::move(createPage),
                this
            )
        );

        QWebEngineSettings *settings =
            this->settings();

        settings->setAttribute(
            QWebEngineSettings::JavascriptEnabled,
            true
        );

        settings->setAttribute(
            QWebEngineSettings::LocalStorageEnabled,
            true
        );

        settings->setAttribute(
            QWebEngineSettings::FullScreenSupportEnabled,
            true
        );

        settings->setAttribute(
            QWebEngineSettings::JavascriptCanOpenWindows,
            true
        );

        settings->setAttribute(
            QWebEngineSettings::JavascriptCanAccessClipboard,
            true
        );

        settings->setAttribute(
            QWebEngineSettings::AllowRunningInsecureContent,
            false
        );

        settings->setAttribute(
            QWebEngineSettings::PlaybackRequiresUserGesture,
            false
        );

        setZoomFactor(
            zoomPercentage / 100.0
        );
    }
};


class BrowserWindow : public QMainWindow
{
public:
    explicit BrowserWindow(
        QWebEngineProfile *profile,
        const BrowserConfig &config
    )
        : QMainWindow(nullptr),
          profile_(profile),
          config_(config)
    {
        setWindowTitle("LewWeb");
        resize(1280, 800);

        applyConfig();

        tabs_ =
            new QTabWidget(this);

        tabs_->setTabBarAutoHide(false);
        tabs_->setTabsClosable(false);
        tabs_->setDocumentMode(true);
        tabs_->tabBar()->hide();

        setCentralWidget(tabs_);

        progressBar_ =
            new QProgressBar(this);

        progressBar_->setRange(0, 100);
        progressBar_->setValue(0);
        progressBar_->setTextVisible(false);
        progressBar_->setFixedHeight(3);
        progressBar_->hide();

        commandPalette_ =
            new QWidget(this);

        commandPalette_->setObjectName(
            "commandPalette"
        );

        QHBoxLayout *commandLayout =
            new QHBoxLayout(
                commandPalette_
            );

        commandLayout->setContentsMargins(
            10,
            5,
            10,
            5
        );

        commandLayout->setSpacing(8);

        commandPrompt_ =
            new QLabel(
                "lewweb:>$",
                commandPalette_
            );

        commandBar_ =
            new QLineEdit(
                commandPalette_
            );

        commandBar_->setFrame(false);

        commandLayout->addWidget(
            commandPrompt_
        );

        commandLayout->addWidget(
            commandBar_
        );

        QStringList commands = {
            "--open",
            "--search",
            "--home",
            "--back",
            "--forward",
            "--reload",
            "--stop",

            "--wikipedia",
            "--github",
            "--youtube",
            "--reddit",
            "--google",
            "--images",
            "--news",

            "--new-tab",
            "--close-tab",
            "--tab",
            "--next-tab",
            "--previous-tab",
            "--list-tabs",

            "--download-file",
            "--download-pinterest-image",

            "--new-bookmark",
            "--save-bookmark",
            "--open-bookmark",
            "--delete-bookmark",

            "--fullscreen",
            "--exit-fullscreen",
            "--javascript",
            "--light-mode",
            "--dark-mode",
            "--zoom-in",
            "--zoom-out",
            "--zoom",

            "--help",
            "--version",
            "--config",
            "--quiet"
        };

        QCompleter *completer =
            new QCompleter(
                commands,
                commandBar_
            );

        completer->setCaseSensitivity(
            Qt::CaseInsensitive
        );

        completer->setCompletionMode(
            QCompleter::PopupCompletion
        );

        commandBar_->setCompleter(
            completer
        );

        commandPalette_->hide();

        connect(
            commandBar_,
            &QLineEdit::returnPressed,
            this,
            [this]()
            {
                QString command =
                    commandBar_->text();

                commandBar_->clear();
                commandPalette_->hide();

                if (!command.trimmed().isEmpty())
                    executeCommand(command);

                currentTabFocus();
            }
        );

        applyWidgetStyle();

        addTab(
            QUrl(config_.homepage)
        );

        setupShortcuts();
        setupDownloads();
    }


    void applyConfig(
        const BrowserConfig &config
    )
    {
        config_ = config;

        applyConfig();
    }


    void resizeEvent(
        QResizeEvent *event
    ) override
    {
        QMainWindow::resizeEvent(event);

        if (commandPalette_)
        {
            commandPalette_->setGeometry(
                8,
                height() - 50,
                width() - 16,
                42
            );
        }

        if (progressBar_)
        {
            progressBar_->setGeometry(
                0,
                0,
                width(),
                3
            );
        }
    }


    void setupDownloads()
    {
        profile_->setDownloadPath(
            QStandardPaths::writableLocation(
                QStandardPaths::DownloadLocation
            )
        );

        connect(
            profile_,
            &QWebEngineProfile::downloadRequested,
            this,
            [this](
                QWebEngineDownloadRequest *download
            )
            {
                if (!download)
                    return;

                QString directory =
                    pendingDownloadDirectory_;

                pendingDownloadDirectory_.clear();

                if (directory.isEmpty())
                {
                    directory =
                        profile_->downloadPath();
                }

                QDir dir(directory);

                if (!dir.exists())
                {
                    if (!dir.mkpath("."))
                    {
                        qWarning()
                            << "LewWeb: could not create download directory:"
                            << directory;

                        download->cancel();
                        return;
                    }
                }

                QString fileName =
                    download->downloadFileName();

                if (fileName.isEmpty())
                    fileName =
                        download->suggestedFileName();

                if (fileName.isEmpty())
                    fileName = "download";

                download->setDownloadDirectory(
                    directory
                );

                download->setDownloadFileName(
                    fileName
                );

                connect(
                    download,
                    &QWebEngineDownloadRequest::stateChanged,
                    this,
                    [download](
                        QWebEngineDownloadRequest::DownloadState state
                    )
                    {
                        if (
                            state ==
                            QWebEngineDownloadRequest::DownloadCompleted
                        )
                        {
                            qInfo().noquote()
                                << "LewWeb: downloaded"
                                << download->downloadFileName();
                        }
                        else if (
                            state ==
                            QWebEngineDownloadRequest::DownloadInterrupted
                        )
                        {
                            qWarning().noquote()
                                << "LewWeb: download interrupted:"
                                << download->interruptReasonString();
                        }
                    }
                );

                download->accept();

                qInfo().noquote()
                    << "LewWeb: downloading"
                    << download->url().toString()
                    << "->"
                    << QDir(directory)
                        .filePath(fileName);
            }
        );
    }


    void setupShortcuts()
    {
        shortcut(
            "Ctrl+T",
            [this]()
            {
                addTab(
                    QUrl(config_.homepage)
                );
            }
        );

        shortcut(
            "Meta+T",
            [this]()
            {
                openCommandBar();
            }
        );

        shortcut(
            "Ctrl+W",
            [this]()
            {
                closeCurrentTab();
            }
        );

        shortcut(
            "Meta+W",
            [this]()
            {
                closeCurrentTab();
            }
        );

        shortcut(
            "Ctrl+R",
            [this]()
            {
                reload();
            }
        );

        shortcut(
            "Meta+R",
            [this]()
            {
                reload();
            }
        );

        shortcut(
            "Alt+Left",
            [this]()
            {
                back();
            }
        );

        shortcut(
            "Meta+Left",
            [this]()
            {
                back();
            }
        );

        shortcut(
            "Alt+Right",
            [this]()
            {
                forward();
            }
        );

        shortcut(
            "Meta+Right",
            [this]()
            {
                forward();
            }
        );

        shortcut(
            "Ctrl+Tab",
            [this]()
            {
                nextTab();
            }
        );

        shortcut(
            "Ctrl+Shift+Tab",
            [this]()
            {
                previousTab();
            }
        );

        for (int i = 1; i <= 9; ++i)
        {
            QShortcut *sc =
                new QShortcut(
                    QKeySequence(
                        QString("Meta+%1").arg(i)
                    ),
                    this
                );

            connect(
                sc,
                &QShortcut::activated,
                this,
                [this, i]()
                {
                    selectTab(i - 1);
                }
            );
        }

        shortcut(
            "F11",
            [this]()
            {
                fullscreen();
            }
        );

        shortcut(
            "+",
            [this]()
            {
                zoomIn();
            }
        );

        shortcut(
            "=",
            [this]()
            {
                zoomIn();
            }
        );

        shortcut(
            "-",
            [this]()
            {
                zoomOut();
            }
        );

        shortcut(
            "Meta+/",
            [this]()
            {
                openReference();
            }
        );
    }


    template <typename Function>
    void shortcut(
        const QString &sequence,
        Function function
    )
    {
        QShortcut *sc =
            new QShortcut(
                QKeySequence(sequence),
                this
            );

        sc->setContext(
            Qt::WindowShortcut
        );

        connect(
            sc,
            &QShortcut::activated,
            this,
            function
        );
    }


    void openCommandBar()
    {
        commandPalette_->show();
        commandPalette_->raise();

        commandBar_->clear();
        commandBar_->setFocus();

        if (commandBar_->completer())
            commandBar_->completer()->complete();
    }


    void currentTabFocus()
    {
        if (auto *view = currentTab())
            view->setFocus();
    }


    void openReference()
    {
        QString path =
            referencePath();

        if (path.isEmpty())
        {
            QMessageBox::warning(
                this,
                "LewWeb",
                "lewweb-reference.html "
                "could not be found."
            );

            return;
        }

        addTab(
            QUrl::fromLocalFile(path)
        );
    }


    BrowserTab *addTab(
        const QUrl &url
    )
    {
        BrowserTab *view =
            new BrowserTab(
                profile_,
                [this]() -> QWebEnginePage *
                {
                    BrowserTab *tab =
                        addTab(
                            QUrl("about:blank")
                        );

                    return tab->page();
                },
                config_.zoom
            );

        int index =
            tabs_->addTab(
                view,
                QString()
            );

        tabs_->setCurrentIndex(index);

        connect(
            view->page(),
            &QWebEnginePage::fullScreenRequested,
            this,
            [view](
                QWebEngineFullScreenRequest request
            )
            {
                if (request.toggleOn())
                    view->showFullScreen();
                else
                    view->showNormal();

                request.accept();
            }
        );

        connect(
            view,
            &QWebEngineView::titleChanged,
            this,
            [this](const QString &)
            {
                updateWindowTitle();
            }
        );

        connect(
            view,
            &QWebEngineView::loadProgress,
            this,
            [this](int progress)
            {
                if (progress >= 100)
                {
                    progressBar_->setValue(100);
                    progressBar_->hide();
                    return;
                }

                progressBar_->setValue(progress);
                progressBar_->show();
                progressBar_->raise();
            }
        );

        connect(
            view,
            &QWebEngineView::loadFinished,
            this,
            [this, view](bool)
            {
                progressBar_->hide();
                updateWindowTitle();
                siteTheme(siteTheme_);
            }
        );

        view->load(url);
        view->setFocus();

        return view;
    }


    BrowserTab *currentTab() const
    {
        return static_cast<BrowserTab *>(
            tabs_->currentWidget()
        );
    }


    void open(
        const QUrl &url
    )
    {
        if (auto *view = currentTab())
            view->load(url);
    }


    void search(
        const QString &query,
        const QString &site = QString()
    )
    {
        QString encoded =
            QString::fromUtf8(
                QUrl::toPercentEncoding(query)
            );

        if (site == "wikipedia")
        {
            open(
                QUrl(
                    "https://en.wikipedia.org/wiki/"
                    "Special:Search?search=" +
                    encoded
                )
            );
        }
        else if (site == "github")
        {
            open(
                QUrl(
                    "https://github.com/search?q=" +
                    encoded
                )
            );
        }
        else if (site == "youtube")
        {
            open(
                QUrl(
                    "https://www.youtube.com/results?"
                    "search_query=" +
                    encoded
                )
            );
        }
        else if (site == "reddit")
        {
            open(
                QUrl(
                    "https://www.reddit.com/search/?q=" +
                    encoded
                )
            );
        }
        else if (site == "google")
        {
            open(
                QUrl(
                    "https://www.google.com/search?q=" +
                    encoded
                )
            );
        }
        else if (site == "images")
        {
            open(
                QUrl(
                    "https://duckduckgo.com/?iax=images"
                    "&ia=images&q=" +
                    encoded
                )
            );
        }
        else if (site == "news")
        {
            open(
                QUrl(
                    "https://duckduckgo.com/?iar=news"
                    "&ia=news&q=" +
                    encoded
                )
            );
        }
        else
        {
            open(
                QUrl(
                    "https://duckduckgo.com/?q=" +
                    encoded
                )
            );
        }
    }


    void back()
    {
        if (auto *view = currentTab())
            view->back();
    }


    void forward()
    {
        if (auto *view = currentTab())
            view->forward();
    }


    void reload()
    {
        if (auto *view = currentTab())
            view->reload();
    }


    void stop()
    {
        if (auto *view = currentTab())
            view->stop();
    }


    void nextTab()
    {
        int count =
            tabs_->count();

        if (count == 0)
            return;

        int index =
            tabs_->currentIndex() + 1;

        if (index >= count)
            index = 0;

        tabs_->setCurrentIndex(index);
        currentTabFocus();
    }


    void previousTab()
    {
        int count =
            tabs_->count();

        if (count == 0)
            return;

        int index =
            tabs_->currentIndex() - 1;

        if (index < 0)
            index = count - 1;

        tabs_->setCurrentIndex(index);
        currentTabFocus();
    }


    void selectTab(
        int index
    )
    {
        if (
            index >= 0 &&
            index < tabs_->count()
        )
        {
            tabs_->setCurrentIndex(index);
            currentTabFocus();
        }
    }


    void closeCurrentTab()
    {
        int index =
            tabs_->currentIndex();

        if (index < 0)
            return;

        if (tabs_->count() == 1)
        {
            close();
            return;
        }

        QWidget *widget =
            tabs_->widget(index);

        tabs_->removeTab(index);
        widget->deleteLater();

        currentTabFocus();
    }


    void zoomIn()
    {
        if (auto *view = currentTab())
        {
            view->setZoomFactor(
                view->zoomFactor() + 0.1
            );
        }
    }


    void zoomOut()
    {
        if (auto *view = currentTab())
        {
            view->setZoomFactor(
                view->zoomFactor() - 0.1
            );
        }
    }


    void zoom(
        int percentage
    )
    {
        if (auto *view = currentTab())
        {
            view->setZoomFactor(
                percentage / 100.0
            );
        }
    }


    void fullscreen()
    {
        if (isFullScreen())
            showNormal();
        else
            showFullScreen();
    }


    void exitFullscreen()
    {
        if (isFullScreen())
            showNormal();
    }


    void javascript(
        bool enabled
    )
    {
        if (auto *view = currentTab())
        {
            view->settings()->setAttribute(
                QWebEngineSettings::JavascriptEnabled,
                enabled
            );

            view->reload();
        }
    }


    void siteTheme(
        const QString &theme
    )
    {
        auto *view = currentTab();

        if (!view)
            return;

        const QString script =
            QString(R"JS(
                (() => {
                    const theme = '%1';
                    const root = document.documentElement;
                    const styleId = 'lewweb-site-theme';

                    root.style.setProperty(
                        'color-scheme',
                        theme,
                        'important'
                    );

                    let meta =
                        document.querySelector(
                            'meta[name="color-scheme"]'
                        );

                    if (!meta)
                    {
                        meta = document.createElement('meta');
                        meta.name = 'color-scheme';
                        document.head.appendChild(meta);
                    }

                    meta.content = theme;
                    root.setAttribute(
                        'data-lewweb-theme',
                        theme
                    );

                    let style =
                        document.getElementById(styleId);

                    if (style)
                        style.remove();

                    if (theme === 'dark')
                    {
                        style = document.createElement('style');
                        style.id = styleId;
                        style.textContent = `
                            html, body {
                                background-color: #121212 !important;
                                color: #e8e8e8 !important;
                            }

                            body *:not(img):not(video):not(canvas):not(svg):not(path) {
                                color: #e8e8e8 !important;
                                border-color: #444 !important;
                            }

                            body, main, section, article, aside, header,
                            footer, nav, div, form, fieldset, table, tr,
                            td, th, pre, blockquote, textarea, input,
                            select, button {
                                background-color: #121212 !important;
                            }

                            a {
                                color: #8ab4f8 !important;
                            }

                            input, textarea, select, button {
                                color: #e8e8e8 !important;
                                background-color: #1e1e1e !important;
                                border-color: #555 !important;
                            }

                            ::placeholder {
                                color: #aaa !important;
                            }
                        `;
                        (document.head || document.documentElement)
                            .appendChild(style);
                    }
                })();
            )JS").arg(theme);

        view->page()->runJavaScript(script);
    }

    QString currentUrl() const
    {
        if (auto *view = currentTab())
            return view->url().toString();

        return {};
    }


    void listTabs()
    {
        qInfo() << "TABS";

        for (
            int i = 0;
            i < tabs_->count();
            ++i
        )
        {
            auto *view =
                static_cast<BrowserTab *>(
                    tabs_->widget(i)
                );

            if (!view)
                continue;

            QString title =
                view->title();

            if (title.isEmpty())
                title = "New Tab";

            qInfo().noquote()
                << QString("[%1] %2")
                    .arg(i + 1)
                    .arg(title);
        }
    }


    void saveCurrentBookmark(
        const QString &name
    )
    {
        QString url =
            currentUrl();

        if (!url.isEmpty())
            addBookmark(name, url);
    }


    void downloadFile(
        const QString &url,
        const QString &directory
    )
    {
        if (
            url.trimmed().isEmpty() ||
            directory.trimmed().isEmpty()
        )
        {
            qWarning()
                << "LewWeb: download requires URL and directory.";

            return;
        }

        QDir dir(directory);

        if (!dir.exists())
        {
            if (!dir.mkpath("."))
            {
                qWarning()
                    << "LewWeb: could not create:"
                    << directory;

                return;
            }
        }

        pendingDownloadDirectory_ =
            QDir(directory).absolutePath();

        QUrl target(url);

        if (!target.isValid())
        {
            pendingDownloadDirectory_.clear();

            qWarning()
                << "LewWeb: invalid download URL:"
                << url;

            return;
        }

        if (auto *view = currentTab())
        {
            view->page()->download(target);
        }
        else
        {
            pendingDownloadDirectory_.clear();

            qWarning()
                << "LewWeb: no browser tab available.";
        }
    }


    void downloadPinterestImage(
        const QString &url,
        const QString &directory
    )
    {
        if (
            url.trimmed().isEmpty() ||
            directory.trimmed().isEmpty()
        )
        {
            return;
        }

        runPinterestDownloader(
            url,
            directory
        );
    }


private:
    void applyConfig()
    {
        if (config_.windowBorder == "borderless")
        {
            setWindowFlags(
                Qt::FramelessWindowHint |
                Qt::Window
            );
        }
        else
        {
            setWindowFlags(
                Qt::Window
            );
        }
    }


    void applyWidgetStyle()
    {
        QString browserColour =
            QColor(config_.browserColour)
                .name();

        QString borderColour =
            QColor(config_.borderColour)
                .name();

        QString backgroundColour =
            config_.backgroundColour;

        QString textColour =
            config_.textColour;

        if (backgroundColour.isEmpty())
        {
            if (config_.theme == "dark")
                backgroundColour = "#181818";
            else
                backgroundColour = "#ffffff";
        }

        if (textColour.isEmpty())
        {
            if (config_.theme == "dark")
                textColour = "#ffffff";
            else
                textColour = "#111111";
        }

        QString windowStyle;

        if (config_.windowBorder == "borderless")
        {
            windowStyle =
                QString(
                    "QMainWindow {"
                    "border: 2px solid %1;"
                    "background: %2;"
                    "}"
                )
                .arg(
                    borderColour,
                    backgroundColour
                );
        }
        else
        {
            windowStyle =
                QString(
                    "QMainWindow {"
                    "background: %1;"
                    "}"
                )
                .arg(backgroundColour);
        }

        QString commandStyle =
            QString(
                "QWidget#commandPalette {"
                "background: qlineargradient("
                    "x1:0, y1:0, x2:0, y2:1,"
                    "stop:0 #f5f5f5,"
                    "stop:0.08 #dedede,"
                    "stop:0.5 #bcbcbc,"
                    "stop:0.92 #929292,"
                    "stop:1 #737373"
                ");"
                "border: 1px solid #4a4a4a;"
                "border-radius: 7px;"
                "}"
                "QLabel {"
                "color: #202020;"
                "font-family: monospace;"
                "font-size: 14px;"
                "font-weight: bold;"
                "}"
                "QLineEdit {"
                "background: rgba(255,255,255,220);"
                "color: #111111;"
                "border: 1px solid #666666;"
                "border-top-color: #333333;"
                "border-radius: 3px;"
                "padding: 5px 8px;"
                "font-family: monospace;"
                "font-size: 14px;"
                "selection-background-color: %1;"
                "}"
                "QAbstractItemView {"
                "background: #eeeeee;"
                "color: #111111;"
                "border: 1px solid #555555;"
                "selection-background-color: %1;"
                "selection-color: white;"
                "font-family: monospace;"
                "font-size: 13px;"
                "}"
            )
            .arg(browserColour);

        QString progressStyle =
            QString(
                "QProgressBar {"
                "background: transparent;"
                "border: none;"
                "}"
                "QProgressBar::chunk {"
                "background: %1;"
                "}"
            )
            .arg(browserColour);

        setStyleSheet(
            windowStyle +
            commandStyle
        );

        if (commandPalette_)
            commandPalette_->setStyleSheet(
                commandStyle
            );

        if (progressBar_)
            progressBar_->setStyleSheet(
                progressStyle
            );

        QPalette palette =
            QApplication::palette();

        palette.setColor(
            QPalette::Window,
            QColor(backgroundColour)
        );

        palette.setColor(
            QPalette::Base,
            QColor(backgroundColour)
        );

        palette.setColor(
            QPalette::Text,
            QColor(textColour)
        );

        palette.setColor(
            QPalette::WindowText,
            QColor(textColour)
        );

        setPalette(palette);
    }


    void executeCommand(
        QString command
    )
    {
        command = command.trimmed();

        if (command.startsWith("lewweb"))
        {
            command =
                command
                    .mid(
                        QString("lewweb").length()
                    )
                    .trimmed();
        }

        if (command.isEmpty())
            return;

        QStringList parts =
            command.split(
                ' ',
                Qt::SkipEmptyParts
            );

        if (parts.isEmpty())
            return;

        QString action =
            parts.first();

        parts.removeFirst();

        if (action == "--search")
        {
            QStringList queryParts;
            QString site;

            for (const QString &part : parts)
            {
                if (part == "--wikipedia")
                    site = "wikipedia";
                else if (part == "--github")
                    site = "github";
                else if (part == "--youtube")
                    site = "youtube";
                else if (part == "--reddit")
                    site = "reddit";
                else if (part == "--google")
                    site = "google";
                else if (part == "--images")
                    site = "images";
                else if (part == "--news")
                    site = "news";
                else
                    queryParts.append(part);
            }

            search(
                queryParts.join(" "),
                site
            );
        }
        else if (action == "--open")
        {
            if (!parts.isEmpty())
                open(QUrl(parts.join(" ")));
        }
        else if (action == "--home")
        {
            open(QUrl(config_.homepage));
        }
        else if (action == "--back")
        {
            back();
        }
        else if (action == "--forward")
        {
            forward();
        }
        else if (action == "--reload")
        {
            reload();
        }
        else if (action == "--stop")
        {
            stop();
        }
        else if (action == "--new-tab")
        {
            addTab(QUrl(config_.homepage));
        }
        else if (action == "--close-tab")
        {
            closeCurrentTab();
        }
        else if (action == "--next-tab")
        {
            nextTab();
        }
        else if (action == "--previous-tab")
        {
            previousTab();
        }
        else if (action == "--tab")
        {
            if (!parts.isEmpty())
                selectTab(
                    parts.first().toInt() - 1
                );
        }
        else if (action == "--zoom-in")
        {
            zoomIn();
        }
        else if (action == "--zoom-out")
        {
            zoomOut();
        }
        else if (action == "--zoom")
        {
            if (!parts.isEmpty())
                zoom(parts.first().toInt());
        }
        else if (action == "--fullscreen")
        {
            fullscreen();
        }
        else if (action == "--exit-fullscreen")
        {
            exitFullscreen();
        }
        else if (action == "--javascript")
        {
            if (!parts.isEmpty())
            {
                javascript(
                    parts.first().toLower() != "off"
                );
            }
        }
        else if (action == "--light-mode")
        {
            siteTheme_ = "light";
            siteTheme(siteTheme_);
        }
        else if (action == "--dark-mode")
        {
            siteTheme_ = "dark";
            siteTheme(siteTheme_);
        }
        else if (action == "--download-file")
        {
            if (parts.size() >= 2)
            {
                downloadFile(
                    parts.at(0),
                    parts.at(1)
                );
            }
        }
        else if (action == "--download-pinterest-image")
        {
            if (parts.size() >= 2)
            {
                downloadPinterestImage(
                    parts.at(0),
                    parts.at(1)
                );
            }
        }
        else if (action == "--list-tabs")
        {
            listTabs();
        }
        else if (action == "--new-bookmark")
        {
            if (parts.size() >= 2)
            {
                addBookmark(
                    parts.first(),
                    parts.mid(1).join(" ")
                );
            }
        }
        else if (action == "--save-bookmark")
        {
            if (!parts.isEmpty())
                saveCurrentBookmark(parts.first());
        }
        else if (action == "--open-bookmark")
        {
            if (!parts.isEmpty())
            {
                QString url =
                    bookmarkUrl(parts.first());

                if (!url.isEmpty())
                    open(QUrl(url));
            }
        }
        else if (action == "--delete-bookmark")
        {
            if (!parts.isEmpty())
                removeBookmark(parts.first());
        }
        else if (action == "--help")
        {
            printHelp();
        }
        else if (action == "--version")
        {
            printVersion();
        }
    }


    void updateWindowTitle()
    {
        auto *view =
            currentTab();

        if (!view)
        {
            setWindowTitle("LewWeb");
            return;
        }

        QString title =
            view->title();

        if (title.isEmpty())
            title = "LewWeb";

        setWindowTitle(
            title + " — LewWeb"
        );
    }


    QTabWidget *tabs_;
    QWebEngineProfile *profile_;
    QWidget *commandPalette_;
    QLabel *commandPrompt_;
    QLineEdit *commandBar_;
    QProgressBar *progressBar_;

    BrowserConfig config_;

    QString siteTheme_ = "light";

    QString pendingDownloadDirectory_;
};


static void printHelp()
{
    qInfo().noquote()
        << R"(LEWWEB HELP                                      lewweb

lewweb(1)               LewWeb Manual               lewweb(1)

NAME
    lewweb - terminal-controlled Qt web browser

SYNOPSIS
    lewweb [options]

DESCRIPTION
    LewWeb is a terminal-driven web browser using
    Qt WebEngine and Chromium.

NAVIGATION
    --open URL
    --search QUERY
    --home
    --back
    --forward
    --reload
    --stop

SEARCH
    --wikipedia
    --github
    --youtube
    --reddit
    --google
    --images
    --news

TABS
    --new-tab
    --close-tab
    --tab N
    --next-tab
    --previous-tab
    --list-tabs

DOWNLOADS
    --download-file URL DIR
        Download a direct file using Qt WebEngine.

    --download-pinterest-image URL DIR
        Download a Pinterest image using lew-dlp.

BOOKMARKS
    --new-bookmark NAME URL
    --save-bookmark NAME
    --open-bookmark NAME
    --delete-bookmark NAME

CONFIGURATION
    --config FILE
        Load browser configuration from a JSON file.

    Example:
        lewweb --config ~/Documents/lewweb.json

    Supported configuration keys:

        window_border
            "default"
            "borderless"

        theme
            "light"
            "dark"

        browser_colour
            CSS/hex colour used by LewWeb's browser accent.

        border_colour
            CSS/hex colour used for the borderless window.

        background_colour
            CSS/hex background colour.

        text_colour
            CSS/hex text colour.

        homepage
            Default homepage/new-tab URL.

        search_engine
            Search engine identifier.

        zoom
            Default zoom percentage.

BROWSER
    --fullscreen
    --exit-fullscreen
    --javascript on
    --javascript off
    --light-mode
    --dark-mode
    --zoom-in
    --zoom-out
    --zoom N
    --quiet

KEYBOARD
    Cmd+/                open LewWeb reference

    Cmd+T                command palette
    Cmd+W                close tab
    Cmd+R                reload
    Cmd+Left             back
    Cmd+Right            forward
    Cmd+1 ... Cmd+9      select tab

    Ctrl+T               new tab
    Ctrl+W               close tab
    Ctrl+Tab             next tab
    Ctrl+Shift+Tab       previous tab

    Alt+Left             back
    Alt+Right            forward

    + / =                 zoom in
    -                     zoom out
    F11                   browser fullscreen

EXAMPLES
    lewweb --home
    lewweb --search "jupiter" --wikipedia
    lewweb --open "https://github.com"

    lewweb --config ~/Documents/lewweb.json

    lewweb --download-file \
        "https://example.com/file.pdf" \
        ~/Downloads

    lewweb --download-pinterest-image \
        "https://www.pinterest.com/pin/123456789/" \
        ~/Pictures

    lewweb --javascript on
    lewweb --javascript off

    lewweb --save-bookmark github
    lewweb --open-bookmark github
    lewweb --new-bookmark github https://github.com
    lewweb --delete-bookmark github

)";
}


static void printVersion()
{
    qInfo().noquote()
        << "LewWeb "
        << LEWWEB_VERSION;
}


static bool sendCommand(
    const QByteArray &command
)
{
    QLocalSocket socket;

    socket.connectToServer(
        SERVER_NAME
    );

    if (!socket.waitForConnected(250))
        return false;

    socket.write(command);
    socket.flush();
    socket.waitForBytesWritten(250);
    socket.disconnectFromServer();

    return true;
}


static void setupServer(
    QLocalServer &server,
    BrowserWindow &window
)
{
    QLocalServer::removeServer(
        SERVER_NAME
    );

    server.listen(
        SERVER_NAME
    );

    QObject::connect(
        &server,
        &QLocalServer::newConnection,
        &window,
        [&server, &window]()
        {
            while (
                server.hasPendingConnections()
            )
            {
                QLocalSocket *socket =
                    server.nextPendingConnection();

                QObject::connect(
                    socket,
                    &QLocalSocket::readyRead,
                    socket,
                    [socket, &window]()
                    {
                        QList<QByteArray> commands =
                            socket
                                ->readAll()
                                .split('\n');

                        for (
                            const QByteArray &command :
                            commands
                        )
                        {
                            if (command.isEmpty())
                                continue;

                            QJsonParseError error;

                            QJsonDocument document =
                                QJsonDocument::fromJson(
                                    command,
                                    &error
                                );

                            if (
                                error.error !=
                                QJsonParseError::NoError
                            )
                            {
                                continue;
                            }

                            QJsonObject object =
                                document.object();

                            QString action =
                                object["action"]
                                    .toString();

                            if (action == "open")
                            {
                                window.open(
                                    QUrl(
                                        object["url"]
                                            .toString()
                                    )
                                );
                            }
                            else if (action == "search")
                            {
                                window.search(
                                    object["query"]
                                        .toString(),
                                    object["site"]
                                        .toString()
                                );
                            }
                            else if (action == "home")
                            {
                                window.open(
                                    QUrl(
                                        "https://duckduckgo.com/"
                                    )
                                );
                            }
                            else if (action == "back")
                            {
                                window.back();
                            }
                            else if (action == "forward")
                            {
                                window.forward();
                            }
                            else if (action == "reload")
                            {
                                window.reload();
                            }
                            else if (action == "stop")
                            {
                                window.stop();
                            }
                            else if (action == "new-tab")
                            {
                                QString url =
                                    object["url"]
                                        .toString();

                                if (url.isEmpty())
                                {
                                    url =
                                        "https://duckduckgo.com/";
                                }

                                window.addTab(
                                    QUrl(url)
                                );
                            }
                            else if (action == "close-tab")
                            {
                                window.closeCurrentTab();
                            }
                            else if (action == "next-tab")
                            {
                                window.nextTab();
                            }
                            else if (action == "previous-tab")
                            {
                                window.previousTab();
                            }
                            else if (action == "tab")
                            {
                                window.selectTab(
                                    object["index"]
                                        .toInt() - 1
                                );
                            }
                            else if (action == "zoom-in")
                            {
                                window.zoomIn();
                            }
                            else if (action == "zoom-out")
                            {
                                window.zoomOut();
                            }
                            else if (action == "zoom")
                            {
                                window.zoom(
                                    object["value"]
                                        .toInt()
                                );
                            }
                            else if (action == "fullscreen")
                            {
                                window.fullscreen();
                            }
                            else if (action == "exit-fullscreen")
                            {
                                window.exitFullscreen();
                            }
                            else if (action == "javascript")
                            {
                                window.javascript(
                                    object["enabled"]
                                        .toBool()
                                );
                            }
                            else if (action == "light-mode")
                            {
                                window.siteTheme("light");
                            }
                            else if (action == "dark-mode")
                            {
                                window.siteTheme("dark");
                            }
                            else if (action == "download-file")
                            {
                                window.downloadFile(
                                    object["url"]
                                        .toString(),
                                    object["directory"]
                                        .toString()
                                );
                            }
                            else if (
                                action ==
                                "download-pinterest-image"
                            )
                            {
                                window.downloadPinterestImage(
                                    object["url"]
                                        .toString(),
                                    object["directory"]
                                        .toString()
                                );
                            }
                            else if (action == "list-tabs")
                            {
                                window.listTabs();
                            }
                            else if (action == "save-bookmark")
                            {
                                window.saveCurrentBookmark(
                                    object["name"]
                                        .toString()
                                );
                            }
                            else if (action == "open-bookmark")
                            {
                                QString url =
                                    bookmarkUrl(
                                        object["name"]
                                            .toString()
                                    );

                                if (!url.isEmpty())
                                {
                                    window.open(
                                        QUrl(url)
                                    );
                                }
                            }
                            else if (action == "new-bookmark")
                            {
                                addBookmark(
                                    object["name"]
                                        .toString(),
                                    object["url"]
                                        .toString()
                                );
                            }
                            else if (action == "delete-bookmark")
                            {
                                removeBookmark(
                                    object["name"]
                                        .toString()
                                );
                            }
                            else if (action == "config")
                            {
                                QString path =
                                    object["path"]
                                        .toString();

                                if (!path.isEmpty())
                                {
                                    BrowserConfig config =
                                        loadConfig(path);

                                    window.applyConfig(
                                        config
                                    );
                                }
                            }
                        }

                        socket->disconnectFromServer();
                    }
                );
            }
        }
    );
}


int main(
    int argc,
    char *argv[]
)
{
    for (
        int i = 1;
        i < argc;
        ++i
    )
    {
        if (
            QString::fromLocal8Bit(argv[i]) ==
            "--quiet"
        )
        {
            qputenv(
                "QT_LOGGING_RULES",
                "*.debug=false;"
                "*.info=false;"
                "*.warning=false;"
                "qt.webenginecontext=false;"
                "qt.webengine*=false"
            );

            break;
        }
    }


    QApplication app(
        argc,
        argv
    );


    QCoreApplication::setApplicationName(
        "lewweb"
    );

    QCoreApplication::setApplicationVersion(
        LEWWEB_VERSION
    );


    QCommandLineParser parser;

    parser.setApplicationDescription(
        "Terminal-controlled Qt web browser"
    );

    parser.addHelpOption();
    parser.addVersionOption();


    QCommandLineOption quietOption(
        "quiet",
        "Suppress Qt and Chromium logging."
    );

    QCommandLineOption configOption(
        "config",
        "Load browser configuration from a JSON file.",
        "file"
    );

    QCommandLineOption openOption(
        "open",
        "Open URL.",
        "url"
    );

    QCommandLineOption searchOption(
        "search",
        "Search DuckDuckGo.",
        "query"
    );

    QCommandLineOption wikipediaOption(
        "wikipedia",
        "Search Wikipedia."
    );

    QCommandLineOption githubOption(
        "github",
        "Search GitHub."
    );

    QCommandLineOption youtubeOption(
        "youtube",
        "Search YouTube."
    );

    QCommandLineOption redditOption(
        "reddit",
        "Search Reddit."
    );

    QCommandLineOption googleOption(
        "google",
        "Search Google."
    );

    QCommandLineOption imagesOption(
        "images",
        "Search DuckDuckGo Images."
    );

    QCommandLineOption newsOption(
        "news",
        "Search DuckDuckGo News."
    );

    QCommandLineOption homeOption(
        "home",
        "Open configured homepage."
    );

    QCommandLineOption backOption(
        "back",
        "Go back."
    );

    QCommandLineOption forwardOption(
        "forward",
        "Go forward."
    );

    QCommandLineOption reloadOption(
        "reload",
        "Reload page."
    );

    QCommandLineOption stopOption(
        "stop",
        "Stop loading."
    );

    QCommandLineOption newTabOption(
        "new-tab",
        "Open a new tab."
    );

    QCommandLineOption closeTabOption(
        "close-tab",
        "Close current tab."
    );

    QCommandLineOption tabOption(
        "tab",
        "Select tab.",
        "number"
    );

    QCommandLineOption nextTabOption(
        "next-tab",
        "Select next tab."
    );

    QCommandLineOption previousTabOption(
        "previous-tab",
        "Select previous tab."
    );

    QCommandLineOption listTabsOption(
        "list-tabs",
        "List tabs."
    );

    QCommandLineOption fullscreenOption(
        "fullscreen",
        "Toggle fullscreen."
    );

    QCommandLineOption exitFullscreenOption(
        "exit-fullscreen",
        "Exit fullscreen."
    );

    QCommandLineOption javascriptOption(
        "javascript",
        "Enable or disable JavaScript.",
        "on|off"
    );

    QCommandLineOption lightModeOption(
        "light-mode",
        "Force a light colour scheme for web content."
    );

    QCommandLineOption darkModeOption(
        "dark-mode",
        "Force a dark colour scheme for web content."
    );

    QCommandLineOption zoomInOption(
        "zoom-in",
        "Increase zoom."
    );

    QCommandLineOption zoomOutOption(
        "zoom-out",
        "Decrease zoom."
    );

    QCommandLineOption zoomOption(
        "zoom",
        "Set zoom percentage.",
        "percentage"
    );

    QCommandLineOption downloadFileOption(
        "download-file",
        "Download a file using Qt WebEngine.",
        "url"
    );

    QCommandLineOption downloadPinterestOption(
        "download-pinterest-image",
        "Download a Pinterest image using lew-dlp.",
        "url"
    );

    QCommandLineOption newBookmarkOption(
        "new-bookmark",
        "Create a bookmark.",
        "name"
    );

    QCommandLineOption saveBookmarkOption(
        "save-bookmark",
        "Save current page as a bookmark.",
        "name"
    );

    QCommandLineOption openBookmarkOption(
        "open-bookmark",
        "Open a bookmark.",
        "name"
    );

    QCommandLineOption deleteBookmarkOption(
        "delete-bookmark",
        "Delete a bookmark.",
        "name"
    );


    parser.addOption(quietOption);
    parser.addOption(configOption);

    parser.addOption(openOption);
    parser.addOption(searchOption);

    parser.addOption(wikipediaOption);
    parser.addOption(githubOption);
    parser.addOption(youtubeOption);
    parser.addOption(redditOption);
    parser.addOption(googleOption);
    parser.addOption(imagesOption);
    parser.addOption(newsOption);

    parser.addOption(homeOption);

    parser.addOption(backOption);
    parser.addOption(forwardOption);
    parser.addOption(reloadOption);
    parser.addOption(stopOption);

    parser.addOption(newTabOption);
    parser.addOption(closeTabOption);
    parser.addOption(tabOption);
    parser.addOption(nextTabOption);
    parser.addOption(previousTabOption);
    parser.addOption(listTabsOption);

    parser.addOption(fullscreenOption);
    parser.addOption(exitFullscreenOption);
    parser.addOption(javascriptOption);
    parser.addOption(lightModeOption);
    parser.addOption(darkModeOption);
    parser.addOption(zoomInOption);
    parser.addOption(zoomOutOption);
    parser.addOption(zoomOption);

    parser.addOption(downloadFileOption);
    parser.addOption(downloadPinterestOption);

    parser.addOption(newBookmarkOption);
    parser.addOption(saveBookmarkOption);
    parser.addOption(openBookmarkOption);
    parser.addOption(deleteBookmarkOption);


    parser.process(app);


    if (parser.isSet("version"))
    {
        printVersion();
        return 0;
    }


    bool hasAction =
        parser.isSet(configOption) ||
        parser.isSet(openOption) ||
        parser.isSet(searchOption) ||
        parser.isSet(homeOption) ||
        parser.isSet(backOption) ||
        parser.isSet(forwardOption) ||
        parser.isSet(reloadOption) ||
        parser.isSet(stopOption) ||
        parser.isSet(newTabOption) ||
        parser.isSet(closeTabOption) ||
        parser.isSet(tabOption) ||
        parser.isSet(nextTabOption) ||
        parser.isSet(previousTabOption) ||
        parser.isSet(listTabsOption) ||
        parser.isSet(fullscreenOption) ||
        parser.isSet(exitFullscreenOption) ||
        parser.isSet(javascriptOption) ||
        parser.isSet(lightModeOption) ||
        parser.isSet(darkModeOption) ||
        parser.isSet(zoomInOption) ||
        parser.isSet(zoomOutOption) ||
        parser.isSet(zoomOption) ||
        parser.isSet(downloadFileOption) ||
        parser.isSet(downloadPinterestOption) ||
        parser.isSet(newBookmarkOption) ||
        parser.isSet(saveBookmarkOption) ||
        parser.isSet(openBookmarkOption) ||
        parser.isSet(deleteBookmarkOption);


    if (hasAction)
    {
        QJsonObject command;


        if (parser.isSet(configOption))
        {
            command["action"] = "config";
            command["path"] =
                QFileInfo(
                    parser.value(configOption)
                ).absoluteFilePath();
        }
        else if (parser.isSet(openOption))
        {
            command["action"] = "open";
            command["url"] =
                parser.value(openOption);
        }
        else if (parser.isSet(searchOption))
        {
            command["action"] = "search";
            command["query"] =
                parser.value(searchOption);

            if (parser.isSet(wikipediaOption))
                command["site"] = "wikipedia";
            else if (parser.isSet(githubOption))
                command["site"] = "github";
            else if (parser.isSet(youtubeOption))
                command["site"] = "youtube";
            else if (parser.isSet(redditOption))
                command["site"] = "reddit";
            else if (parser.isSet(googleOption))
                command["site"] = "google";
            else if (parser.isSet(imagesOption))
                command["site"] = "images";
            else if (parser.isSet(newsOption))
                command["site"] = "news";
        }
        else if (parser.isSet(homeOption))
        {
            command["action"] = "home";
        }
        else if (parser.isSet(backOption))
        {
            command["action"] = "back";
        }
        else if (parser.isSet(forwardOption))
        {
            command["action"] = "forward";
        }
        else if (parser.isSet(reloadOption))
        {
            command["action"] = "reload";
        }
        else if (parser.isSet(stopOption))
        {
            command["action"] = "stop";
        }
        else if (parser.isSet(newTabOption))
        {
            command["action"] = "new-tab";
        }
        else if (parser.isSet(closeTabOption))
        {
            command["action"] = "close-tab";
        }
        else if (parser.isSet(nextTabOption))
        {
            command["action"] = "next-tab";
        }
        else if (parser.isSet(previousTabOption))
        {
            command["action"] = "previous-tab";
        }
        else if (parser.isSet(tabOption))
        {
            command["action"] = "tab";
            command["index"] =
                parser.value(tabOption).toInt();
        }
        else if (parser.isSet(listTabsOption))
        {
            command["action"] = "list-tabs";
        }
        else if (parser.isSet(fullscreenOption))
        {
            command["action"] = "fullscreen";
        }
        else if (parser.isSet(exitFullscreenOption))
        {
            command["action"] = "exit-fullscreen";
        }
        else if (parser.isSet(javascriptOption))
        {
            command["action"] = "javascript";
            command["enabled"] =
                parser.value(
                    javascriptOption
                ).toLower() != "off";
        }
        else if (parser.isSet(lightModeOption))
        {
            command["action"] = "light-mode";
        }
        else if (parser.isSet(darkModeOption))
        {
            command["action"] = "dark-mode";
        }
        else if (parser.isSet(zoomInOption))
        {
            command["action"] = "zoom-in";
        }
        else if (parser.isSet(zoomOutOption))
        {
            command["action"] = "zoom-out";
        }
        else if (parser.isSet(zoomOption))
        {
            command["action"] = "zoom";
            command["value"] =
                parser.value(zoomOption).toInt();
        }
        else if (parser.isSet(downloadFileOption))
        {
            command["action"] =
                "download-file";

            command["url"] =
                parser.value(downloadFileOption);

            const QStringList positional =
                parser.positionalArguments();

            if (!positional.isEmpty())
                command["directory"] =
                    positional.first();
        }
        else if (
            parser.isSet(downloadPinterestOption)
        )
        {
            command["action"] =
                "download-pinterest-image";

            command["url"] =
                parser.value(
                    downloadPinterestOption
                );

            const QStringList positional =
                parser.positionalArguments();

            if (!positional.isEmpty())
                command["directory"] =
                    positional.first();
        }
        else if (parser.isSet(newBookmarkOption))
        {
            command["action"] = "new-bookmark";
            command["name"] =
                parser.value(newBookmarkOption);

            const QStringList positional =
                parser.positionalArguments();

            if (!positional.isEmpty())
                command["url"] =
                    positional.first();
        }
        else if (parser.isSet(saveBookmarkOption))
        {
            command["action"] = "save-bookmark";
            command["name"] =
                parser.value(saveBookmarkOption);
        }
        else if (parser.isSet(openBookmarkOption))
        {
            command["action"] = "open-bookmark";
            command["name"] =
                parser.value(openBookmarkOption);
        }
        else if (parser.isSet(deleteBookmarkOption))
        {
            command["action"] = "delete-bookmark";
            command["name"] =
                parser.value(deleteBookmarkOption);
        }


        QByteArray payload =
            QJsonDocument(command)
                .toJson(
                    QJsonDocument::Compact
                );

        payload.append('\n');


        if (sendCommand(payload))
            return 0;
    }


    BrowserConfig config;

    if (parser.isSet(configOption))
    {
        config =
            loadConfig(
                QFileInfo(
                    parser.value(configOption)
                ).absoluteFilePath()
            );
    }


    QWebEngineProfile *profile =
        new QWebEngineProfile(
            "LewWeb",
            &app
        );


    QString path =
        profilePath();


    profile->setPersistentStoragePath(
        path
    );

    profile->setCachePath(
        QDir(path).filePath("cache")
    );

    profile->setPersistentCookiesPolicy(
        QWebEngineProfile::ForcePersistentCookies
    );


    BrowserWindow window(
        profile,
        config
    );


    QLocalServer server;

    setupServer(
        server,
        window
    );


    window.show();


    if (parser.isSet(openOption))
    {
        window.open(
            QUrl(
                parser.value(openOption)
            )
        );
    }
    else if (parser.isSet(searchOption))
    {
        QString site;

        if (parser.isSet(wikipediaOption))
            site = "wikipedia";
        else if (parser.isSet(githubOption))
            site = "github";
        else if (parser.isSet(youtubeOption))
            site = "youtube";
        else if (parser.isSet(redditOption))
            site = "reddit";
        else if (parser.isSet(googleOption))
            site = "google";
        else if (parser.isSet(imagesOption))
            site = "images";
        else if (parser.isSet(newsOption))
            site = "news";

        window.search(
            parser.value(searchOption),
            site
        );
    }
    else if (parser.isSet(homeOption))
    {
        window.open(
            QUrl(config.homepage)
        );
    }
    else if (parser.isSet(lightModeOption))
    {
        window.siteTheme("light");
    }
    else if (parser.isSet(darkModeOption))
    {
        window.siteTheme("dark");
    }


    return app.exec();
}
