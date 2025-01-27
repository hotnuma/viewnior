TEMPLATE = app
TARGET = src/viewnior
CONFIG = c99 link_pkgconfig
DEFINES = _GNU_SOURCE
INCLUDEPATH =
PKGCONFIG =

PKGCONFIG += gtk+-3.0
PKGCONFIG += gdk-pixbuf-2.0
PKGCONFIG += gio-2.0
PKGCONFIG += glib-2.0
PKGCONFIG += exiv2
PKGCONFIG += shared-mime-info

SOURCES = \
    src/main.c \
    src/uni-anim-view.c \
    src/uni-cache.c \
    src/uni-dragger.c \
    src/uni-exiv2.cpp \
    src/uni-image-view.c \
    src/uni-nav.c \
    src/uni-scroll-win.c \
    src/uni-utils.c \
    src/vnr-crop.c \
    src/vnr-file.c \
    src/vnr-message-area.c \
    src/vnr-prefs.c \
    src/vnr-properties-dialog.c \
    src/vnr-tools.c \
    src/vnr-window.c

DISTFILES = \
    README.md \
    install.sh \
    meson.build \
    meson_post_install.py \
    src/meson.build

HEADERS += \
    src/uni-anim-view.h \
    src/uni-cache.h \
    src/uni-dragger.h \
    src/uni-exiv2.hpp \
    src/uni-image-view.h \
    src/uni-nav.h \
    src/uni-scroll-win.h \
    src/uni-utils.h \
    src/uni-zoom.h \
    src/vnr-crop.h \
    src/vnr-file.h \
    src/vnr-message-area.h \
    src/vnr-prefs.h \
    src/vnr-properties-dialog.h \
    src/vnr-tools.h \
    src/vnr-window.h


