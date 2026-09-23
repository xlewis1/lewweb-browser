#include <QApplication>
#include <QAction>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QShortcut>
#include <QStandardPaths>
#include <QStyle>
#include <QTabBar>
#include <QTabWidget>
#include <QToolButton>
#include <QUrl>
#include <QSignalBlocker>
#include <QColor>
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QWebEngineDownloadRequest>
#include <QWebEngineHistory>
#include <QWebEngineFullScreenRequest>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineView>

#include <functional>
#include <utility>

constexpr const char *LEWWEB_VERSION = "1.0.0";

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

struct HistoryEntry
{
    QString title;
    QString url;
};

static BrowserConfig defaultConfig()
{
    return BrowserConfig{};
}

static QString profilePath()
{
    QString base =
        QStandardPaths::writableLocation(
            QStandardPaths::AppDataLocation
        );

    QDir dir(base);

    if (!dir.exists())
        dir.mkpath(".");

    QString path = dir.filePath("profile");

    QDir profile(path);

    if (!profile.exists())
        profile.mkpath(".");

    return path;
}

static QString configPath()
{
    return QDir(profilePath()).filePath("lewweb.json");
}

static QString bookmarksPath()
{
    return QDir(profilePath()).filePath("bookmarks.json");
}

static QString historyPath()
{
    return QDir(profilePath()).filePath("history.json");
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

static bool saveBookmarks(const QJsonObject &bookmarks)
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

    QJsonObject bookmarks = loadBookmarks();
    bookmarks[name] = url;

    return saveBookmarks(bookmarks);
}

static bool removeBookmark(const QString &name)
{
    QJsonObject bookmarks = loadBookmarks();

    if (!bookmarks.contains(name))
        return false;

    bookmarks.remove(name);
    return saveBookmarks(bookmarks);
}

static QString bookmarkUrl(const QString &name)
{
    return loadBookmarks()
        .value(name)
        .toString();
}

static QList<HistoryEntry> loadHistory()
{
    QList<HistoryEntry> history;

    QFile file(historyPath());

    if (!file.open(QIODevice::ReadOnly))
        return history;

    QJsonParseError error;

    QJsonDocument document =
        QJsonDocument::fromJson(
            file.readAll(),
            &error
        );

    if (
        error.error !=
        QJsonParseError::NoError
    )
    {
        return history;
    }

    if (!document.isArray())
        return history;

    for (const QJsonValue &value : document.array())
    {
        if (!value.isObject())
            continue;

        QJsonObject object = value.toObject();

        QString url =
            object["url"].toString();

        if (url.trimmed().isEmpty())
            continue;

        HistoryEntry entry;
        entry.title = object["title"].toString();
        entry.url = url;

        history.append(entry);
    }

    return history;
}

static bool saveHistory(
    const QList<HistoryEntry> &history
)
{
    QJsonArray array;

    for (const HistoryEntry &entry : history)
    {
        QJsonObject object;
        object["title"] = entry.title;
        object["url"] = entry.url;
        array.append(object);
    }

    QFile file(historyPath());

    if (!file.open(
        QIODevice::WriteOnly |
        QIODevice::Truncate
    ))
    {
        return false;
    }

    file.write(
        QJsonDocument(array)
            .toJson(QJsonDocument::Indented)
    );

    return true;
}

static BrowserConfig loadConfig(const QString &path)
{
    BrowserConfig config = defaultConfig();

    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
        return config;

    QJsonParseError error;

    QJsonDocument document =
        QJsonDocument::fromJson(
            file.readAll(),
            &error
        );

    if (
        error.error !=
        QJsonParseError::NoError ||
        !document.isObject()
    )
    {
        return config;
    }

    QJsonObject object = document.object();

    if (object.contains("window_border"))
        config.windowBorder =
            object["window_border"]
                .toString(config.windowBorder)
                .toLower();

    if (object.contains("theme"))
        config.theme =
            object["theme"]
                .toString(config.theme)
                .toLower();

    if (object.contains("browser_colour"))
        config.browserColour =
            object["browser_colour"]
                .toString(config.browserColour);

    if (object.contains("border_colour"))
        config.borderColour =
            object["border_colour"]
                .toString(config.borderColour);

    if (object.contains("background_colour"))
        config.backgroundColour =
            object["background_colour"].toString();

    if (object.contains("text_colour"))
        config.textColour =
            object["text_colour"].toString();

    if (object.contains("homepage"))
        config.homepage =
            object["homepage"]
                .toString(config.homepage);

    if (object.contains("search_engine"))
        config.searchEngine =
            object["search_engine"]
                .toString(config.searchEngine)
                .toLower();

    if (object.contains("zoom"))
        config.zoom =
            object["zoom"].toInt(config.zoom);

    if (
        config.windowBorder != "default" &&
        config.windowBorder != "borderless"
    )
    {
        config.windowBorder = "default";
    }

    if (
        config.theme != "light" &&
        config.theme != "dark"
    )
    {
        config.theme = "light";
    }

    if (!QColor(config.browserColour).isValid())
        config.browserColour = "#9b59b6";

    if (!QColor(config.borderColour).isValid())
        config.borderColour = "#9b59b6";

    if (
        !config.backgroundColour.isEmpty() &&
        !QColor(config.backgroundColour).isValid()
    )
    {
        config.backgroundColour.clear();
    }

    if (
        !config.textColour.isEmpty() &&
        !QColor(config.textColour).isValid()
    )
    {
        config.textColour.clear();
    }

    if (config.zoom < 25)
        config.zoom = 25;

    if (config.zoom > 500)
        config.zoom = 500;

    return config;
}

static bool saveConfig(
    const BrowserConfig &config,
    const QString &path
)
{
    if (path.trimmed().isEmpty())
        return false;

    QDir().mkpath(
        QFileInfo(path).absolutePath()
    );

    QJsonObject object;

    object["window_border"] = config.windowBorder;
    object["theme"] = config.theme;
    object["browser_colour"] = config.browserColour;
    object["border_colour"] = config.borderColour;
    object["background_colour"] = config.backgroundColour;
    object["text_colour"] = config.textColour;
    object["homepage"] = config.homepage;
    object["search_engine"] = config.searchEngine;
    object["zoom"] = config.zoom;

    QFile file(path);

    if (!file.open(
        QIODevice::WriteOnly |
        QIODevice::Truncate
    ))
    {
        return false;
    }

    file.write(
        QJsonDocument(object)
            .toJson(QJsonDocument::Indented)
    );

    return true;
}

static QString searchUrl(
    const QString &engine,
    const QString &query
)
{
    QString encoded =
        QString::fromUtf8(
            QUrl::toPercentEncoding(query)
        );

    if (engine == "google")
        return "https://www.google.com/search?q=" + encoded;

    if (engine == "bing")
        return "https://www.bing.com/search?q=" + encoded;

    if (engine == "yahoo")
        return "https://search.yahoo.com/search?p=" + encoded;

    return "https://duckduckgo.com/?q=" + encoded;
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

        QWebEngineSettings *settings = this->settings();

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
          config_(config),
          history_(loadHistory())
    {
        setWindowTitle("LewWeb");
        resize(1280, 800);

        buildInterface();
        applyConfig();
        setupShortcuts();
        setupDownloads();

        addTab(
            QUrl(config_.homepage)
        );
    }

private:
    void buildInterface()
    {
        tabs_ = new QTabWidget(this);

        tabs_->setDocumentMode(true);
        tabs_->setTabsClosable(true);
        tabs_->setMovable(true);
        tabs_->tabBar()->setExpanding(false);

        connect(
            tabs_,
            &QTabWidget::currentChanged,
            this,
            [this](int)
            {
                updateUrlBar();
                updateButtons();
                updateWindowTitle();
            }
        );

        connect(
            tabs_,
            &QTabWidget::tabCloseRequested,
            this,
            [this](int index)
            {
                closeTab(index);
            }
        );

        QWidget *toolbar = new QWidget(this);
        toolbar->setObjectName("toolbar");

        QHBoxLayout *layout =
            new QHBoxLayout(toolbar);

        layout->setContentsMargins(
            8, 7, 8, 7
        );

        layout->setSpacing(5);

        backButton_ = makeButton(
            QApplication::style()->standardIcon(
                QStyle::SP_ArrowBack
            ),
            "Back"
        );

        forwardButton_ = makeButton(
            QApplication::style()->standardIcon(
                QStyle::SP_ArrowForward
            ),
            "Forward"
        );

        reloadButton_ = makeButton(
            QApplication::style()->standardIcon(
                QStyle::SP_BrowserReload
            ),
            "Reload"
        );

        homeButton_ = makeButton(
            QApplication::style()->standardIcon(
                QStyle::SP_DirHomeIcon
            ),
            "Home"
        );

        backButton_->setObjectName("navButton");
        forwardButton_->setObjectName("navButton");
        reloadButton_->setObjectName("navButton");
        homeButton_->setObjectName("navButton");

        connect(
            backButton_,
            &QToolButton::clicked,
            this,
            [this] { back(); }
        );

        connect(
            forwardButton_,
            &QToolButton::clicked,
            this,
            [this] { forward(); }
        );

        connect(
            reloadButton_,
            &QToolButton::clicked,
            this,
            [this]
            {
                if (auto *view = currentTab())
                {
                    if (view->page()->isLoading())
                        view->stop();
                    else
                        view->reload();
                }
            }
        );

        connect(
            homeButton_,
            &QToolButton::clicked,
            this,
            [this]
            {
                openUrl(
                    QUrl(config_.homepage)
                );
            }
        );

        urlBar_ = new QLineEdit(this);
        urlBar_->setObjectName("urlBar");
        urlBar_->setPlaceholderText(
            "Search or enter website address"
        );
        urlBar_->setClearButtonEnabled(true);

        connect(
            urlBar_,
            &QLineEdit::returnPressed,
            this,
            [this]
            {
                navigateFromUrlBar();
            }
        );

        starButton_ = makeButton(
            QApplication::style()->standardIcon(
                QStyle::SP_DialogSaveButton
            ),
            "Bookmark this page"
        );

        starButton_->setObjectName("navButton");

        connect(
            starButton_,
            &QToolButton::clicked,
            this,
            [this]
            {
                saveCurrentBookmark();
            }
        );

        newTabButton_ = makeButton(
            QApplication::style()->standardIcon(
                QStyle::SP_FileDialogNewFolder
            ),
            "New Tab"
        );

        newTabButton_->setObjectName("navButton");

        connect(
            newTabButton_,
            &QToolButton::clicked,
            this,
            [this]
            {
                addTab(
                    QUrl(config_.homepage)
                );
            }
        );

        menuButton_ = makeButton(
            QApplication::style()->standardIcon(
                QStyle::SP_TitleBarMenuButton
            ),
            "LewWeb menu"
        );

        menuButton_->setObjectName("navButton");

        connect(
            menuButton_,
            &QToolButton::clicked,
            this,
            [this]
            {
                showMenu();
            }
        );

        layout->addWidget(backButton_);
        layout->addWidget(forwardButton_);
        layout->addWidget(reloadButton_);
        layout->addWidget(homeButton_);
        layout->addWidget(urlBar_, 1);
        layout->addWidget(starButton_);
        layout->addWidget(newTabButton_);
        layout->addWidget(menuButton_);

        progressBar_ = new QProgressBar(this);
        progressBar_->setRange(0, 100);
        progressBar_->setValue(0);
        progressBar_->setTextVisible(false);
        progressBar_->setFixedHeight(3);
        progressBar_->hide();

        QWidget *content = new QWidget(this);

        QVBoxLayout *contentLayout =
            new QVBoxLayout(content);

        contentLayout->setContentsMargins(0, 0, 0, 0);
        contentLayout->setSpacing(0);

        contentLayout->addWidget(toolbar);
        contentLayout->addWidget(progressBar_);
        contentLayout->addWidget(tabs_, 1);

        setCentralWidget(content);
    }

    QToolButton *makeButton(
        const QIcon &icon,
        const QString &toolTip
    )
    {
        QToolButton *button =
            new QToolButton(this);

        button->setIcon(icon);
        button->setToolTip(toolTip);
        button->setAutoRaise(true);
        button->setIconSize(QSize(18, 18));

        return button;
    }

    void setupShortcuts()
    {
        shortcut(
            "Ctrl+T",
            [this]
            {
                addTab(QUrl(config_.homepage));
            }
        );

        shortcut(
            "Meta+T",
            [this]
            {
                addTab(QUrl(config_.homepage));
            }
        );

        shortcut(
            "Ctrl+W",
            [this]
            {
                closeCurrentTab();
            }
        );

        shortcut(
            "Meta+W",
            [this]
            {
                closeCurrentTab();
            }
        );

        shortcut(
            "Ctrl+L",
            [this]
            {
                focusUrlBar();
            }
        );

        shortcut(
            "Meta+L",
            [this]
            {
                focusUrlBar();
            }
        );

        shortcut(
            "Ctrl+R",
            [this]
            {
                reload();
            }
        );

        shortcut(
            "Meta+R",
            [this]
            {
                reload();
            }
        );

        shortcut(
            "Alt+Left",
            [this]
            {
                back();
            }
        );

        shortcut(
            "Meta+Left",
            [this]
            {
                back();
            }
        );

        shortcut(
            "Alt+Right",
            [this]
            {
                forward();
            }
        );

        shortcut(
            "Meta+Right",
            [this]
            {
                forward();
            }
        );

        shortcut(
            "Ctrl+Tab",
            [this]
            {
                nextTab();
            }
        );

        shortcut(
            "Ctrl+Shift+Tab",
            [this]
            {
                previousTab();
            }
        );

        shortcut(
            "Meta+1",
            [this] { selectTab(0); }
        );

        shortcut(
            "Meta+2",
            [this] { selectTab(1); }
        );

        shortcut(
            "Meta+3",
            [this] { selectTab(2); }
        );

        shortcut(
            "Meta+4",
            [this] { selectTab(3); }
        );

        shortcut(
            "Meta+5",
            [this] { selectTab(4); }
        );

        shortcut(
            "Meta+6",
            [this] { selectTab(5); }
        );

        shortcut(
            "Meta+7",
            [this] { selectTab(6); }
        );

        shortcut(
            "Meta+8",
            [this] { selectTab(7); }
        );

        shortcut(
            "Meta+9",
            [this]
            {
                selectTab(
                    tabs_->count() - 1
                );
            }
        );

        shortcut(
            "F11",
            [this]
            {
                toggleFullscreen();
            }
        );

        shortcut(
            "Ctrl+=",
            [this] { zoomIn(); }
        );

        shortcut(
            "Meta+=",
            [this] { zoomIn(); }
        );

        shortcut(
            "Ctrl+-",
            [this] { zoomOut(); }
        );

        shortcut(
            "Meta+-",
            [this] { zoomOut(); }
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
                    download->downloadDirectory();

                if (directory.isEmpty())
                    directory =
                        profile_->downloadPath();

                QString fileName =
                    download->downloadFileName();

                if (fileName.isEmpty())
                    fileName =
                        download->suggestedFileName();

                if (fileName.isEmpty())
                    fileName = "download";

                QDir dir(directory);

                if (!dir.exists())
                    dir.mkpath(".");

                download->setDownloadDirectory(
                    directory
                );

                download->setDownloadFileName(
                    fileName
                );

                download->accept();
            }
        );
    }

    BrowserTab *addTab(const QUrl &url)
    {
        BrowserTab *view =
            new BrowserTab(
                profile_,
                [this]() -> QWebEnginePage *
                {
                    BrowserTab *tab =
                        addTab(QUrl("about:blank"));

                    return tab->page();
                },
                config_.zoom
            );

        int index =
            tabs_->addTab(
                view,
                "New Tab"
            );

        tabs_->setCurrentIndex(index);

        connect(
            view,
            &QWebEngineView::titleChanged,
            this,
            [this, view](const QString &title)
            {
                QString tabTitle =
                    title.trimmed();

                if (tabTitle.isEmpty())
                    tabTitle = "New Tab";

                int index =
                    tabs_->indexOf(view);

                if (index >= 0)
                {
                    tabs_->setTabText(
                        index,
                        tabTitle.left(32)
                    );
                }

                updateWindowTitle();
            }
        );

        connect(
            view,
            &QWebEngineView::urlChanged,
            this,
            [this, view](const QUrl &)
            {
                if (view == currentTab())
                    updateUrlBar();

                updateButtons();
            }
        );

        connect(
            view,
            &QWebEngineView::iconChanged,
            this,
            [this, view](const QIcon &icon)
            {
                int index =
                    tabs_->indexOf(view);

                if (
                    index >= 0 &&
                    !icon.isNull()
                )
                {
                    tabs_->setTabIcon(
                        index,
                        icon
                    );
                }
            }
        );

        connect(
            view,
            &QWebEngineView::loadProgress,
            this,
            [this, view](int progress)
            {
                if (view != currentTab())
                    return;

                if (progress >= 100)
                {
                    progressBar_->hide();
                    reloadButton_->setIcon(
                        QApplication::style()->standardIcon(
                            QStyle::SP_BrowserReload
                        )
                    );
                }
                else
                {
                    progressBar_->setValue(progress);
                    progressBar_->show();
                    reloadButton_->setIcon(
                        QApplication::style()->standardIcon(
                            QStyle::SP_BrowserStop
                        )
                    );
                }
            }
        );

        connect(
            view,
            &QWebEngineView::loadFinished,
            this,
            [this, view](bool ok)
            {
                progressBar_->hide();

                reloadButton_->setIcon(
                    QApplication::style()->standardIcon(
                        QStyle::SP_BrowserReload
                    )
                );

                if (!ok)
                    return;

                QString url =
                    view->url().toString();

                if (!url.isEmpty())
                    recordHistory(
                        view->title(),
                        url
                    );

                if (
                    !siteTheme_.isEmpty() &&
                    siteTheme_ != "system"
                )
                {
                    applySiteTheme(
                        view,
                        siteTheme_
                    );
                }
            }
        );

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

        view->load(url);
        return view;
    }

    void closeTab(int index)
    {
        if (
            index < 0 ||
            index >= tabs_->count()
        )
        {
            return;
        }

        if (tabs_->count() == 1)
        {
            addTab(QUrl(config_.homepage));
        }

        QWidget *widget =
            tabs_->widget(index);

        tabs_->removeTab(index);
        widget->deleteLater();

        updateUrlBar();
        updateButtons();
    }

    void closeCurrentTab()
    {
        closeTab(
            tabs_->currentIndex()
        );
    }

    BrowserTab *currentTab() const
    {
        return static_cast<BrowserTab *>(
            tabs_->currentWidget()
        );
    }

    void openUrl(const QUrl &url)
    {
        if (auto *view = currentTab())
            view->load(url);
    }

    void navigateFromUrlBar()
    {
        QString input =
            urlBar_->text().trimmed();

        if (input.isEmpty())
            return;

        QUrl url = QUrl::fromUserInput(input);

        bool looksLikeUrl =
            input.contains("://") ||
            input.startsWith("localhost") ||
            input.startsWith("127.0.0.1") ||
            input.contains('.');

        if (
            !looksLikeUrl ||
            !url.isValid() ||
            url.scheme().isEmpty()
        )
        {
            url =
                QUrl(
                    searchUrl(
                        config_.searchEngine,
                        input
                    )
                );
        }

        openUrl(url);
        currentTabFocus();
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
        int count = tabs_->count();

        if (count < 2)
            return;

        int index =
            (tabs_->currentIndex() + 1) %
            count;

        tabs_->setCurrentIndex(index);
        currentTabFocus();
    }

    void previousTab()
    {
        int count = tabs_->count();

        if (count < 2)
            return;

        int index =
            tabs_->currentIndex() - 1;

        if (index < 0)
            index = count - 1;

        tabs_->setCurrentIndex(index);
        currentTabFocus();
    }

    void selectTab(int index)
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

    void currentTabFocus()
    {
        if (auto *view = currentTab())
            view->setFocus();
    }

    void focusUrlBar()
    {
        urlBar_->setFocus();
        urlBar_->selectAll();
    }

    void updateUrlBar()
    {
        if (!urlBar_)
            return;

        if (auto *view = currentTab())
        {
            QSignalBlocker blocker(urlBar_);

            urlBar_->setText(
                view->url().toString()
            );

            urlBar_->setCursorPosition(0);
        }
        else
        {
            urlBar_->clear();
        }
    }

    void updateButtons()
    {
        auto *view = currentTab();

        bool valid = view != nullptr;

        backButton_->setEnabled(
            valid && view->history()->canGoBack()
        );

        forwardButton_->setEnabled(
            valid && view->history()->canGoForward()
        );

        reloadButton_->setEnabled(valid);
        homeButton_->setEnabled(valid);
        starButton_->setEnabled(valid);
    }

    void updateWindowTitle()
    {
        auto *view = currentTab();

        if (!view)
        {
            setWindowTitle("LewWeb");
            return;
        }

        QString title =
            view->title().trimmed();

        if (title.isEmpty())
            title = "LewWeb";

        setWindowTitle(
            title + " — LewWeb"
        );
    }

    void recordHistory(
        const QString &title,
        const QString &url
    )
    {
        QString cleanUrl =
            url.trimmed();

        if (
            cleanUrl.isEmpty() ||
            cleanUrl.startsWith(
                "about:",
                Qt::CaseInsensitive
            )
        )
        {
            return;
        }

        for (int i = 0; i < history_.size(); ++i)
        {
            if (history_.at(i).url == cleanUrl)
            {
                history_.removeAt(i);
                break;
            }
        }

        HistoryEntry entry;

        entry.title =
            title.trimmed();

        if (entry.title.isEmpty())
            entry.title = QUrl(cleanUrl).host();

        if (entry.title.isEmpty())
            entry.title = "New Page";

        entry.url = cleanUrl;

        history_.prepend(entry);

        while (history_.size() > 100)
            history_.removeLast();

        saveHistory(history_);
    }

    void saveCurrentBookmark()
    {
        auto *view = currentTab();

        if (!view)
            return;

        QString url =
            view->url().toString();

        if (url.isEmpty())
            return;

        QString title =
            view->title().trimmed();

        if (title.isEmpty())
            title = view->url().host();

        if (title.isEmpty())
            title = "Bookmark";

        if (!addBookmark(title, url))
        {
            QMessageBox::warning(
                this,
                "LewWeb",
                "Could not save bookmark."
            );
        }
        else
        {
            starButton_->setToolTip(
                "Bookmark saved"
            );
        }
    }

    void showMenu()
    {
        QMenu menu(this);

        QAction *newTab =
            menu.addAction("New Tab");

        QAction *home =
            menu.addAction("Home");

        QAction *history =
            menu.addAction("History");

        QAction *bookmarks =
            menu.addAction("Bookmarks");

        menu.addSeparator();

        QMenu *zoomMenu =
            menu.addMenu("Zoom");

        QAction *zoomInAction =
            zoomMenu->addAction("Zoom In");

        QAction *zoomOutAction =
            zoomMenu->addAction("Zoom Out");

        QAction *resetZoomAction =
            zoomMenu->addAction("Reset Zoom");

        QMenu *themeMenu =
            menu.addMenu("Web Theme");

        QAction *lightAction =
            themeMenu->addAction("Light");

        QAction *darkAction =
            themeMenu->addAction("Dark");

        menu.addSeparator();

        QAction *fullscreen =
            menu.addAction("Fullscreen");

        QAction *settings =
            menu.addAction("Browser Settings");

        QAction *quit =
            menu.addAction("Quit");

        QAction *chosen =
            menu.exec(
                menuButton_->mapToGlobal(
                    QPoint(
                        0,
                        menuButton_->height()
                    )
                )
            );

        if (chosen == newTab)
        {
            addTab(QUrl(config_.homepage));
        }
        else if (chosen == home)
        {
            openUrl(QUrl(config_.homepage));
        }
        else if (chosen == history)
        {
            showHistory();
        }
        else if (chosen == bookmarks)
        {
            showBookmarks();
        }
        else if (chosen == zoomInAction)
        {
            zoomIn();
        }
        else if (chosen == zoomOutAction)
        {
            zoomOut();
        }
        else if (chosen == resetZoomAction)
        {
            setZoom(config_.zoom);
        }
        else if (chosen == lightAction)
        {
            config_.theme = "light";
            siteTheme_ = "light";
            saveConfig(config_, configPath());
            applyConfig();
            applySiteThemeToCurrent();
        }
        else if (chosen == darkAction)
        {
            config_.theme = "dark";
            siteTheme_ = "dark";
            saveConfig(config_, configPath());
            applyConfig();
            applySiteThemeToCurrent();
        }
        else if (chosen == fullscreen)
        {
            toggleFullscreen();
        }
        else if (chosen == settings)
        {
            showSettings();
        }
        else if (chosen == quit)
        {
            close();
        }
    }

    void showHistory()
    {
        QMenu menu(this);

        int count = 0;

        for (const HistoryEntry &entry : history_)
        {
            if (count >= 30)
                break;

            QString label =
                entry.title.isEmpty()
                    ? entry.url
                    : entry.title;

            QAction *action =
                menu.addAction(label.left(80));

            action->setToolTip(entry.url);
            action->setData(entry.url);

            ++count;
        }

        if (count == 0)
        {
            menu.addAction("No history.")
                ->setEnabled(false);
        }

        QAction *clear =
            menu.addAction("Clear History");

        QAction *chosen =
            menu.exec(
                menuButton_->mapToGlobal(
                    QPoint(
                        0,
                        menuButton_->height()
                    )
                )
            );

        if (chosen == clear)
        {
            history_.clear();
            saveHistory(history_);
            return;
        }

        if (
            chosen &&
            chosen->data().isValid()
        )
        {
            openUrl(
                QUrl(
                    chosen->data().toString()
                )
            );
        }
    }

    void showBookmarks()
    {
        QMenu menu(this);

        QJsonObject bookmarks =
            loadBookmarks();

        QList<QAction *> bookmarkActions;

        for (
            auto it = bookmarks.begin();
            it != bookmarks.end();
            ++it
        )
        {
            QAction *action =
                menu.addAction(it.key());

            action->setData(
                it.value().toString()
            );

            bookmarkActions.append(action);
        }

        if (bookmarkActions.isEmpty())
        {
            menu.addAction(
                "No bookmarks."
            )->setEnabled(false);
        }

        menu.addSeparator();

        QAction *remove =
            menu.addAction(
                "Remove Current Page Bookmark"
            );

        QAction *chosen =
            menu.exec(
                menuButton_->mapToGlobal(
                    QPoint(
                        0,
                        menuButton_->height()
                    )
                )
            );

        if (chosen == remove)
        {
            auto *view = currentTab();

            if (view)
            {
                QString title =
                    view->title().trimmed();

                if (!title.isEmpty())
                    removeBookmark(title);
            }

            return;
        }

        if (
            chosen &&
            chosen->data().isValid()
        )
        {
            openUrl(
                QUrl(
                    chosen->data().toString()
                )
            );
        }
    }

    void showSettings()
    {
        QMessageBox box(this);

        box.setWindowTitle("LewWeb Browser Settings");

        box.setText(
            QString(
                "LewWeb %1\n\n"
                "Homepage: %2\n"
                "Search engine: %3\n"
                "Default zoom: %4%\n\n"
                "Persistent browser profile:\n%5"
            )
            .arg(
                LEWWEB_VERSION,
                config_.homepage,
                config_.searchEngine,
                QString::number(config_.zoom),
                profilePath()
            )
        );

        box.setInformativeText(
            "Your cookies, local storage, cache, and other "
            "Chromium profile data are kept in the persistent "
            "LewWeb profile so supported website logins can "
            "survive browser restarts."
        );

        box.exec();
    }

    void zoomIn()
    {
        if (auto *view = currentTab())
        {
            view->setZoomFactor(
                qMin(
                    5.0,
                    view->zoomFactor() + 0.1
                )
            );
        }
    }

    void zoomOut()
    {
        if (auto *view = currentTab())
        {
            view->setZoomFactor(
                qMax(
                    0.25,
                    view->zoomFactor() - 0.1
                )
            );
        }
    }

    void setZoom(int percentage)
    {
        if (auto *view = currentTab())
        {
            view->setZoomFactor(
                qBound(
                    0.25,
                    percentage / 100.0,
                    5.0
                )
            );
        }
    }

    void toggleFullscreen()
    {
        if (isFullScreen())
            showNormal();
        else
            showFullScreen();
    }

    void applySiteThemeToCurrent()
    {
        if (auto *view = currentTab())
            applySiteTheme(
                view,
                siteTheme_
            );
    }

    void applySiteTheme(
        BrowserTab *view,
        const QString &theme
    )
    {
        if (!view)
            return;

        QString script =
            QString(
                R"JS(
                    (() => {
                        const root =
                            document.documentElement;

                        root.style.setProperty(
                            'color-scheme',
                            '%1',
                            'important'
                        );

                        let meta =
                            document.querySelector(
                                'meta[name="color-scheme"]'
                            );

                        if (!meta) {
                            meta =
                                document.createElement(
                                    'meta'
                                );

                            meta.name =
                                'color-scheme';

                            document.head.appendChild(
                                meta
                            );
                        }

                        meta.content = '%1';

                        root.setAttribute(
                            'data-lewweb-theme',
                            '%1'
                        );
                    })();
                )JS"
            )
            .arg(theme);

        view->page()->runJavaScript(script);
    }

    void applyConfig()
    {
        if (
            config_.windowBorder ==
            "borderless"
        )
        {
            setWindowFlags(
                Qt::FramelessWindowHint |
                Qt::Window
            );
        }
        else
        {
            setWindowFlags(Qt::Window);
        }

        QString browserColour =
            QColor(config_.browserColour)
                .name();

        QString backgroundColour =
            config_.backgroundColour;

        QString textColour =
            config_.textColour;

        if (backgroundColour.isEmpty())
        {
            backgroundColour =
                config_.theme == "dark"
                    ? "#181818"
                    : "#ffffff";
        }

        if (textColour.isEmpty())
        {
            textColour =
                config_.theme == "dark"
                    ? "#ffffff"
                    : "#111111";
        }

        QString toolbarColour =
            config_.theme == "dark"
                ? "#000000"
                : "#ffffff";

        QString toolbarTop =
            config_.theme == "dark"
                ? "#2a2a2a"
                : "#ffffff";

        QString toolbarBottom =
            config_.theme == "dark"
                ? "#000000"
                : "#d9d9d9";

        QString buttonTop =
            config_.theme == "dark"
                ? "#3a3a3a"
                : "#ffffff";

        QString buttonBottom =
            config_.theme == "dark"
                ? "#111111"
                : "#cfcfcf";

        QString buttonBorder =
            config_.theme == "dark"
                ? "#555555"
                : "#8a8a8a";

        QString chromeText =
            config_.theme == "dark"
                ? "#ffffff"
                : "#111111";

        setStyleSheet(
            QString(
                "QMainWindow {"
                "background: %1;"
                "}"
                "QWidget#toolbar {"
                "background: qlineargradient("
                "x1:0, y1:0, x2:0, y2:1,"
                "stop:0 %2, stop:0.48 %3, stop:1 %4);"
                "border-top: 1px solid %5;"
                "border-bottom: 1px solid %6;"
                "}"
                "QToolButton#navButton {"
                "color: %7;"
                "background: qlineargradient("
                "x1:0, y1:0, x2:0, y2:1,"
                "stop:0 %8, stop:0.48 %8, stop:1 %9);"
                "border: 1px solid %10;"
                "border-radius: 5px;"
                "padding: 5px 7px;"
                "}"
                "QToolButton#navButton:hover {"
                "background: qlineargradient("
                "x1:0, y1:0, x2:0, y2:1,"
                "stop:0 %2, stop:0.48 %3, stop:1 %4);"
                "}"
                "QToolButton#navButton:pressed {"
                "padding-top: 6px;"
                "padding-left: 8px;"
                "background: %9;"
                "}"
                "QToolButton#navButton:disabled {"
                "color: #888888;"
                "}"
                "QLineEdit#urlBar {"
                "background: qlineargradient("
                "x1:0, y1:0, x2:0, y2:1,"
                "stop:0 %2, stop:0.45 %2, stop:1 %3);"
                "color: %7;"
                "border: 1px solid %10;"
                "border-radius: 16px;"
                "padding: 7px 14px;"
                "selection-background-color: %11;"
                "}"
                "QLineEdit#urlBar:focus {"
                "border: 1px solid %11;"
                "}"
                "QTabBar::tab {"
                "background: %3;"
                "color: %7;"
                "padding: 8px 14px;"
                "min-width: 100px;"
                "max-width: 220px;"
                "border: 1px solid %10;"
                "border-bottom: none;"
                "}"
                "QTabBar::tab:selected {"
                "background: %11;"
                "color: white;"
                "}"
                "QProgressBar {"
                "background: transparent;"
                "border: none;"
                "}"
                "QProgressBar::chunk {"
                "background: %11;"
                "}"
            )
            .arg(
                backgroundColour,
                toolbarTop,
                toolbarBottom,
                toolbarColour,
                buttonBorder,
                buttonBorder,
                chromeText,
                buttonTop,
                buttonBottom,
                buttonBorder,
                browserColour
            )
        );

        showNormal();
        updateUrlBar();
        updateButtons();
        updateWindowTitle();
    }

protected:
    void closeEvent(QCloseEvent *event) override
    {
        saveConfig(config_, configPath());
        QMainWindow::closeEvent(event);
    }

private:
    QTabWidget *tabs_ = nullptr;
    QWebEngineProfile *profile_ = nullptr;

    QToolButton *backButton_ = nullptr;
    QToolButton *forwardButton_ = nullptr;
    QToolButton *reloadButton_ = nullptr;
    QToolButton *homeButton_ = nullptr;
    QToolButton *starButton_ = nullptr;
    QToolButton *newTabButton_ = nullptr;
    QToolButton *menuButton_ = nullptr;

    QLineEdit *urlBar_ = nullptr;
    QProgressBar *progressBar_ = nullptr;

    BrowserConfig config_;
    QList<HistoryEntry> history_;

    QString siteTheme_ = "light";
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName(
        "LewWeb"
    );

    QCoreApplication::setApplicationVersion(
        LEWWEB_VERSION
    );

    BrowserConfig config =
        loadConfig(configPath());

    QWebEngineProfile *profile =
        new QWebEngineProfile(
            "LewWeb",
            &app
        );

    QString path =
        profilePath();

    /*
     * Keep one persistent Chromium profile.
     *
     * This is important for login compatibility:
     * cookies, local storage, IndexedDB, service-worker
     * data and other persistent website state stay here.
     */
    profile->setPersistentStoragePath(path);
    profile->setCachePath(
        QDir(path).filePath("cache")
    );

    profile->setPersistentCookiesPolicy(
        QWebEngineProfile::ForcePersistentCookies
    );

    profile->setHttpAcceptLanguage(
        "en-GB,en;q=0.9"
    );

    BrowserWindow window(
        profile,
        config
    );

    window.show();

    return app.exec();
}
