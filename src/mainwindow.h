#pragma once

#include "shaderrenderer.h"
#include <QActionGroup>
#include <QFile>
#include <QMainWindow>
#include <QMenu>
#include <QSlider>
#include <QToolBar>

class QFormLayout;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QApplication *app, QWidget *parent = nullptr);
    void openScene(const QString &f);

private slots:
    void updateOpenRecentMenu();

    void open();
    void openByTriggeredAction();
    void reload();
    void openSetting();
    void toggleTimeRunning();
    void toggleCameraControlMode();
    void moveCameraHome();
    void toggleMenubar();
    void toggleFullscreen();
    void about();

    void toggleShadows();

    void changeRenderDistance(int slider_value);
    void changeMaxSteps(int slider_value);
    void changeMinHitDistance(int slider_value);

    void changeCameraSpeed(int slider_value);
    void changeCameraFov(int slider_value);
    void changeCameraRotationSensitivity(int slider_value);

private:
    static QWidget *sliderWithLabel(const QString &str, QSlider *slider);

    static QAction *prettyOpenFileAction(const QString &file, bool add_suffix = false);
    static void setDisabledWithActions(QMenu *menu, bool value);
    void createOpenBuiltinMenu();

    ShaderRenderer *renderer;
    QString current_scene;

    QToolBar *raymarching_properties = new QToolBar(tr("RayMarching"), this);
    QToolBar *camera_properties = new QToolBar(tr("Camera"), this);

    QMenu *file_menu;
    QMenu *control_menu;
    QMenu *view_menu;
    QMenu *help_menu;

    QAction *open_act = new QAction(QIcon::fromTheme("document-open"), tr("&Open..."));
    QMenu *open_builtin_menu = new QMenu(tr("Open Built-in Scene..."));
    QMenu *open_recent_menu = new QMenu(tr("Open &Recent"));
    QAction *reload_act = new QAction(tr("Re&load Current Scene"));

    QAction *open_setting_act = new QAction(tr("&Settings"));

    QAction *quit_act = new QAction(tr("&Quit"));

    QAction *pause_time_act = new QAction(tr("Pause &Time"));

    QAction *camera_control_mode_act = new QAction(tr("&Camera Control Mode"));
    QAction *camera_go_home_act = new QAction(tr("Move Camera to &Zero"));

    QAction *show_menubar_act = new QAction(tr("Show &Menubar"));
    QAction *show_toolbar_act = new QAction(tr("Show Toolbar..."));
    QAction *show_fullscreen_act = new QAction(tr("Show &Fullscreen"));

    QAction *about_act = new QAction(tr("&About SimpleRayMarcher"));
    QAction *about_qt_act = new QAction(QIcon::fromTheme("qt"), tr("About &Qt..."));

    QAction *enable_shadows_act = new QAction(tr("Enable Shadows"));
};
