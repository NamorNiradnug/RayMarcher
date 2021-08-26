#pragma once

#include "shaderrenderer.h"
#include <QActionGroup>
#include <QFile>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QSlider>
#include <QToolBar>

class QProcess;
class QFormLayout;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void openScene(const QString &path);

private slots:
    void updateOpenRecentMenu();

    void openSceneDialog();
    void openSceneByTriggeredAction();
    void toggleMenubar();
    void toggleFullscreen();
    void about();
    void license();

    void changeRenderDistance(int slider_value);
    void changeMaxSteps(int slider_value);
    void changeMinHitDistance(int slider_value);

    void changeCameraSpeed(int slider_value);
    void changeCameraFov(int slider_value);
    void changeCameraRotationSensitivity(int slider_value);

private:
    static QWidget *sliderWithLabel(const QString &str, QSlider *slider);
    static QAction *prettyOpenFileAction(const QString &file, bool add_suffix = false);
    static void disableMenuWithActions(QMenu *menu, bool is_disabled = true);

    void afterSceneProcessingFinished(QProcess *python, const QString &path);

    void createOpenBuiltinMenu();

    inline bool checkOpeningFileExists(const QString &path);
    inline void setNoScene();

    ShaderRenderer *renderer;
    QString current_scene;
    QProcess *scene_processor = nullptr;
    QMessageBox *info_box = new QMessageBox(QMessageBox::Information, tr("RayMarcher - status"), "",
                                            QMessageBox::Ok | QMessageBox::Close, this);

    QToolBar *raymarching_properties = new QToolBar(tr("RayMarching"), this);
    QToolBar *camera_properties = new QToolBar(tr("Camera"), this);

    QMenu *file_menu;
    QMenu *control_menu;
    QMenu *view_menu;
    QMenu *help_menu;

    QAction *open_act = new QAction(QIcon::fromTheme("document-open"), tr("&Open..."));
    QMenu *open_builtin_menu = new QMenu(tr("Open Built-in Scene..."));
    QMenu *open_recent_menu = new QMenu(tr("Open &Recent"));
    QAction *reload_act = new QAction(QIcon::fromTheme("view-refresh"), tr("Re&load Current Scene"));

    QAction *open_setting_act = new QAction(QIcon::fromTheme("configure"), tr("&Settings"));

    QAction *quit_act = new QAction(QIcon::fromTheme("application-exit"), tr("&Quit"));

    QAction *pause_time_act = new QAction(tr("Pause &Time"));

    QAction *camera_control_mode_act = new QAction(tr("&Camera Control Mode"));
    QAction *camera_go_home_act = new QAction(QIcon::fromTheme("go-home"), tr("Move Camera to &Zero"));

    QAction *show_menubar_act = new QAction(tr("Show &Menubar"));
    QAction *show_fullscreen_act = new QAction(QIcon::fromTheme("view-fullscreen"), tr("Show &Fullscreen"));

    QAction *about_act = new QAction(QIcon::fromTheme("system-help"), tr("&About RayMarcher"));
    QAction *license_act = new QAction(tr("License"));
    QAction *about_qt_act = new QAction(QIcon::fromTheme("qt"), tr("About &Qt..."));

    QAction *enable_shadows_act = new QAction(tr("Enable Shadows"));
};

inline bool MainWindow::checkOpeningFileExists(const QString &path)
{
    if (!QFile::exists(path))
    {
        QMessageBox::warning(this, tr("File Not Found"), tr("File at '") + path + "' cannot be found!");
        renderer->setNoShader();
        return false;
    }
    return true;
}

inline void MainWindow::setNoScene()
{
    renderer->setNoShader();
    disableMenuWithActions(control_menu);
}
