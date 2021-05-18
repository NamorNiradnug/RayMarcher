#pragma once

#include "lazy.h"
#include <QFile>
#include <QMap>
#include <QOpenGLShaderProgram>

struct RotationSource
{
    Lazy *x, *y, *z, *angle;
    inline QVector4D value() const
    {
        return QVector4D(
            QVector4D(Lazy3D{x, y, z}.value().normalized() * qSin(angle->value() / 2), qCos(angle->value() / 2)));
    };
};

class Scene
{
public:
    inline static const QString TEMPLATE_SHADER_PATH = ":/shaders/shaders/raymarcher.frag";
    inline static const QByteArray TRANSFORM_PREFIX = "_TRANSFORM_";
    inline static const QByteArray GENERATED_PREFIX = "__GENERATED_";
    inline static const QByteArray CMD_VAR_DEF = "SDF";
    inline static const QByteArray CMD_SCENE_DEF = "SCENE";
    inline static const QByteArray CMD_NAME = "NAME";
    inline static const QByteArray CMD_COLOR_DEF = "COLOR";
    inline static const QByteArray CMD_SUN = "SUN";
    inline static const QByteArray CMD_SDFTYPE = "SDFTYPE";
    inline static const QByteArray COMMENT_SEQ = "//";
    inline static const QMap<QByteArray, QList<QByteArray>> TEMPLATES_TYPES = {
        {"Intersection", {}}, {"Union", {}}, {"Difference", {}}, {"SmoothUnion", {"k"}}};
    inline static const QByteArray UNIFORMS_MACRO = "TEMPLATE_UNIFORMS";
    inline static const QByteArray SDSCENE_MACRO = "TEMPLATE_SDSCENE";
    inline static const QByteArray SDFTYPES_MACRO = "TEMPLATE_SDFTYPES";

    Lazy *addVariable(const QByteArray &name, Lazy *value);
    Lazy *variable(const QByteArray &varname) const;

    Scene(const QString &f);
    void parse(const QSet<const QByteArray> &cmds = {});
    QByteArray generateShader() const;
    void setUniformsValues(QOpenGLShaderProgram *prog) const;
    const QByteArray &getName();
    bool isParsed() const;

    const QString &getFile() const;

private:
    QString f;

    QMap<QByteArray, Lazy *> variables = {{"PI", Lazy::number(M_PI)}};
    QMap<QByteArray, Lazy *> params_values;
    QMap<QByteArray, Lazy3D> params3d_values = {
        {"sun.color", {Lazy::number(1.0), Lazy::number(1.0), Lazy::number(1.0)}}};
    QMap<QByteArray, Lazy3D> normal_params3d_values = {
        {"sun.dir", {Lazy::number(1.0), Lazy::number(1.0), Lazy::number(1.0)}}};
    QMap<QByteArray, RotationSource> rotations;
    QMap<QByteArray, QByteArray> vartypes;

    bool parsed = false;
    QMap<QByteArray, QByteArray> macros;
    QByteArray scene_var;
    QByteArray name;
};
