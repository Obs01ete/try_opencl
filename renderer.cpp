//
// Created by Dmitrii Khizbullin on 2019-06-29.
// Based on http://photonherder.blogspot.com/2011/09/tutorial-simple-opengl-program-in-xcode.html
//

#include "renderer.h"

#include <cmath>
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>

#include "timer.h"


const int FRAME_INTERVAL_MS = 10; // 100


Engine* Renderer::m_engine = nullptr;
int Renderer::m_cnt = 128;
ReentryTimer Renderer::m_fpsTimer;


Renderer::Renderer(Engine* engine)
{
    m_engine = engine;
}

int Renderer::run_glut() {
    int argc = 0;
    char *argv[1] = {(char *) ""};
    glutInit(&argc, argv);

    glutInitWindowSize(1024, 1024);

    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

    glutCreateWindow("Rubber World");

    glutDisplayFunc(display);

    glutReshapeFunc(reshape);

    glutTimerFunc(FRAME_INTERVAL_MS, Loop, 0);

    glutPassiveMotionFunc(mouse);
    glutEntryFunc(entry);

    glClearColor(0, 0, 0, 1);

    glutMainLoop();

    return 0;
}


void Renderer::drawPoints(const std::vector<Point2f>& points)
{
    int frameWidth = glutGet(GLUT_WINDOW_WIDTH);
    int frameHeight = glutGet(GLUT_WINDOW_HEIGHT);

    glBegin(GL_POINTS);
    for (const auto& p : points)
    {
        glVertex2f(p.x*frameWidth, p.y*frameHeight);
    }
    glEnd();
}


void Renderer::drawCircles(const std::vector<Point2f>& points)
{
    int num_vertices = 8;
    float r = 2.0f;

    int frameWidth = glutGet(GLUT_WINDOW_WIDTH);
    int frameHeight = glutGet(GLUT_WINDOW_HEIGHT);

    for (int ip = 0; ip < points.size(); ip++)
    {
        const auto& p = points[ip];
        if (ip % 133 != 0) { continue; }
        glBegin(GL_POLYGON);
        for (int i = 0; i < num_vertices; i++)
        {
            auto cx = p.x*frameWidth;
            auto cy = p.y*frameHeight;
            glVertex2f(
                    cx + r*cosf(2*(float)M_PI*i/num_vertices),
                    cy + r*sinf(2*(float)M_PI*i/num_vertices));
        }
        glEnd();
    }
}


void Renderer::display() {
    /* clear the color buffer (resets everything to black) */
    glClear(GL_COLOR_BUFFER_BIT);

    /* set the current drawing color to red */
    glColor3f(m_cnt / 255.0f, 0, 0);

    /* start drawing triangles, each triangle takes 3 vertices */
    glBegin(GL_TRIANGLES);

    /* give the 3 triangle vertex coordinates 1 at a time */
    glVertex2f(10, 10);
    glVertex2f(30, 10);
    glVertex2f(20, 25);

    /* tell OpenGL we're done drawing triangles */
    glEnd();

    if (m_engine)
    {
        // fetch rendered image
        const std::vector<Point2f>& points = m_engine->getState();
        // and draw it with opengl
        glColor3f(1, 1, 1);
        drawCircles(points);
    }

    glutSwapBuffers();

    m_cnt = (m_cnt+1) % 256;

    m_fpsTimer.print("Frame interval");
}

/*! glut reshape callback function.  GLUT calls this function whenever
 the window is resized, including the first time it is created.
 You can use variables to keep track the current window size.
 */
void Renderer::reshape(int width, int height) {
    /* tell OpenGL we want to display in a recangle that is the
     same size as the window
     */
    glViewport(0, 0, width, height);

    /* switch to the projection matrix */
    glMatrixMode(GL_PROJECTION);

    /* clear the projection matrix */
    glLoadIdentity();

    /* set the camera view, orthographic projection in 2D */
    gluOrtho2D(0, width, 0, height);

    /* switch back to the model view matrix */
    glMatrixMode(GL_MODELVIEW);
}

void Renderer::Loop(int v) {
    // Update the positions of the objects
    // ......
    // ......

    if (m_engine)
    {
        m_engine->process();
    }

    glutPostRedisplay();
    glutTimerFunc(FRAME_INTERVAL_MS, Loop, 0);
}

void Renderer::mouse(int x, int y)
{
    int frameWidth = glutGet(GLUT_WINDOW_WIDTH);
    int frameHeight = glutGet(GLUT_WINDOW_HEIGHT);

    if ((x < 0) || (y < 0) || (x >= frameWidth) || (y >= frameHeight))
    {
        return;
    }

    m_engine->setRepelentCoords(
            true,
            (float)x/frameWidth,
            (float)(frameHeight-y)/frameHeight);
}

void Renderer::entry(int state)
{
    if (state == GLUT_LEFT)
    {
        m_engine->setRepelentCoords(false);
    }
}