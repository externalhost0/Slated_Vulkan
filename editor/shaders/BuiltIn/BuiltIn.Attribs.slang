
// for editor editing of uniforms

// if we dont want to see it in editor properties panel
[__AttributeUsage(_AttributeTargets.Var)]
struct NoExposeAttribute {};

enum Blend : uint8_t {
    Off,
    Additive,
    AlphaBlend,
    Premultiply
}
[__AttributeUsage(_AttributeTargets.Function)]
struct BlendingAttribute
{
    Blend mode;
};