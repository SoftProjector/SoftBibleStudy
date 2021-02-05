##**************************************************************************
##
##    softProjector - an open source media projection software
##    Copyright (C) 2017  Vladislav Kobzar
##
##    This program is free software: you can redistribute it and/or modify
##    it under the terms of the GNU General Public License as published by
##    the Free Software Foundation version 3 of the License.
##
##    This program is distributed in the hope that it will be useful,
##    but WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##    GNU General Public License for more details.
##
##    You should have received a copy of the GNU General Public License
##    along with this program.  If not, see <http:##www.gnu.org/licenses/>.
##
##**************************************************************************


QT += core \
    gui \
    widgets \
    network \
    sql \
    qml \
    quick \
    printsupport

TARGET = SoftBibleStudy
TEMPLATE = app
CONFIG += x86 ppc x86_64 ppc64 # Compile a universal build

RES_DIR = $${PWD}/unknownsys_build
win32: RES_DIR = $${PWD}/win32_build
unix:  RES_DIR = $${PWD}/unix_build
macx: RES_DIR = $${PWD}/mac_build

DESTDIR = $${RES_DIR}/bin
OBJECTS_DIR = $${RES_DIR}/obj
MOC_DIR = $${RES_DIR}/moc
UI_DIR = $${RES_DIR}/ui
RCC_DIR = $${RES_DIR}/rcc
OUT_PWD = $${RES_DIR}/bin

SOURCES += main.cpp \
    softbiblestudy.cpp \
    bible.cpp \
    settingsdialog.cpp \
    aboutdialog.cpp \
    highlight.cpp \
    managedatadialog.cpp \
    managedata.cpp \
    helpdialog.cpp \
    bibleinformationdialog.cpp \
    settings.cpp \
    spfunctions.cpp \
    moduledownloaddialog.cpp \
    moduleprogressdialog.cpp \
    displaysetting.cpp \
    printpreviewdialog.cpp
HEADERS += \
    softbiblestudy.hpp \
    bible.hpp \
    settingsdialog.hpp \
    aboutdialog.hpp \
    highlight.hpp \
    managedatadialog.hpp \
    managedata.hpp \
    helpdialog.hpp \
    bibleinformationdialog.hpp \
    settings.hpp \
    spfunctions.hpp \
    moduledownloaddialog.hpp \
    moduleprogressdialog.hpp \
    displaysetting.hpp \
    printpreviewdialog.hpp
FORMS += \
    softbiblestudy.ui \
    settingsdialog.ui \
    aboutdialog.ui \
    managedatadialog.ui \
    helpdialog.ui \
    bibleinformationdialog.ui \
    moduledownloaddialog.ui \
    moduleprogressdialog.ui \
    printpreviewdialog.ui
TRANSLATIONS += translations/softpro_de.ts\
    translations/softpro_ru.ts\
    translations/softpro_cs.ts\
    translations/softpro_ua.ts\
    translations/softpro_hy.ts
CODECFORTR = UTF-8
RESOURCES += softprojector.qrc

win32 {
    RC_FILE = softprojector.rc
}
