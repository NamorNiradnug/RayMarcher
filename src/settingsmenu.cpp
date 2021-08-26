#include "settingsmenu.h"
#include <QApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QIcon>
#include <QPushButton>
#include <QSettings>

SettingsMenu::SettingsMenu(QWidget *parent) : QWidget(parent)
{
    setWindowFlag(Qt::Dialog);
    setWindowModality(Qt::WindowModal);
    setWindowTitle(QApplication::applicationName() + " - " + tr("Settings"));

    auto *properties = new QFormLayout(this);

    frame_time_inp->setMinimum(0);
    properties->addRow(tr("Frame time:"), frame_time_inp);

    auto *python_path_inp_layout = new QHBoxLayout();
    connect(python_path_inp, &QLineEdit::textChanged, this, &SettingsMenu::onPythonPathChanged);
    python_path_inp_layout->addWidget(python_path_inp);
    auto *python_path_browse_btn = new QPushButton(QIcon::fromTheme("document-open"), tr("Browse"));
    connect(python_path_browse_btn, &QPushButton::released, this, &SettingsMenu::onPythonPathBrowseReleased);
    python_path_inp_layout->addWidget(python_path_browse_btn);
    properties->addRow(tr("Python path:"), python_path_inp_layout);

    connect(buttons->button(QDialogButtonBox::Cancel), &QPushButton::released, this, &QWidget::close);
    connect(buttons->button(QDialogButtonBox::Discard), &QPushButton::released, this, &SettingsMenu::readSettings);
    connect(buttons->button(QDialogButtonBox::Apply), &QPushButton::released, this, &SettingsMenu::saveSettings);
    connect(buttons->button(QDialogButtonBox::Ok), &QPushButton::released,
            [=]
            {
                saveSettings();
                close();
            });
    properties->addRow(buttons);

    setLayout(properties);
    readSettings();
}

void SettingsMenu::onPythonPathBrowseReleased()
{
    auto selected = QFileDialog::getOpenFileName(this, tr("Select Python"), QDir::rootPath());
    if (!selected.isEmpty())
    {
        python_path_inp->setText(selected);
    }
}

void SettingsMenu::onPythonPathChanged(const QString &value)
{
    if (QFile(value).exists())
    {
        python_path_inp->setStyleSheet("");
    }
    else
    {
        python_path_inp->setStyleSheet("color: red;");
    }
}

void SettingsMenu::saveSettings()
{
    QSettings settings;
    settings.setValue("frameTime", frame_time_inp->value());
    settings.setValue("pythonPath", python_path_inp->text());
    emit settingsSaved();
}

void SettingsMenu::readSettings()
{
    QSettings settings;
    frame_time_inp->setValue(settings.value("frameTime", 10).toInt());
    python_path_inp->setText(settings.value("pythonPath").toString());
}
