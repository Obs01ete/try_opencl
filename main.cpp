#include "engine.h"
#include "renderer.h"


Engine g_engine;
Renderer g_renderer(&g_engine);


int main(int argc, char** argv) {

    g_renderer.run_glut();

    return 0;
}
