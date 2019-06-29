//
// Created by Dmitrii Khizbullin on 2019-06-29.
// Based on http://photonherder.blogspot.com/2011/09/tutorial-simple-opengl-program-in-xcode.html
//

#include "renderer.h"

#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>


const int FRAME_INTERVAL_MS = 10; // 100


Renderer::Renderer(Engine* engine)
{
    m_engine = engine;
}

int Renderer::run_glut() {
    int argc = 0;
    char *argv[1] = {(char *) ""};
    glutInit(&argc, argv);

    /* set the window size to 512 x 512 */
    glutInitWindowSize(512, 512);

    /* set the display mode to Red, Green, Blue and Alpha
     allocate a depth buffer
     enable double buffering
     */
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

    /* create the window (and call it Lab 1) */
    glutCreateWindow("Lab 1");

    /* set the glut display callback function
     this is the function GLUT will call every time
     the window needs to be drawn
     */
    glutDisplayFunc(display);

    /* set the glut reshape callback function
     this is the function GLUT will call whenever
     the window is resized, including when it is
     first created
     */
    glutReshapeFunc(reshape);

    glutTimerFunc(FRAME_INTERVAL_MS, Loop, 0);

    /* set the default background color to black */
    glClearColor(0, 0, 0, 1);

    /* enter the main event loop so that GLUT can process
     all of the window event messages
     */
    glutMainLoop();

    return 0;
}

/*! glut display callback function.  Every time the window needs to be drawn,
 glut will call this function.  This includes when the window size
 changes, or when another window covering part of this window is
 moved so this window is uncovered.
 */
void Renderer::display() {
    static int cnt = 128;
    /* clear the color buffer (resets everything to black) */
    glClear(GL_COLOR_BUFFER_BIT);

    /* set the current drawing color to red */
    glColor3f((cnt % 255) / 255.0f, 0, 0);

    /* start drawing triangles, each triangle takes 3 vertices */
    glBegin(GL_TRIANGLES);

    /* give the 3 triangle vertex coordinates 1 at a time */
    glVertex2f(10, 10);
    glVertex2f(250, 400);
    glVertex2f(400, 10);

    /* tell OpenGL we're done drawing triangles */
    glEnd();

    int frameWidth = glutGet(GLUT_WINDOW_WIDTH);
    int frameHeight = glutGet(GLUT_WINDOW_HEIGHT);

    if (m_engine)
    {
        // fetch rendered image
        std::vector<Point2f> points = m_engine->getState();
        int debug = 0;
        // and draw it with opengl
        glColor3f(1, 1, 1);
        glBegin(GL_POINTS);
        for (const auto& p : points)
        {
            glVertex2f(p.x*frameWidth, p.y*frameHeight);
        }
        glEnd();
    }

    /* swap the back and front buffers so we can see what we just drew */
    glutSwapBuffers();

    //    if (cnt == 200)
    //    {
    //        exit(0);
    //    }

    cnt++;
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
