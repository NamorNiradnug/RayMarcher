#include "mainwindow.h"

#include <QApplication>
#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>

MainWindow::MainWindow(QApplication *app, QWidget *parent) : QMainWindow(parent)
{
    resize(640, 480);

    renderer = new ShaderRenderer(this);
    setCentralWidget(renderer);

    file_menu = menuBar()->addMenu(tr("&File"));
    control_menu = menuBar()->addMenu(tr("&Control"));
    view_menu = menuBar()->addMenu(tr("&View"));
    help_menu = menuBar()->addMenu(tr("&Help"));
    file_menu->addAction(open_act);
    createOpenBuiltinMenu();
    file_menu->addMenu(open_builtin_menu);
    updateOpenRecentMenu();
    file_menu->addMenu(open_recent_menu);
    file_menu->addAction(reload_act);
    file_menu->addSeparator();
    open_setting_act->setEnabled(false);
    file_menu->addAction(open_setting_act);
    file_menu->addSeparator();
    file_menu->addAction(quit_act);

    pause_time_act->setCheckable(true);
    camera_control_mode_act->setCheckable(true);
    control_menu->addAction(pause_time_act);
    control_menu->addSection(tr("Camera"));
    control_menu->addActions({
        camera_control_mode_act,
        camera_go_home_act,
    });
    setDisabledWithActions(control_menu, true);

    show_menubar_act->setCheckable(true);
    show_menubar_act->setChecked(true);
    show_fullscreen_act->setCheckable(true);
    view_menu->addActions({
        show_menubar_act,
        show_fullscreen_act,
    });
    view_menu->addSection("Toolbars");
    view_menu->addActions({
        camera_properties->toggleViewAction(),
        raymarching_properties->toggleViewAction(),
    });

    help_menu->addActions({
        about_act,
        license_act,
        about_qt_act,
    });

#define ADD_SLIDER_PROPERTY(name, min_val, max_val, start_value, on_changed, tool_bar)                                 \
    QSlider *__slider##on_changed = new QSlider(Qt::Horizontal);                                                       \
    __slider##on_changed->setMinimum(min_val);                                                                         \
    __slider##on_changed->setMaximum(max_val);                                                                         \
    connect(__slider##on_changed, SIGNAL(valueChanged(int)), this, SLOT(on_changed(int)));                             \
    __slider##on_changed->setValue(start_value);                                                                       \
    (tool_bar)->addWidget(sliderWithLabel(tr(name), __slider##on_changed));

    camera_properties->addAction(camera_control_mode_act);
    ADD_SLIDER_PROPERTY("Speed", 100, 400, 200, changeCameraSpeed, camera_properties);
    ADD_SLIDER_PROPERTY("Fov", 30, 120, 90, changeCameraFov, camera_properties);
    ADD_SLIDER_PROPERTY("Rotation Sensitivity", 10, 200, 100, changeCameraRotationSensitivity, camera_properties);

    enable_shadows_act->setCheckable(true);
    enable_shadows_act->setChecked(renderer->properties.is_shadows_enabled);
    raymarching_properties->addAction(enable_shadows_act);
    ADD_SLIDER_PROPERTY("Render Distance", 5, 500, 100, changeRenderDistance, raymarching_properties);
    ADD_SLIDER_PROPERTY("Max Steps", 10, 2000, 1000, changeMaxSteps, raymarching_properties);
    ADD_SLIDER_PROPERTY("Min Hit Distance", 2, 500, 50, changeMinHitDistance, raymarching_properties);

    addToolBar(camera_properties);
    addToolBar(raymarching_properties);

#undef ADD_SLIDER_PROPERTY

    open_act->setShortcut(QKeySequence::Open);
    reload_act->setShortcut(QKeySequence::Refresh);
    open_setting_act->setShortcut(QKeySequence::Preferences);
    show_menubar_act->setShortcut(Qt::CTRL + Qt::Key_M);
    show_fullscreen_act->setShortcut(QKeySequence::FullScreen);
    quit_act->setShortcut(QKeySequence::Quit);

    pause_time_act->setShortcut(Qt::Key_T);
    camera_control_mode_act->setShortcut(Qt::Key_Escape);
    camera_go_home_act->setShortcut(Qt::Key_Home);

    about_act->setShortcut(QKeySequence::HelpContents);

    connect(file_menu, &QMenu::aboutToShow, this, &MainWindow::updateOpenRecentMenu);

    connect(open_act, &QAction::triggered, this, &MainWindow::open);
    connect(reload_act, &QAction::triggered, this, &MainWindow::reload);
    connect(open_setting_act, &QAction::triggered, this, &MainWindow::openSetting);
    connect(quit_act, &QAction::triggered, this, &MainWindow::close);
    connect(pause_time_act, &QAction::triggered, this, &MainWindow::toggleTimeRunning);
    connect(camera_control_mode_act, &QAction::toggled, this, &MainWindow::toggleCameraControlMode);
    connect(camera_go_home_act, &QAction::triggered, this, &MainWindow::moveCameraHome);
    connect(show_menubar_act, &QAction::toggled, this, &MainWindow::toggleMenubar);
    connect(show_fullscreen_act, &QAction::toggled, this, &MainWindow::toggleFullscreen);
    connect(about_act, &QAction::triggered, this, &MainWindow::about);
    connect(license_act, &QAction::triggered, this, &MainWindow::license);
    connect(about_qt_act, &QAction::triggered, app, &QApplication::aboutQt);

    connect(enable_shadows_act, &QAction::toggled, this, &MainWindow::toggleShadows);
}

void MainWindow::open()
{
    openScene(QFileDialog::getOpenFileName(this, tr("Choose scene"), QDir::homePath()));
}

void MainWindow::openByTriggeredAction()
{
    QAction *triggerer = qobject_cast<QAction *>(sender());
    if (triggerer != nullptr)
    {
        openScene(triggerer->property("scene_file_name").toString());
    }
}

void MainWindow::openScene(const QString &f)
{
    if (!f.isEmpty())
    {
        QSettings settings;
        QStringList updated_history = settings.value("history").toStringList();
        updated_history.removeAll(f);
        updated_history.append(f);
        settings.setValue("history", updated_history);
        if (!QFile(f).exists())
        {
            QMessageBox::critical(this, tr("File not found"), tr("File at ") + f + tr(" not found"));
        }
        current_scene = f;
        renderer->setScene(current_scene);
        setDisabledWithActions(control_menu, !renderer->program->isLinked());
        if (!renderer->program->isLinked())
        {
            QMessageBox::critical(this, tr("Bad Scene"), tr("Scene at ") + f + tr(" contains errors."));
        }
        if (!renderer->scene.getName().isEmpty())
        {
            setWindowTitle("RayMarcher - " + renderer->scene.getName());
        }
        else
        {
            setWindowTitle("RayMarcher - " + renderer->scene.getFile());
        }
    }
}

void MainWindow::updateOpenRecentMenu()
{
    open_recent_menu->clear();

    QSettings settings;
    QStringList history = settings.value("history").toStringList();
    QList<QAction *> recent_files;
    for (int i = 0; i < settings.value("max_recent", 5).toInt() && i < history.size(); ++i)
    {
        recent_files.append(prettyOpenFileAction(history[history.size() - i - 1], true));
        connect(recent_files.back(), &QAction::triggered, this, &MainWindow::openByTriggeredAction);
        recent_files.back()->setDisabled(!QFile(recent_files.back()->property("scene_file_name").toString()).exists());
    }

    open_recent_menu->addActions(recent_files);
    open_recent_menu->setDisabled(recent_files.isEmpty());
}

void MainWindow::createOpenBuiltinMenu()
{
    QDir examples(":/examples/examples");
    QList<QAction *> scene_actions;
    for (const QString &scene : examples.entryList(QDir::Files))
    {
        qDebug() << "Built-in scene" << scene << "found.";
        scene_actions.append(prettyOpenFileAction(examples.filePath(scene)));
        connect(scene_actions.back(), &QAction::triggered, this, &MainWindow::openByTriggeredAction);
    }
    open_builtin_menu->addActions(scene_actions);
}

void MainWindow::reload()
{
    openScene(current_scene);
}

void MainWindow::openSetting()
{
}

void MainWindow::toggleTimeRunning()
{
    renderer->is_time_running = !pause_time_act->isChecked();
}

void MainWindow::toggleCameraControlMode()
{
    renderer->setCameraControlMode(camera_control_mode_act->isChecked());
}

void MainWindow::moveCameraHome()
{
    renderer->camera.position = QVector3D();
    renderer->update();
}

void MainWindow::toggleMenubar()
{
    static int saved_height = menuBar()->height();
    if (show_menubar_act->isChecked())
    {
        menuBar()->setFixedHeight(saved_height);
    }
    else
    {
        saved_height = menuBar()->height();
        menuBar()->setFixedHeight(0);
    }
}

void MainWindow::toggleFullscreen()
{
    if (show_fullscreen_act->isChecked())
    {
        showFullScreen();
    }
    else
    {
        showNormal();
    }
}

void MainWindow::about()
{
    QFile f(":/about/ABOUT.html");
    f.open(QFile::ReadOnly);
    QMessageBox::about(this, "About RayMarcher", f.readAll());
    f.close();
}

void MainWindow::license()
{
    QFile f(":/about/LICENSE");
    f.open(QFile::ReadOnly);
    QByteArray text = f.readAll();
    text.replace("\n\n", "$$");
    text.replace("\n", "");
    text.replace("$$", "\n\n");
    QMessageBox::about(this, "RayMarcher License", text);
    f.close();
}

void MainWindow::toggleShadows()
{
    renderer->properties.is_shadows_enabled = enable_shadows_act->isChecked();
}

void MainWindow::changeRenderDistance(int slider_value)
{
    renderer->properties.render_distance = slider_value;
}

void MainWindow::changeMaxSteps(int slider_value)
{
    renderer->properties.max_steps = slider_value;
}

void MainWindow::changeMinHitDistance(int slider_value)
{
    renderer->properties.min_hit_distance = slider_value / 1000000.0;
}

void MainWindow::changeCameraSpeed(int slider_value)
{
    renderer->camera.speed = slider_value / 100.0;
}

void MainWindow::changeCameraFov(int slider_value)
{
    renderer->camera.fov = qDegreesToRadians((qreal)slider_value);
}

void MainWindow::changeCameraRotationSensitivity(int slider_value)
{
    renderer->camera.rotation_sensitivity = slider_value / 100.0;
}

QWidget *MainWindow::sliderWithLabel(const QString &str, QSlider *slider)
{
    auto *layout = new QFormLayout();
    slider->setMinimumWidth(100);
    layout->addRow(str, slider);
    auto *w = new QWidget();
    w->setLayout(layout);
    return w;
}

QAction *MainWindow::prettyOpenFileAction(const QString &file, bool add_suffix)
{
    auto *action = new QAction(file);
    action->setProperty("scene_file_name", file);
    if (!Scene(file).getName().isEmpty())
    {
        action->setText(Scene(file).getName());
    }
    if (add_suffix)
    {
        if (file[0] == ":")
        {
            action->setText(action->text() + " (Built-in)");
        }
        else if (action->text() != file)
        {
            action->setText(action->text() + " (" + file + ")");
        }
    }
    return action;
}

void MainWindow::setDisabledWithActions(QMenu *menu, bool value)
{
    menu->setDisabled(value);
    for (QAction *act : menu->actions())
    {
        act->setDisabled(value);
    }
}
