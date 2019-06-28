//
// Created by Dmitry on 2019-06-28.
//

#ifndef ENGINE_H
#define ENGINE_H

#define CL_SILENCE_DEPRECATION
#include <OpenCL/opencl.h>

#import <vector>

struct Point2f
{
    float x;
    float y;
};

class Engine
{
private:
    bool m_initialized;

    cl_device_id device_id;
    cl_context context;
    cl_command_queue commands;
    cl_program program;
    cl_kernel kernel;
    cl_mem input;
    cl_mem output;

    std::vector<Point2f> m_result;

public:
    Engine();
    void process();
    std::vector<Point2f> getState();
    ~Engine();
};

#endif
