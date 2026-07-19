QT += core gui widgets

CONFIG += c++17

TARGET = BridgeMonitor
TEMPLATE = app

SOURCES += \
    main.cpp \
    models.cpp \
    datastore.cpp \
    chartwidget.cpp \
    mainwindow.cpp

HEADERS += \
    models.h \
    datastore.h \
    chartwidget.h \
    mainwindow.h

DISTFILES += \
    data_sources/monitor_points.tsv \
    data_sources/风速风向2023-04-21_10-09-51.xls \
    data_sources/挠度2023-04-21_10-09-24.xls \
    data_sources/伸缩缝2023-04-21_10-19-50.xls \
    data_sources/索力2023-04-21_10-11-01.xls \
    data_sources/温湿度2023-04-21_10-09-05.xls \
    data_sources/振动2023-04-21_10-12-43.xls \
    data_sources/支座位移2023-04-21_10-10-17.xls

FORMS += \
    form.ui \
    form.ui
