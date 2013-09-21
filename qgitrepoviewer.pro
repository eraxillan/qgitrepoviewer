#-------------------------------------------------
#
# Project created by QtCreator 2013-08-24T13:38:22
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qgitrepoviewer
TEMPLATE = app

#
# Add path to libgit2 - library for working with git scm repositories
#
PROJ_ROOT    = ..
GIT2ROOT     = $$PROJ_ROOT/libgit2
INCLUDEPATH += $${GIT2ROOT}/include
LIBS        += -L$$PROJ_ROOT/libgit2-build -lgit2

SOURCES += main.cpp\
	CCommitModel.cpp \
	CBranchModel.cpp \
    CSearchLineWidget.cpp \
    CMainWindow.cpp

HEADERS  += \
	CCommitModel.h \
	CBranchModel.h \
    CSearchLineWidget.h \
    CMainWindow.h

FORMS    += \
    CSearchLineWidget.ui \
    CMainWindow.ui
