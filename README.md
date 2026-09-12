# LewWeb Browser

**A keyboard-driven, CLI-controlled web browser built with C++ and QtWebEngine.**

LewWeb is an experimental web browser built around a simple idea: **the browser itself doesn't need to be covered in buttons.**

Instead of a traditional browser interface with visible tabs, an address bar, search bar, and toolbar, LewWeb puts its controls in the terminal and lets the browser window focus on the web.

## Features

* **C++**
* **Qt 6**
* **QtWebEngine**
* **Chromium-based web rendering**
* CLI-controlled browsing
* Keyboard-driven interface
* Invisible browser tabs
* Built-in command palette
* DuckDuckGo as the default search engine
* Configurable browser appearance
* Light and dark themes
* Configurable browser and border colours
* Browser fullscreen support
* YouTube/video fullscreen support
* New-window and new-tab handling
* Command completion through the command palette

## The Interface

LewWeb deliberately doesn't look like a conventional web browser.

There is no permanent:

* Tab bar
* Address bar
* Search bar
* Toolbar full of buttons

Tabs still exist internally, but the tab bar is hidden. Browser control is handled through LewWeb's CLI and keyboard interface.

Press **`Ctrl+T`** to open the LewWeb command palette.

The palette uses the prompt:

`lewweb:>$`

Commands appear as you type, with completion available for supported commands.

On macOS, **`Cmd+T`** opens a new browser tab.

## Why?

LewWeb started as an experiment in what a web browser could look like when the traditional browser chrome is removed.

The goal isn't to recreate another existing browser or build a Vim browser.

The idea is to make the **terminal and browser work together as one interface**.

The terminal controls the browser.

The browser displays the web.

## Configuration

LewWeb uses a JSON configuration file.

Configuration can control things such as:

* Window border style
* Light/dark theme
* Browser colour
* Border colour
* Background colour
* Text colour
* Homepage
* Search engine
* Zoom level

A configuration file can be supplied when launching LewWeb:

`lewweb --config /path/to/lewweb.json`

## Building

LewWeb requires:

* C++17 or newer
* CMake
* Qt 6
* QtWebEngine

Clone the repository and build it using the included build system.

The resulting executable can then be run directly or installed somewhere on your `PATH`.

## Project Status

**Version: 1.0.0**

LewWeb is currently an experimental browser project and is actively being developed.

The first version focuses on establishing the core architecture and the CLI-controlled browser experience.

Expect things to change as the project develops.

## Technology

LewWeb is built with:

**C++ → Qt 6 → QtWebEngine → Chromium**

Qt handles the application interface and browser integration, while QtWebEngine provides the underlying Chromium-based web rendering engine.

## Philosophy

LewWeb follows a simple philosophy:

> **Less browser chrome. More browser.**

The interface doesn't try to imitate every feature of a conventional browser.

Instead, it asks what happens when browsing is controlled primarily through commands and the keyboard.

---

# Third-Party Software & Legal Notices

LewWeb is an independent project and is **not affiliated with, endorsed by, or sponsored by** The Qt Company, Google, or the Chromium Project.

LewWeb makes use of third-party software and libraries. Their respective copyrights and licenses remain with their original authors and copyright holders.

## Qt 6

LewWeb uses **Qt 6**, including Qt modules used for the application interface and browser integration.

Qt is developed and maintained by **The Qt Company Ltd.** and contributors.

Qt is available under various licensing options, including the **GNU Lesser General Public License version 3 (LGPLv3)** and, for applicable modules, the **GNU General Public License (GPL)**, as well as commercial licensing. The applicable license depends on the Qt components being used and how they are distributed.

For licensing information:

[Qt Licensing](https://www.qt.io/licensing/?utm_source=chatgpt.com)

[Qt 6 Licensing Documentation](https://doc.qt.io/qt-6/licensing.html?utm_source=chatgpt.com)

## Qt WebEngine

LewWeb uses **Qt WebEngine** for its web rendering functionality.

Qt WebEngine incorporates **Chromium**, meaning distributions of Qt WebEngine are subject to the applicable licenses and notices for both the Qt WebEngine components and the Chromium components contained within it.

For licensing information:

[Qt WebEngine Licensing](https://doc.qt.io/qt-6/qtwebengine-licensing.html?utm_source=chatgpt.com)

## Chromium

LewWeb uses Chromium through Qt WebEngine.

**Chromium is a project of The Chromium Authors and Google.**

Chromium contains software distributed under multiple open-source licenses, including BSD, Apache, LGPL, and other applicable licenses depending on the individual component. The Chromium project maintains its own third-party licensing and attribution information.

LewWeb does not claim ownership of Chromium or any of its third-party components.

For Chromium licensing and third-party information:

[Chromium Open Source Documentation](https://www.chromium.org/chromium-projects/?utm_source=chatgpt.com)

## Third-Party Components

Qt and Qt WebEngine may contain additional third-party software distributed under their own respective licenses. Only the components actually included in a particular distribution are subject to the corresponding attribution and licensing requirements. Qt provides a list of third-party components and their licenses in its documentation.

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
Copyright © 2026 Lewis Overall (xlewis1)
LewWeb is licensed under the Apache License 2.0.

**LewWeb 1.0.0**

Copyright © 2026 Lewis Overall.
