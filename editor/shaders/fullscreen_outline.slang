import "BuiltIn.Common";

struct v2f {
    float4 ClipPos : SV_Position;
    float2 UV      : TEXCOORD0;
};

struct PushConstant {
    uint imageTexId;
    uint maskTexId;
}
[[vk::push_constant]]
PushConstant push;

[shader("vertex")]
v2f vs_main(uint VertexID : SV_VertexID) {
    v2f output;
    // generates the fullscreen triangle without any cpu interaction
    // https://wallisc.github.io/rendering/2021/04/18/Fullscreen-Pass.html

    output.UV = float2((VertexID << 1) & 2, VertexID & 2 );
    output.ClipPos = float4(output.UV * 2.0 + -1.0, 0.0, 1.0);
    output.UV.y = 1.0 - output.UV.y;
    return output;
}

struct FSOutput {
    float4 FragColor : SV_Target0;
};

static constexpr float LINE_WEIGHT = 3.5f;
static constexpr float3 LINE_COLOR = float3(1.f, 0.f, 0.f);

[shader("pixel")]
FSOutput fs_main(v2f input) {
    FSOutput output;
    
    float dx = (1.0 / perFrame.resolution.x) * LINE_WEIGHT;
    float dy = (1.0 / perFrame.resolution.y) * LINE_WEIGHT;
 
    float2 uvCenter   = input.UV;
    float2 uvRight    = float2(uvCenter.x + dx, uvCenter.y);
    float2 uvTop      = float2(uvCenter.x, uvCenter.y - dx);
    float2 uvTopRight = float2(uvCenter.x + dx, uvCenter.y - dx);
 
    float mCenter   = textureBindless2D(push.maskTexId, 0, uvCenter).r;
    float mTop      = textureBindless2D(push.maskTexId, 0, uvTop).r;
    float mRight    = textureBindless2D(push.maskTexId, 0, uvRight).r;
    float mTopRight = textureBindless2D(push.maskTexId, 0, uvTopRight).r;
    
    float dT  = abs(mCenter - mTop);
    float dR  = abs(mCenter - mRight);
    float dTR = abs(mCenter - mTopRight);
    
    float delta = 0.0;
    delta = max(delta, dT);
    delta = max(delta, dR);
    delta = max(delta, dTR);
 

    float4 scene = textureBindless2D(push.imageTexId, 0, input.UV);
    float3 final = lerp(scene.rgb, LINE_COLOR, delta).rgb;

    output.FragColor = float4(final, 1);
    return output;
}