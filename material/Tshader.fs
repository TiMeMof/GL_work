#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2D texture0;

// 可控透明度：通过 uniform alpha 来调整贴图的最终 alpha
uniform float alpha;

void main()
{
    vec4 tex = texture(texture0, TexCoord);
    // 保留纹理本身的 alpha，同时乘以 uniform alpha（便于强制半透明）
    FragColor = vec4(tex.rgb, tex.a * alpha);
}