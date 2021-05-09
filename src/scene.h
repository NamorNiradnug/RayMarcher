#pragma once

#include "lazy.h"
#include <QFile>
#include <QMap>
#include <QOpenGLShaderProgram>

struct RotationSource
{
    Lazy *x;
    Lazy *y;
    Lazy *z;
    Lazy *angle;
};

struct LazyVector3D
{
    Lazy *x, *y, *z;
};

class Scene
{
public:
    inline static const QString TEMPLATE_SHADER_PATH = ":/shaders/shaders/raymarcher.frag";
    inline static const QByteArray TRANSFORM_PREFIX = "__TRANSFORM_";
    inline static const QByteArray CMD_VAR_DEF = "SDF";
    inline static const QByteArray CMD_SCENE_DEF = "SCENE";
    inline static const QByteArray CMD_NAME = "NAME";
    inline static const QByteArray CMD_COLOR_DEF = "COLOR";
    inline static const QByteArray CMD_LIGHT = "LIGHT";
    inline static const QByteArray COMMENT_SEQ = "//";
    inline static const QByteArrayList TEMPLATES_TYPES = {"Intersection", "Union", "Difference"};
    inline static const QByteArray UNIFROMS_MACRO = "TEMPLATE_UNIFORMS";
    inline static const QByteArray SDSCENE_MACRO = "TEMPLATE_SDSCENE";
    inline static const QByteArray COLOR_MACRO = "TEMPLATE_COLOR";

    Lazy *addVariable(const QByteArray &name, Lazy *value);
    Lazy *variable(const QByteArray &varname) const;

    Scene(QString f);
    void parse(const QSet<const QByteArray> &cmds = {});
    QByteArray generateShader() const;
    void setUniformsValues(QOpenGLShaderProgram *prog) const;
    const QByteArray &getName();
    bool isParsed() const;

    const QString &getFile() const;

private:
    QString f;

    QMap<QByteArray, Lazy *> variables;
    QMap<QByteArray, Lazy *> params_values;
    QMap<QByteArray, LazyVector3D> translations;
    QMap<QByteArray, QByteArray> vartypes;
    QMap<QByteArray, RotationSource> rotations;

    bool parsed = false;
    QMap<QByteArray, QByteArray> macros = {{COLOR_MACRO, "vec3(1.0,1.0,1.0)"}};
    QByteArray scene_var;
    QByteArray name;
};
