//
// Created by Dmitrii Khizbullin on 2019-06-28.
//

#include "engine.h"

#include <stdio.h>
#include <iostream>
#include <fstream>


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
    m_commands = clCreateCommandQueue(m_context, m_deviceId, 0, &err);
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

    int n = 4;
    const int numRows = 32*n;
    const int numCols = 32*n;
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
        if (true)
        {
            point = cl_float2 {
                point.x * 1.5f - 0.25f,
                point.y * 1.5f - 0.25f
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

void Engine::process()
{
    if (!m_initialized)
    {
        return;
    }

    std::cout << "Engine processing frame" << std::endl;

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
    clEnqueueNDRangeKernel(m_commands, m_kernelSimStep, 1, NULL, &global, &local, 0, NULL, NULL);
    clFinish(m_commands);
    m_result.clear();
    m_result.resize(m_positionsSize);
    clEnqueueReadBuffer(m_commands, m_positions, CL_TRUE, 0, sizeBytes(m_result), m_result.data(), 0, NULL, NULL);
    clFinish(m_commands);
    std::cout << m_result[33].x << " " << m_result[33].y << std::endl;
}

std::vector<Point2f> Engine::getState()
{
    if (!m_initialized)
    {
        return std::vector<Point2f>();
    }
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

