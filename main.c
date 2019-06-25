#define CL_SILENCE_DEPRECATION

#include <stdio.h>
#include <OpenCL/opencl.h>

#define DATA_SIZE (1024)

const char *KernelSource =
  "__kernel void square(__global float* input, __global float* output, const unsigned int count) { \n" \
  "   int i = get_global_id(0);                                                                    \n" \
  "   if(i < count) { output[i] = input[i] * input[i]; }                                           \n" \
  "}";

int main(void) {
  int err;
  cl_device_id device_id;
  //clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
  clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);
  cl_context context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
  cl_command_queue commands = clCreateCommandQueue(context, device_id, 0, &err);
  cl_program program = clCreateProgramWithSource(context, 1, (const char **) & KernelSource, NULL, &err);
  clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  cl_kernel kernel = clCreateKernel(program, "square", &err);
  cl_mem input = clCreateBuffer(context,  CL_MEM_READ_ONLY,  sizeof(float) * DATA_SIZE, NULL, NULL);
  cl_mem output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * DATA_SIZE, NULL, NULL);
  float data[DATA_SIZE];
  for (int i = 0; i < DATA_SIZE; i++) { data[i] = i; }
  err = clEnqueueWriteBuffer(commands, input, CL_TRUE, 0, sizeof(float) * DATA_SIZE, data, 0, NULL, NULL);
  clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
  clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
  unsigned int count = DATA_SIZE;
  clSetKernelArg(kernel, 2, sizeof(unsigned int), &count);
  size_t local;
  clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
  size_t global = count;
  clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
  clFinish(commands);
  float results[DATA_SIZE];
  clEnqueueReadBuffer(commands, output, CL_TRUE, 0, sizeof(float) * count, results, 0, NULL, NULL);
  unsigned int correct = 0;
  for (int i = 0; i < count; i++) {
      if (results[i] == data[i] * data[i]) { correct++; }
  }
  printf("Computed '%d/%d' correct values!\n", correct, count);
  clReleaseMemObject(input);
  clReleaseMemObject(output);
  clReleaseProgram(program);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(commands);
  clReleaseContext(context);
  return 0;
}
