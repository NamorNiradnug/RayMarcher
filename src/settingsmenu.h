#pragma once

#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QScrollArea>

class SettingsMenu : public QWidget
{
    Q_OBJECT;

public:
    SettingsMenu(QWidget *parent = nullptr);

signals:
    void settingsSaved();

private slots:
    void onPythonPathBrowseReleased();
    void onPythonPathChanged(const QString &value);
    void readSettings();
    void saveSettings();

private:
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Discard |
                                                         QDialogButtonBox::Apply | QDialogButtonBox::Ok,
                                                     Qt::Horizontal);
    QSpinBox *frame_time_inp = new QSpinBox(this);
    QLineEdit *python_path_inp = new QLineEdit();
};
