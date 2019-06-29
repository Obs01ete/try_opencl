__kernel void sim_step(
    __global float2* state,
    const unsigned int stateSize
    )
{
    float alpha = 1e-4f;
    const float2 repelent = (float2)(0.3f, 0.4f);
    size_t threadIdx = get_global_id(0);
    if (threadIdx < stateSize)
    {
        float2 position = state[threadIdx];
        float2 radiusVector = position - repelent;
        float len = length(radiusVector);
        float2 force = alpha * radiusVector / (len*len*len + 1e-6f);
        float saturationRate = 0.1f;
        float2 displacement = force * saturationRate;
        float2 newPosition = position + displacement;
        state[threadIdx] = newPosition;
        // state[threadIdx] = state[threadIdx] + 0.01f;
    }
}

__kernel void experimental(
    __global float* m_state,
    __global float* m_output,
    const unsigned int count)
{
    size_t threadIdx = get_global_id(0);
    if (threadIdx < count) {
        float acc = 0.0f;
        size_t j = 0;
        for (j = 0; j < count; j++) {
            if (j != threadIdx) {
                acc += m_state[j];
            }
        }
        m_output[threadIdx] = acc;
    }
}
