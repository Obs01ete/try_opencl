//
// Created by Dmitrii Khizbullin on 2019-06-28.
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

    cl_device_id m_deviceId;
    cl_context m_context;
    cl_command_queue m_commands;
    cl_program m_program;
    cl_kernel m_kernelSimStep;
    cl_mem m_state;
    size_t m_stateSize;

    std::vector<Point2f> m_result;

public:
    Engine();
    void process();
    std::vector<Point2f> getState();
    ~Engine();
};

#endif
