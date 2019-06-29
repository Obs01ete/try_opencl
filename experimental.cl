__kernel void sim_step(
    __global float2* positions,
    __global float2* anchors,
    const unsigned int positionsSize,
    float2 repelent
    )
{
    float alpha = 1e-1f;
    float beta = 1e-3f;
    float saturationRate = 0.2f;
    float smoothnessFactor = 5e-4f;

    size_t threadIdx = get_global_id(0);

    if (threadIdx < positionsSize)
    {
        float2 position = positions[threadIdx];
        float2 anchor = anchors[threadIdx];

        float2 repelVector = position - repelent;
        float repelLen = length(repelVector);
        float2 repelForce = beta * repelVector /
            (repelLen*repelLen*repelLen + smoothnessFactor);

        float2 attractVector = position - anchor;
        float2 attractForce = - alpha * attractVector;

        float2 totalForce = repelForce + attractForce;

        float2 displacement = totalForce * saturationRate;
        float2 newPosition = position + displacement;
        positions[threadIdx] = newPosition;
    }
}
