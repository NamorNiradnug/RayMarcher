#pragma once

#include "scene.h"

#include <QDateTime>
#include <QFile>
#include <QOpenGLFunctions>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QTimer>
#include <QtMath>

class ShaderRenderer : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    ShaderRenderer(QWidget *parent = nullptr);
    void mouseReleaseEvent(QMouseEvent *) override;
    void keyPressEvent(QKeyEvent *ev) override;
    void keyReleaseEvent(QKeyEvent *ev) override;
    void focusOutEvent(QFocusEvent *) override;
    void setCameraControlMode(bool value);
    void cameraMovement(qreal delta);
    void cameraRotation();
    void setScene(const QString &f);

protected slots:
    void timerEvent(QTimerEvent *) override;
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

private:
    friend class MainWindow;

    inline QRect globalGeometry();
    inline void computeNewDirection();

    QSet<int> pressed_keys = {};
    bool camera_control_mode = false;
    QPointF mouse_offsets = QPointF(0, 0);
    struct Camera
    {
        QVector3D position;
        QVector3D direction;
        qreal fov;
        qreal speed;
        qreal rotation_sensitivity;
    } camera;
    struct RayMarchingProperties
    {
        bool is_shadows_enabled = true;
        qreal render_distance;
        qreal min_hit_distance;
        qint32 max_steps;
    } properties;
    QOpenGLShaderProgram *program;
    Scene scene = Scene("");
    QOpenGLShader *frag_shader;
    QString source;
    bool is_time_running = true;
    qreal time_speed = 1;
    qreal time = 0;
};

inline QRect ShaderRenderer::globalGeometry()
{
    return QRect(mapToGlobal(QPoint()), geometry().size());
}

inline void ShaderRenderer::computeNewDirection()
{
    camera.direction = QVector3D(qCos(mouse_offsets.y()) * qCos(-mouse_offsets.x()), qSin(mouse_offsets.y()),
                                 qCos(mouse_offsets.y()) * qSin(-mouse_offsets.x()))
                           .normalized();
}
