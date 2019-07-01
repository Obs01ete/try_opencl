//
// Created by Dmitrii Khizbullin on 2019-06-29.
//

#ifndef TRY_OPENCL_RENDERER_H
#define TRY_OPENCL_RENDERER_H

#include "engine.h"
#include "timer.h"


class Renderer {

public:
    static Engine* m_engine;
    static int m_cnt;
    static ReentryTimer m_fpsTimer;

public:
    Renderer(Engine* engine);

    static int run_glut();

private:

    /*! glut display callback function.  Every time the window needs to be drawn,
     glut will call this function.  This includes when the window size
     changes, or when another window covering part of this window is
     moved so this window is uncovered.
     */
    static void display();

    /*! glut reshape callback function.  GLUT calls this function whenever
     the window is resized, including the first time it is created.
     You can use variables to keep track the current window size.
     */
    static void reshape(int width, int height);

    static void Loop(int v);

    static void drawPoints(const std::vector<Point2f>& points);

    static void drawCircles(const std::vector<Point2f>& points);

    static void mouse(int x, int y);

    static void entry(int state);
};

#endif //TRY_OPENCL_RENDERER_H
