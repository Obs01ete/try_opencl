//
// Created by Dmitrii Khizbullin on 2019-06-28.
//

#ifndef ENGINE_H
#define ENGINE_H

#import <vector>

#define CL_SILENCE_DEPRECATION
#include <OpenCL/opencl.h>

#include "png_wrapper.h"


struct Point2f
{
    float x;
    float y;
};

class Engine
{
private:
    bool m_initialized;

    cl_device_id m_deviceId;
    cl_context m_context;
    cl_command_queue m_commands;
    cl_program m_program;
    cl_kernel m_kernelSimStep;

    size_t m_positionsSize;
    cl_mem m_positions; // size m_positionsSize elems
    cl_mem m_anchors; // size m_positionsSize elems

    std::vector<Point2f> m_result;

    std::unique_ptr<cl_float2> m_repelent;

public:
    Engine();
    void process();
    const std::vector<Point2f>& getState();
    ~Engine();

    void setRepelentCoords(bool exists, float x = 0.0f, float y = 0.0f);
};

#endif
