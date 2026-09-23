# LewWeb Browser

**A cross-platform graphical web browser built with C++ and QtWebEngine.**

LewWeb is a conventional desktop web browser focused on a clean graphical interface, persistent browsing, tabbed browsing, and Chromium-based web compatibility.

## Features

* **C++**
* **Qt 6**
* **QtWebEngine**
* **Chromium-based web rendering**
* Graphical browser interface
* Tabbed browsing
* Persistent browser profile
* Persistent cookies, cache, and web storage
* Address and search bar
* Back, forward, reload, stop, and home controls
* Bookmarks
* Browsing history
* Downloads
* Light and dark browser themes
* Configurable browser and border colours
* Configurable homepage
* Configurable search engine
* Configurable zoom level
* Browser fullscreen support
* Web/video fullscreen support
* New-window and new-tab handling
* Keyboard shortcuts
* Favicons and page titles
* JavaScript and local-storage support

## The Interface

LewWeb is designed to behave like a conventional graphical web browser.

The main browser window provides:

* A tab bar for managing open pages
* An address/search bar
* Navigation controls
* Browser actions such as home, bookmarks, downloads, and new tabs
* A Chromium-based web view for displaying websites

LewWeb also supports keyboard shortcuts for common browser actions while keeping the primary browsing experience inside the graphical window.

## Configuration

LewWeb uses a JSON configuration file stored with the persistent browser profile.

Configuration includes settings such as:

* Window border style
* Light/dark theme
* Browser colour
* Border colour
* Background colour
* Text colour
* Homepage
* Search engine
* Zoom level

The configuration is saved so browser appearance and preferences persist between launches.

## Building

LewWeb requires:

* C++17 or newer
* Qt 6
* QtWebEngine

On macOS, LewWeb can be built against the Qt 6 frameworks and QtWebEngine libraries.

The project can be compiled with a compatible C++ compiler. LewWeb is also developed within the author's own compiler/toolchain projects.

## Project Status

**Version: 1.0.0**

LewWeb has evolved from an experimental browser concept into a conventional graphical desktop browser.

The current version focuses on providing a usable browser experience with tabs, persistent browsing data, navigation, bookmarks, downloads, themes, and Chromium-based web compatibility.

## Technology

LewWeb is built with:

**C++ → Qt 6 → QtWebEngine → Chromium**

Qt provides the application interface and browser integration, while QtWebEngine provides the underlying Chromium-based web rendering engine.

## Philosophy

LewWeb is built around a simple idea:

> **A browser should feel like a browser.**

The project began as an experiment with alternative browser interfaces and has evolved into a full graphical browser while keeping the project lightweight and under the author's control.

---

# Third-Party Software & Legal Notices

LewWeb is an independent project and is **not affiliated with, endorsed by, or sponsored by** The Qt Company, Google, or the Chromium Project.

LewWeb makes use of third-party software and libraries. Their respective copyrights and licenses remain with their original authors and copyright holders.

## Qt 6

LewWeb uses **Qt 6**, including Qt modules used for the application interface and browser integration.

Qt is developed and maintained by **The Qt Company Ltd.** and contributors.

Qt is available under various licensing options, including the **GNU Lesser General Public License version 3 (LGPLv3)** and, for applicable modules, the **GNU General Public License (GPL)**, as well as commercial licensing. The applicable license depends on the Qt components being used and how they are distributed.

For licensing information:

[Qt Licensing](https://www.qt.io/licensing/)

[Qt 6 Licensing Documentation](https://doc.qt.io/qt-6/licensing.html)

## Qt WebEngine

LewWeb uses **Qt WebEngine** for its web rendering functionality.

Qt WebEngine incorporates **Chromium**, meaning distributions of Qt WebEngine are subject to the applicable licenses and notices for both the Qt WebEngine components and the Chromium components contained within it.

## Chromium

LewWeb uses Chromium through Qt WebEngine.

**Chromium is a project of The Chromium Authors and Google.**

Chromium contains software distributed under multiple open-source licenses, including BSD, Apache, LGPL, and other applicable licenses depending on the individual component.

The Chromium project maintains its own third-party licensing and attribution information.

LewWeb does not claim ownership of Chromium or any of its third-party components.

## Third-Party Components

Qt and Qt WebEngine may contain additional third-party software distributed under their own respective licenses.

Only the components actually included in a particular distribution are subject to the corresponding attribution and licensing requirements.

Users redistributing LewWeb should ensure that they comply with all applicable licenses and notices for the versions of Qt, Qt WebEngine, Chromium, and other third-party components included with their distribution.

## Trademarks

**Qt** and the Qt logo are trademarks of The Qt Company Ltd. in Finland and/or other countries.

**Chromium** and related marks are associated with the Chromium project and/or Google.

**Google**, **Google Chrome**, **YouTube**, **DuckDuckGo**, and other product or service names referenced by LewWeb are trademarks of their respective owners.

LewWeb does not claim ownership of these trademarks.

## Disclaimer

LewWeb is provided on an **"as is"** basis, without warranties of any kind.

The author of LewWeb is not responsible for the content, availability, security, privacy practices, or functionality of websites accessed through the browser.

Third-party software included or used by LewWeb remains subject to its own respective licenses and terms.

---

Copyright © 2026 **xlewis1**

LewWeb is licensed under the Apache License 2.0.

**LewWeb 1.0.0**

Copyright © 2026 **xlewis1**
