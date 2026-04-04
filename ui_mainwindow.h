/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.13
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionOpen;
    QAction *actionSaveAs;
    QAction *actionExit;
    QAction *actionUndo;
    QAction *actionCatmull_Clark;
    QAction *actionDoo_Sabin;
    QAction *actionLoop;
    QAction *actionToggle_Faces;
    QAction *actionToggle_Wireframe;
    QAction *actionToggle_Vertices;
    QAction *actionToggle_Lighting;
    QAction *actionToggle_Smooth_Shading;
    QAction *actionMesh_Color;
    QAction *actionBackground_Color;
    QAction *actionAbout;
    QAction *actionChange_Wire_Color;
    QWidget *centralwidget;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuEdit;
    QMenu *menuView;
    QMenu *menuSubdivision;
    QMenu *menuHelp;
    QToolBar *toolBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1024, 768);
        actionOpen = new QAction(MainWindow);
        actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
        actionSaveAs = new QAction(MainWindow);
        actionSaveAs->setObjectName(QString::fromUtf8("actionSaveAs"));
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        actionUndo = new QAction(MainWindow);
        actionUndo->setObjectName(QString::fromUtf8("actionUndo"));
        actionCatmull_Clark = new QAction(MainWindow);
        actionCatmull_Clark->setObjectName(QString::fromUtf8("actionCatmull_Clark"));
        actionDoo_Sabin = new QAction(MainWindow);
        actionDoo_Sabin->setObjectName(QString::fromUtf8("actionDoo_Sabin"));
        actionLoop = new QAction(MainWindow);
        actionLoop->setObjectName(QString::fromUtf8("actionLoop"));
        actionToggle_Faces = new QAction(MainWindow);
        actionToggle_Faces->setObjectName(QString::fromUtf8("actionToggle_Faces"));
        actionToggle_Faces->setCheckable(true);
        actionToggle_Faces->setChecked(true);
        actionToggle_Wireframe = new QAction(MainWindow);
        actionToggle_Wireframe->setObjectName(QString::fromUtf8("actionToggle_Wireframe"));
        actionToggle_Wireframe->setCheckable(true);
        actionToggle_Wireframe->setChecked(true);
        actionToggle_Vertices = new QAction(MainWindow);
        actionToggle_Vertices->setObjectName(QString::fromUtf8("actionToggle_Vertices"));
        actionToggle_Vertices->setCheckable(true);
        actionToggle_Lighting = new QAction(MainWindow);
        actionToggle_Lighting->setObjectName(QString::fromUtf8("actionToggle_Lighting"));
        actionToggle_Lighting->setCheckable(true);
        actionToggle_Lighting->setChecked(true);
        actionToggle_Smooth_Shading = new QAction(MainWindow);
        actionToggle_Smooth_Shading->setObjectName(QString::fromUtf8("actionToggle_Smooth_Shading"));
        actionToggle_Smooth_Shading->setCheckable(true);
        actionToggle_Smooth_Shading->setChecked(true);
        actionMesh_Color = new QAction(MainWindow);
        actionMesh_Color->setObjectName(QString::fromUtf8("actionMesh_Color"));
        actionBackground_Color = new QAction(MainWindow);
        actionBackground_Color->setObjectName(QString::fromUtf8("actionBackground_Color"));
        actionAbout = new QAction(MainWindow);
        actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
        actionChange_Wire_Color = new QAction(MainWindow);
        actionChange_Wire_Color->setObjectName(QString::fromUtf8("actionChange_Wire_Color"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1024, 20));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuEdit = new QMenu(menubar);
        menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
        menuView = new QMenu(menubar);
        menuView->setObjectName(QString::fromUtf8("menuView"));
        menuSubdivision = new QMenu(menubar);
        menuSubdivision->setObjectName(QString::fromUtf8("menuSubdivision"));
        menuHelp = new QMenu(menubar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
        MainWindow->setMenuBar(menubar);
        toolBar = new QToolBar(MainWindow);
        toolBar->setObjectName(QString::fromUtf8("toolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, toolBar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuEdit->menuAction());
        menubar->addAction(menuView->menuAction());
        menubar->addAction(menuSubdivision->menuAction());
        menubar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionSaveAs);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);
        menuEdit->addAction(actionUndo);
        menuView->addAction(actionToggle_Faces);
        menuView->addAction(actionToggle_Wireframe);
        menuView->addAction(actionToggle_Vertices);
        menuView->addSeparator();
        menuView->addAction(actionToggle_Lighting);
        menuView->addAction(actionToggle_Smooth_Shading);
        menuView->addSeparator();
        menuView->addAction(actionMesh_Color);
        menuView->addAction(actionBackground_Color);
        menuView->addAction(actionChange_Wire_Color);
        menuSubdivision->addAction(actionCatmull_Clark);
        menuSubdivision->addAction(actionDoo_Sabin);
        menuSubdivision->addAction(actionLoop);
        menuHelp->addAction(actionAbout);
        toolBar->addAction(actionOpen);
        toolBar->addAction(actionSaveAs);
        toolBar->addSeparator();
        toolBar->addAction(actionUndo);
        toolBar->addSeparator();
        toolBar->addAction(actionCatmull_Clark);
        toolBar->addAction(actionDoo_Sabin);
        toolBar->addAction(actionLoop);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "SuperMeshPro", nullptr));
        actionOpen->setText(QCoreApplication::translate("MainWindow", "Open...", nullptr));
#if QT_CONFIG(shortcut)
        actionOpen->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+O", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSaveAs->setText(QCoreApplication::translate("MainWindow", "Save As OBJ...", nullptr));
#if QT_CONFIG(shortcut)
        actionSaveAs->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionExit->setText(QCoreApplication::translate("MainWindow", "Exit", nullptr));
#if QT_CONFIG(shortcut)
        actionExit->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Q", nullptr));
#endif // QT_CONFIG(shortcut)
        actionUndo->setText(QCoreApplication::translate("MainWindow", "Undo Subdivision", nullptr));
#if QT_CONFIG(shortcut)
        actionUndo->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Z", nullptr));
#endif // QT_CONFIG(shortcut)
        actionCatmull_Clark->setText(QCoreApplication::translate("MainWindow", "Catmull-Clark", nullptr));
#if QT_CONFIG(shortcut)
        actionCatmull_Clark->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+1", nullptr));
#endif // QT_CONFIG(shortcut)
        actionDoo_Sabin->setText(QCoreApplication::translate("MainWindow", "Doo-Sabin", nullptr));
#if QT_CONFIG(shortcut)
        actionDoo_Sabin->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+2", nullptr));
#endif // QT_CONFIG(shortcut)
        actionLoop->setText(QCoreApplication::translate("MainWindow", "Loop", nullptr));
#if QT_CONFIG(shortcut)
        actionLoop->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+3", nullptr));
#endif // QT_CONFIG(shortcut)
        actionToggle_Faces->setText(QCoreApplication::translate("MainWindow", "Toggle Faces", nullptr));
        actionToggle_Wireframe->setText(QCoreApplication::translate("MainWindow", "Toggle Wireframe", nullptr));
        actionToggle_Vertices->setText(QCoreApplication::translate("MainWindow", "Toggle Vertices", nullptr));
#if QT_CONFIG(shortcut)
        actionToggle_Vertices->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+V", nullptr));
#endif // QT_CONFIG(shortcut)
        actionToggle_Lighting->setText(QCoreApplication::translate("MainWindow", "Toggle Lighting", nullptr));
        actionToggle_Smooth_Shading->setText(QCoreApplication::translate("MainWindow", "Toggle Smooth Shading", nullptr));
        actionMesh_Color->setText(QCoreApplication::translate("MainWindow", "Change Mesh Color...", nullptr));
        actionBackground_Color->setText(QCoreApplication::translate("MainWindow", "Change Background Color...", nullptr));
        actionAbout->setText(QCoreApplication::translate("MainWindow", "About...", nullptr));
        actionChange_Wire_Color->setText(QCoreApplication::translate("MainWindow", "Change Wire Color", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
        menuEdit->setTitle(QCoreApplication::translate("MainWindow", "Edit", nullptr));
        menuView->setTitle(QCoreApplication::translate("MainWindow", "View", nullptr));
        menuSubdivision->setTitle(QCoreApplication::translate("MainWindow", "Subdivision", nullptr));
        menuHelp->setTitle(QCoreApplication::translate("MainWindow", "Help", nullptr));
        toolBar->setWindowTitle(QCoreApplication::translate("MainWindow", "toolBar", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
