__kernel void sim_step(
    __global float2* positions,
    __global float2* anchors,
    const unsigned int positionsSize,
    int repelentExists,
    float2 repelent
    )
{
    float alpha = 1e-1f;
    float beta = 5e-3f;
    float saturationRate = 0.25f;
    float smoothnessFactor = 5e-4f;

    size_t threadIdx = get_global_id(0);

    if (threadIdx < positionsSize)
    {
        float2 position = positions[threadIdx];
        float2 anchor = anchors[threadIdx];

        float2 repelForce = (float2)(0, 0);
        if (repelentExists != 0)
        {
            float2 repelVector = position - repelent;
            float repelLen = length(repelVector);
            repelForce = beta * repelVector /
                (repelLen*repelLen + smoothnessFactor);
        }

        float angle = -1.4f; // radian
        repelForce = (float2)(
            repelForce.x*cos(angle) - repelForce.y*sin(angle),
            repelForce.x*sin(angle) + repelForce.y*cos(angle)
         );

        float2 attractVector = position - anchor;
        float2 attractForce = - alpha * attractVector;

        float2 totalForce = repelForce + attractForce;

        float2 displacement = totalForce * saturationRate;
        float2 newPosition = position + displacement;
        positions[threadIdx] = newPosition;
    }
}
