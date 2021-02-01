//some common geometrys for vertex shaders
//so me don't have to construct a vertex array out side


#ifdef USE_QUADS
    static const float4 quads[6] =
    {
        float4(-1.0f, -1.0f, 0.f,  1.f),
        float4( 1.0f,  1.0f, 0.f,  1.f),
        float4( 1.0f, -1.0f, 0.f,  1.f),
        float4(-1.0f,  1.0f, 0.f,  1.f),
        float4(-1.0f, -1.0f, 0.f,  1.f),
        float4( 1.0f,  1.0f, 0.f,  1.f)
    };
#endif