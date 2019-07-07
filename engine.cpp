//
// Created by Dmitrii Khizbullin on 2019-06-28.
// Inspired by the home page of Neurodata Lab
// https://neurodatalab.com/#home
//

#include "engine.h"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include "timer.h"



template<typename T>
size_t sizeBytes(const std::vector<T>& v)
{
    return sizeof(T) * v.size();
}


std::vector<char> load_kernel(const std::string& name)
{
    std::ifstream file(name, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    if (size < 0)
    {
        std::cout << "Kernel source file not found" << std::endl;
        return std::vector<char>();
    }
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (file.read(buffer.data(), size))
    {
        return buffer;
    }
    else
    {
        return std::vector<char>();
    }
}


Engine::Engine()
{
    m_initialized = false;

    int err = 0;
    m_deviceId = 0;

    clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &m_deviceId, NULL);
    // clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU, 1, &m_deviceId, NULL);
    m_context = clCreateContext(0, 1, &m_deviceId, NULL, NULL, &err);
    cl_command_queue_properties props = CL_QUEUE_PROFILING_ENABLE;
    m_commands = clCreateCommandQueue(m_context, m_deviceId, props, &err);
    std::vector<char> kernelTextArray = load_kernel("experimental.cl");
    if (kernelTextArray.size() == 0)
    {
        return;
    }
    std::string kernel_text(kernelTextArray.data(), kernelTextArray.size());
    const char* kernel_text_data = kernel_text.c_str();
    m_program = clCreateProgramWithSource(m_context, 1, &kernel_text_data, NULL, &err);
    err = clBuildProgram(m_program, 0, NULL, NULL, NULL, NULL);
    printf("clBuildProgram ret = %d, (CL_BUILD_PROGRAM_FAILURE=%d)\n", err, CL_BUILD_PROGRAM_FAILURE);
    if (true || err != CL_SUCCESS)
    {
        char* build_log = nullptr;
        size_t ret_val_size = 0;
        clGetProgramBuildInfo(m_program, m_deviceId, CL_PROGRAM_BUILD_LOG, 0, NULL, &ret_val_size);
        build_log = new char[ret_val_size+1];
        clGetProgramBuildInfo(m_program, m_deviceId, CL_PROGRAM_BUILD_LOG, ret_val_size, build_log, NULL);
        build_log[ret_val_size] = '\0';
        std::cout << build_log << std::endl;
        delete[] build_log;
    }
    if (err != CL_SUCCESS)
    {
        return;
    }
    m_kernelSimStep = clCreateKernel(m_program, "sim_step", &err);

    int n = 16;
    const int numRows = 32*n;
    const int numCols = 32*n;

    Png png;
    png.read_png_file("bobmarley512g.png");
    std::vector<unsigned char> image = png.process_file(numCols, numRows);

    const int numPoints = numRows * numCols;
    std::vector<cl_float2> initState;
    initState.reserve(numPoints);
    for (size_t i = 0; i < numPoints; i++)
    {
        auto row = i / numCols;
        auto col = i % numCols;
        auto point = cl_float2 {
            (float)col/numCols,
            (float)row/numRows
        };
        bool mask = image[i] != 0;
        if (!mask)
        {
            point = cl_float2 {0, 0};
        }
        if (true)
        {
            float gap = 0.05f;
            point = cl_float2 {
                point.x * (1.0f + 2*gap) - gap,
                point.y * (1.0f + 2*gap) - gap
            };
        }
        initState.push_back(point);
    }
    m_positionsSize = numPoints;
    std::cout << sizeBytes(initState) << std::endl;
    m_positions = clCreateBuffer(m_context, CL_MEM_READ_WRITE, sizeBytes(initState), NULL, NULL);
    err = clEnqueueWriteBuffer(m_commands, m_positions, CL_TRUE, 0, sizeBytes(initState), initState.data(), 0, NULL, NULL);
    m_anchors = clCreateBuffer(m_context, CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, sizeBytes(initState), NULL, NULL);
    err = clEnqueueWriteBuffer(m_commands, m_anchors, CL_TRUE, 0, sizeBytes(initState), initState.data(), 0, NULL, NULL);
    clFinish(m_commands);

    m_repelent = nullptr;

    m_initialized = true;
}

void printEventDuration(const cl_event& event)
{
    cl_ulong start, end;
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END,
                            sizeof(cl_ulong), &end, NULL);
    clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START,
                            sizeof(cl_ulong), &start, NULL);
    float executionTimeInMilliseconds = (end - start) * 1.0e-6f;
    std::cout << executionTimeInMilliseconds << std::endl;
}

void Engine::process()
{
    if (!m_initialized)
    {
        return;
    }

    //std::cout << "Engine processing frame (num_particles=" << m_positionsSize << ")" << std::endl;

    if (m_result.size() != m_positionsSize)
    {
        m_result = std::vector<Point2f>(m_positionsSize, Point2f{});
    }


    Timer timerTotal;

    Timer timerKernel;
    clSetKernelArg(m_kernelSimStep, 0, sizeof(cl_mem), &m_positions);
    clSetKernelArg(m_kernelSimStep, 1, sizeof(cl_mem), &m_anchors);
    size_t count = m_positionsSize;
    clSetKernelArg(m_kernelSimStep, 2, sizeof(count), &count);
    cl_int repelentExists = (cl_int)((bool)m_repelent);
    clSetKernelArg(m_kernelSimStep, 3, sizeof(cl_int), &repelentExists);
    cl_float2 repelent = ((bool)m_repelent) ? *m_repelent : cl_float2{0, 0};
    clSetKernelArg(m_kernelSimStep, 4, sizeof(cl_float2), &repelent);
    size_t local = 0;
    clGetKernelWorkGroupInfo(m_kernelSimStep, m_deviceId, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
    size_t global = count;
    cl_event eventSimStep{};
    clEnqueueNDRangeKernel(m_commands, m_kernelSimStep, 1, NULL, &global, &local, 0, NULL, &eventSimStep);
    //clFinish(m_commands);
    //timerKernel.print("Kernel time");
    Timer timerReadout;
    cl_event eventReadPositions{};
    clEnqueueReadBuffer(m_commands, m_positions, CL_TRUE, 0, sizeBytes(m_result), m_result.data(), 0, NULL, &eventReadPositions);
    //clFinish(m_commands);
    //timerReadout.print("Readout");

    timerTotal.print("Engine processing time");

    printEventDuration(eventSimStep);
    printEventDuration(eventReadPositions);

    // std::cout << "Sample point: " << m_result[33].x << " " << m_result[33].y << std::endl;
}

const std::vector<Point2f>& Engine::getState()
{
    return m_result;
}

Engine::~Engine()
{
    if (m_initialized)
    {
        clReleaseMemObject(m_positions);
        clReleaseProgram(m_program);
        clReleaseKernel(m_kernelSimStep);
        clReleaseCommandQueue(m_commands);
        clReleaseContext(m_context);
    }
}

void Engine::setRepelentCoords(bool exists, float x, float y)
{
    if (exists)
    {
        m_repelent = std::make_unique<cl_float2>(cl_float2{x, y});
    }
    else
    {
        m_repelent = nullptr;
    }
}

