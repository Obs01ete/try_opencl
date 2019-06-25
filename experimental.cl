__kernel void experimental(
    __global float* input,
    __global float* output,
    const unsigned int count)
{
    size_t threadIdx = get_global_id(0);
    if (threadIdx < count) {
        float acc = 0.0f;
        size_t j = 0;
        for (j = 0; j < count; j++) {
            if (j != threadIdx) {
                acc += input[j];
            }
        }
        output[threadIdx] = acc;
    }
}
