#include "shaderrenderer.h"
#include "lazy.h"
#include <QKeyEvent>
#include <QPixmap>

#define PERIODIC_FUNC                                                                                                  \
    static qint64 last_call_time__ = QDateTime::currentMSecsSinceEpoch();                                              \
    qint64 current_time__ = QDateTime::currentMSecsSinceEpoch();                                                       \
    qreal delta = (current_time__ - last_call_time__) / 1000.0; /* time from last call of the function in secs */      \
    last_call_time__ = current_time__;

ShaderRenderer::ShaderRenderer(QWidget *parent) : QOpenGLWidget(parent)
{
    computeNewDirection();
    startTimer(10);
}

void ShaderRenderer::mouseReleaseEvent(QMouseEvent *)
{
    setFocus(Qt::MouseFocusReason);
}

void ShaderRenderer::keyPressEvent(QKeyEvent *ev)
{
    pressed_keys.insert(ev->key());
}

void ShaderRenderer::keyReleaseEvent(QKeyEvent *ev)
{
    pressed_keys.remove(ev->key());
}

void ShaderRenderer::focusOutEvent(QFocusEvent *)
{
    pressed_keys.clear();
    setCameraControlMode(false);
}

void ShaderRenderer::setCameraControlMode(bool value)
{
    QPixmap transparent_pixmap(32, 32);
    transparent_pixmap.fill(Qt::transparent);
    camera_control_mode = value;
    if (camera_control_mode)
    {
        setFocus(Qt::OtherFocusReason);
        setCursor(QCursor(transparent_pixmap));
        cursor().setPos(globalGeometry().center());
    }
    else
    {
        unsetCursor();
    }
}

void ShaderRenderer::cameraMovement(qreal delta)
{
    static const QVector3D UP(0, 1, 0);
    QVector3D forward = QVector3D(camera.direction.x(), 0, camera.direction.z()).normalized();
    QVector3D right = QVector3D::normal(forward, UP);
    QVector3D velocity;
    if (pressed_keys.contains(Qt::Key_W))
    {
        velocity += forward;
    }
    if (pressed_keys.contains(Qt::Key_S))
    {
        velocity -= forward;
    }
    if (pressed_keys.contains(Qt::Key_D))
    {
        velocity += right;
    }
    if (pressed_keys.contains(Qt::Key_A))
    {
        velocity -= right;
    }
    if (pressed_keys.contains(Qt::Key_Space))
    {
        velocity += UP;
    }
    if (pressed_keys.contains(Qt::Key_Shift))
    {
        velocity -= UP;
    }
    camera.position += camera.speed * velocity.normalized() * delta;
}

void ShaderRenderer::cameraRotation()
{
    mouse_offsets += ((QPointF)globalGeometry().center() - cursor().pos()) * camera.rotation_sensitivity * 0.01;
    mouse_offsets.setY(qMax(-M_PI_2 + 0.01, qMin(M_PI_2 - 0.01, mouse_offsets.y())));
    computeNewDirection();
    cursor().setPos(globalGeometry().center());
}

void ShaderRenderer::setScene(const QString &f)
{
    scene = Scene(f);
    scene.addVariable("TIME", Lazy::number(&time));
    scene.parse();
    frag_shader->compileSourceCode(scene.generateShader());
    program->link();
    setCameraControlMode(false);
}

void ShaderRenderer::timerEvent(QTimerEvent * /*event*/)
{
    PERIODIC_FUNC;
    if (is_time_running)
    {
        time += delta * time_speed;
    }
    if (camera_control_mode)
    {
        cameraRotation();
        cameraMovement(delta);
    }
    if (program->isLinked())
    {
        update();
    }
}

void ShaderRenderer::initializeGL()
{
    initializeOpenGLFunctions();
    frag_shader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    frag_shader->compileSourceCode("void main(){}");
    program = new QOpenGLShaderProgram(this);
    program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/shaders/vertexshader.vert");
    program->addShader(frag_shader);
    program->link();
    program->enableAttributeArray("attr_pos");
    static const GLfloat vertices[] = {1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f};
    program->setAttributeArray("attr_pos", GL_FLOAT, vertices, 2);
    glClearColor(0, 0, 0, 1);
}

QVector4D rotation(QVector3D axis, qreal angle)
{
    return QVector4D(axis.normalized() * qSin(angle / 2), qCos(angle / 2));
}

void ShaderRenderer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    if (program->isLinked())
    {
        program->bind();
        program->setUniformValue("TIME", (GLfloat)time);
        program->setUniformValue("WIDTH", (GLfloat)geometry().width());
        program->setUniformValue("HEIGHT", (GLfloat)geometry().height());
        program->setUniformValue("camera.position", camera.position);
        program->setUniformValue("camera.direction", camera.direction);
        program->setUniformValue("camera.fov2_tan", (GLfloat)qTan(camera.fov / 2));
        program->setUniformValue("MAIN_LIGHT_DIRECTION", QVector3D(0.5, sin(time * 0.2), cos(time * 0.2)).normalized());
        program->setUniformValue("SHADOWS_ENABLED", properties.is_shadows_enabled);
        program->setUniformValue("RENDER_DISTANCE", (GLfloat)properties.render_distance);
        program->setUniformValue("MIN_HIT_DIST", (GLfloat)properties.min_hit_distance);
        program->setUniformValue("MAX_STEPS", properties.max_steps);
        scene.setUniformsValues(program);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        program->release();
    }
}

void ShaderRenderer::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}
