#include "ball.h"
#include <cmath>

Ball::Ball() : VAO(0), VBO(0), EBO(0), radius(0.05f), currentFrame(0), frameCounter(0), dampingFactor(0.7f), state(Moving) {
    position[0] = 0.0f;
    position[1] = 0.9f;
    velocity[0] = 0.0f;
    velocity[1] = 0.0f;
}

Ball::~Ball() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    for (auto texture : textures) {
        delete texture;
    }
}

void Ball::init(const std::vector<QString> &texturePaths) {
    initializeOpenGLFunctions();
    setupShaders();
    setupBuffers();

    numFrames = texturePaths.size();
    for (const auto& texturePath : texturePaths) {
        QOpenGLTexture *texture = new QOpenGLTexture(QImage(texturePath).mirrored());
        texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        texture->setMagnificationFilter(QOpenGLTexture::Linear);
        texture->setWrapMode(QOpenGLTexture::Repeat);
        textures.push_back(texture);
    }
}

void Ball::setupShaders() {
    const char *vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec2 position;
        layout(location = 1) in vec2 texCoord;

        out vec2 TexCoord;

        uniform mat4 projection;
        uniform vec2 ballPosition;
        uniform float ballRadius;

        void main() {
            gl_Position = projection * vec4(position * ballRadius + ballPosition, 0.0, 1.0);
            TexCoord = texCoord;
        }
    )";

    const char *fragmentShaderSource = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 color;

        uniform sampler2D ballTexture;

        void main() {
            vec4 texColor = texture(ballTexture, TexCoord);
            color = texColor;
        }
    )";

    shader.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    shader.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    shader.link();
}

void Ball::setupBuffers() {
    const int numSegments = 200;
    const float angleIncrement = (2.0f * 3.14159265359f) / numSegments;
    GLfloat vertices[(numSegments + 2) * 4];
    GLuint indices[numSegments * 3];

    // center of the circle
    vertices[0] = 0.0f;
    vertices[1] = 0.0f;
    // center texture coordinate
    vertices[2] = 0.5f;
    vertices[3] = 0.5f;

    for (int i = 1; i <= numSegments + 1; ++i) {
        float angle = i * angleIncrement;
        vertices[i * 4] = cos(angle);
        vertices[i * 4 + 1] = sin(angle);
        // texture coordinates
        vertices[i * 4 + 2] = cos(angle) * 0.5f + 0.5f;
        vertices[i * 4 + 3] = sin(angle) * 0.5f + 0.5f;
    }

    for (int i = 0; i < numSegments; ++i) {
        indices[i * 3] = 0;
        indices[i * 3 + 1] = i + 1;
        indices[i * 3 + 2] = i + 2;
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Ball::setProjectionMatrix(const QMatrix4x4 &projection) {
    projectionMatrix = projection;
}

void Ball::updatePhysics() {
    if (state == Moving) {
        velocity[1] += -12.0f * 0.016f;
        position[1] += velocity[1] * 0.016f;

        if (position[1] <= -0.28f) { // the ground level
            position[1] = -0.28f;
            // reduce velocity
            velocity[1] = -velocity[1] * dampingFactor;

            if (fabs(velocity[1]) < 0.01f) {
                velocity[1] = 0.0f;
                state = Stopped;
            }
        }
    }
}

void Ball::updateAnimationFrame() {
    if (state == Moving) {
        frameCounter++;
        // adjust the number of frames to control the speed
        if (frameCounter >= 10) {
            frameCounter = 0;
            currentFrame = (currentFrame + 1) % numFrames;
        }
    }
}

void Ball::render() {
    shader.bind();
    shader.setUniformValue("projection", projectionMatrix);
    shader.setUniformValue("ballPosition", position[0], position[1]);
    shader.setUniformValue("ballRadius", radius);
    textures[currentFrame]->bind();
    glBindVertexArray(VAO);

    glDrawElements(GL_TRIANGLES, 600, GL_UNSIGNED_INT, 0); // numSegments * 3
    glBindVertexArray(0);
    textures[currentFrame]->release();
    shader.release();
}

QPointF Ball::getPosition() const {
    return QPointF(position[0], position[1]);
}

BallState Ball::getState() const {
    return state;
}

QPointF Ball::getVelocity() const {
    return QPointF(velocity[0], velocity[1]);
}

void Ball::setVelocity(float vx, float vy) {
    velocity[0] = vx;
    velocity[1] = vy;
    state = Moving;
}
