#include "engine.h"

#include <stdio.h>
#include <iostream>
#include <fstream>



#define DATA_SIZE (1024)


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
    device_id = 0;

    clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
    // clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
    commands = clCreateCommandQueue(context, device_id, 0, &err);
    std::vector<char> kernel_text = load_kernel("experimental.cl");
    if (kernel_text.size() == 0)
    {
        return;
    }
    const char* kernel_text_data = kernel_text.data();
    program = clCreateProgramWithSource(context, 1, &kernel_text_data, NULL, &err);
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    printf("clBuildProgram ret = %d, (CL_BUILD_PROGRAM_FAILURE=%d)\n", err, CL_BUILD_PROGRAM_FAILURE);
    if (true || err != CL_SUCCESS)
    {
        char* build_log = nullptr;
        size_t ret_val_size = 0;
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &ret_val_size);
        build_log = new char[ret_val_size+1];
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, ret_val_size, build_log, NULL);
        build_log[ret_val_size] = '\0';
        std::cout << build_log << std::endl;
        delete[] build_log;
    }
    kernel = clCreateKernel(program, "experimental", &err);
    input = clCreateBuffer(context,  CL_MEM_READ_ONLY,  sizeof(float) * DATA_SIZE, NULL, NULL);
    output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * DATA_SIZE, NULL, NULL);

    m_initialized = true;
}

void Engine::process()
{
    if (!m_initialized)
    {
        return;
    }

    std::cout << "Engine processing frame" << std::endl;

    float data[DATA_SIZE];
    for (int i = 0; i < DATA_SIZE; i++) { data[i] = i; }
    int err = clEnqueueWriteBuffer(commands, input, CL_TRUE, 0, sizeof(float) * DATA_SIZE, data, 0, NULL, NULL);
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
    unsigned int count = DATA_SIZE;
    clSetKernelArg(kernel, 2, sizeof(unsigned int), &count);
    size_t local = 0;
    clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
    size_t global = count;
    clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
    clFinish(commands);
    float results[DATA_SIZE];
    clEnqueueReadBuffer(commands, output, CL_TRUE, 0, sizeof(float) * count, results, 0, NULL, NULL);
    unsigned int correct = 0;
    double total = 0.0;
    for (int i = 0; i < count; i++) {
        total += data[i];
    }
    for (int i = 0; i < count; i++) {
        float reference = (float)(total - data[i]);
        //printf("%f %f\n", results[i], reference);
        if (results[i] == reference) { correct++; }
    }
    printf("Computed '%d/%d' correct values!\n", correct, count);

    m_result.reserve(count);
    for (const auto& v : results)
    {
        m_result.push_back({v, v});
    }
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
        clReleaseMemObject(input);
        clReleaseMemObject(output);
        clReleaseProgram(program);
        clReleaseKernel(kernel);
        clReleaseCommandQueue(commands);
        clReleaseContext(context);
    }
}

