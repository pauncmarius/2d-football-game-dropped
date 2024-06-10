#ifndef GLWINDOW_H
#define GLWINDOW_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include "backgroundRenderer.h"
#include "ball.h"
#include "player.h"
#include "debugRectangle.h"
#include "shadow.h"

class GLWindow : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    explicit GLWindow(QWidget *parent = nullptr);
    ~GLWindow();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void updateAnimation();

private:
    BackgroundRenderer backgroundRenderer;
    Ball ball;
    Player player;
    Shadow ballShadow;
    Shadow playerShadow;

    bool moveLeft;
    bool moveRight;
    bool jump;
    bool kick;
    bool isSpawningAnimationDone;

    DebugRectangle debugRectangle;
    QRectF goalZoneLeft;
    QRectF goalZoneRight;
    QRectF playerBoundingBox;
    QRectF ballBoundingBox;

    QMatrix4x4 projection;

};

#endif // GLWINDOW_H
