#include "shaderrenderer.h"
#include <QKeyEvent>
#include <QSettings>

ShaderRenderer::ShaderRenderer(QWidget *parent) : QOpenGLWidget(parent)
{
    fps_label->setFont(QFont("Monospace"));
    fps_label->setStyleSheet("QLabel { background-color: rgba(0, 0, 0, 150); }");
    computeNewDirection();
    current_timer_id = startTimer(QSettings().value("frameTime", 10).toInt());
}

void ShaderRenderer::mouseReleaseEvent(QMouseEvent * /*event*/)
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

void ShaderRenderer::focusOutEvent(QFocusEvent * /*event*/)
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
        QCursor::setPos(globalGeometry().center());
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

void ShaderRenderer::setFragmentShader(const QString &shader)
{
    setCameraControlMode(false);
    frag_shader->compileSourceCode(shader);
    if (!frag_shader->isCompiled())
    {
        setNoShader();
    }
    else
    {
        shader_set = true;
    }
    program->link();
}

void ShaderRenderer::setNoShader()
{
    shader_set = false;
    frag_shader->compileSourceCode("void main(){}");
    program->link();
}

void ShaderRenderer::updateFrameTime()
{
    killTimer(current_timer_id);
    current_timer_id = startTimer(QSettings().value("frameTime", 10).toInt());
}

void ShaderRenderer::timerEvent(QTimerEvent * /*event*/)
{
    auto current_time = QDateTime::currentMSecsSinceEpoch();
    auto delta = (qreal)(current_time - last_frame_time) / 1000.0; ///< time since last frame in sec
    last_frame_time = current_time;
    if (is_time_running)
    {
        time += delta * time_speed;
    }
    if (camera_control_mode)
    {
        cameraRotation();
        cameraMovement(delta);
    }
    if (current_time - last_fps_update >= 500) // updating FPS every 500 msec
    {
        fps_label->setText(QString::number(1 / delta, 'f', 3));
        fps_label->resize(fps_label->sizeHint());
        last_fps_update = current_time;
    }
    update();
}

void ShaderRenderer::initializeGL()
{
    initializeOpenGLFunctions();
    frag_shader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    program = new QOpenGLShaderProgram(this);
    program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vertexshader.vert");
    program->addShader(frag_shader);
    setNoShader();
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
    if (shader_set)
    {
        program->bind();
        program->setUniformValue("TIME", (GLfloat)time);
        program->setUniformValue("WIDTH", (GLfloat)geometry().width());
        program->setUniformValue("HEIGHT", (GLfloat)geometry().height());
        program->setUniformValue("camera.position", camera.position);
        program->setUniformValue("camera.direction", camera.direction);
        program->setUniformValue("camera.fov2_tan", (GLfloat)qTan(camera.fov / 2));
        // TODO(NamorNiradnug): Defining lighting source position in scene
        program->setUniformValue("sun", QVector3D(qSin(time * 0.2), 0.5, qCos(time * 0.2)).normalized());
        program->setUniformValue("SHADOWS_ENABLED", (GLint)properties.is_shadows_enabled);
        program->setUniformValue("RENDER_DISTANCE", (GLfloat)properties.render_distance);
        program->setUniformValue("MIN_HIT_DIST", (GLfloat)properties.min_hit_distance);
        program->setUniformValue("MAX_STEPS", properties.max_steps);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        program->release();
    }
}

void ShaderRenderer::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}
