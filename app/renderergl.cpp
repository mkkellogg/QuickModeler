#include "renderergl.h"

#include <QtQuick/qquickwindow.h>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLContext>

RendererGL::RendererGL() : m_t(0), m_program(nullptr), m_window(nullptr), initialized(false),
                                   engineInitialized(false), engineWindowSizeSet(false), engine(nullptr) {

}

RendererGL::~RendererGL()
{
    if (m_program != nullptr) {
        delete m_program;
        m_program = nullptr;
    }

    if (this->engine != nullptr) {
        delete this->engine;
        this->engine = nullptr;
    }
}


void RendererGL::paint()
{
    if (!initialized) {
        init();
        initialized = true;
    }

    update();
    render();
   // testDraw();

}

Core::Engine& RendererGL::getEngine() {
    return *engine;
}

void RendererGL::init() {
    if (!engineInitialized) {
      engine = new Core::Engine(Core::Engine::GLVersion::Three);
      engine->init();
      engineInitialized = true;
    }

    if (!m_program) {
        initializeOpenGLFunctions();

        m_program = new QOpenGLShaderProgram();
        m_program->addCacheableShaderFromSourceCode(QOpenGLShader::Vertex,
                                                    "attribute highp vec4 vertices;"
                                                    "varying highp vec2 coords;"
                                                    "void main() {"
                                                    "    gl_Position = vertices;"
                                                    "    coords = vertices.xy;"
                                                    "}");
        m_program->addCacheableShaderFromSourceCode(QOpenGLShader::Fragment,
                                                    "uniform lowp float t;"
                                                    "varying highp vec2 coords;"
                                                    "void main() {"
                                                    "    lowp float i = 1. - (pow(abs(coords.x), 4.) + pow(abs(coords.y), 4.));"
                                                    "    i = smoothstep(t - 0.8, t + 0.8, i);"
                                                    "    i = floor(i * 20.) / 20.;"
                                                    "    gl_FragColor = vec4(coords * .5 + .5, i, i);"
                                                    "}");

        m_program->bindAttributeLocation("vertices", 0);
        m_program->link();

    }

    for(std::vector<std::function<void()>>::iterator itr = onInits.begin(); itr != onInits.end(); ++itr) {
        std::function<void()>& func = *itr;
        func();
    }
}

void RendererGL::onInit(std::function<void()>& func) {
    onInits.push_back(func);
}

void RendererGL::update() {
    engine->update();
}

void RendererGL::render() {
    engine->render();
}

void RendererGL::testDraw() {
    m_program->bind();
    m_program->enableAttributeArray(0);

    float values[] = {
        -1, -1,
        1, -1,
        -1, 1,
        1, 1
    };
    m_program->setAttributeArray(0, GL_FLOAT, values, 2);
    m_program->setUniformValue("t", (float) m_t);

    glViewport(0, 0, m_viewportSize.width(), m_viewportSize.height());

    glDisable(GL_DEPTH_TEST);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_program->disableAttributeArray(0);
    m_program->release();

    // Not strictly needed for this example, but generally useful for when
    // mixing with raw OpenGL.
    m_window->resetOpenGLState();
}

void RendererGL::setT(qreal t) {
    m_t = t;
}

void RendererGL::setViewportSize(const QSize &size) {
    if (m_viewportSize.width() != size.width() || m_viewportSize.height() != size.height() || !engineWindowSizeSet) {
        if (engine) {
            engine->resize(size.width(), size.height());
            engineWindowSizeSet = true;
        }
        m_viewportSize = size;
    }
}

void RendererGL::setWindow(QQuickWindow *window) {
    m_window = window;
}
