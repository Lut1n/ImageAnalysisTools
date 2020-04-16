#include "doubleThreshold.hpp"

#include <iostream>

// --------------------------------------------------------------------------
#define GLSL_CODE( src ) #src

// --------------------------------------------------------------------------
static const std::string s_glsl_vertex = GLSL_CODE(
    varying vec4 vertex;
    void main()
    {
        vertex = gl_ModelViewProjectionMatrix * gl_Vertex;
        gl_Position = vertex;
        gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
    }
);

// --------------------------------------------------------------------------
static const std::string s_glsl_2thresholds = GLSL_CODE(
    uniform sampler2D u_input;
    uniform float u_thMajor;
    uniform float u_thMinor;

    void main()
    {
        vec2 uv = gl_TexCoord[0].xy;
        uv.y = 1.0 - uv.y;

        vec3 color = texture2D(u_input, uv).xyz;

        float b = 0.0;
        if(color.r>u_thMinor) b = (color.r>u_thMajor) ? 1.0 : 0.5;

        vec3 thresholded = vec3(b);
        gl_FragColor = vec4(thresholded,1.0);
    }
);



// --------------------------------------------------------------------------
DoubleThreshold::DoubleThreshold()
{
    initialize();
}

// --------------------------------------------------------------------------
DoubleThreshold::~DoubleThreshold()
{
    cleanup();
}

// --------------------------------------------------------------------------
void DoubleThreshold::initialize()
{
    m_vertexBuffer = sf::VertexBuffer(sf::Quads, sf::VertexBuffer::Static);
    m_vertexBuffer.create(4);

    if (!m_2thresholdShader.loadFromMemory(s_glsl_vertex, s_glsl_2thresholds))
    {
        std::cout << "err with thresholding shader..." << std::endl;
    }
}

// --------------------------------------------------------------------------
void DoubleThreshold::cleanup()
{
}

// --------------------------------------------------------------------------
void DoubleThreshold::resizeRenderTarget(const sf::Vector2u& size)
{
    m_target.create(size.x,size.y);

    sf::Vertex vertices[] =
    {
        sf::Vertex(sf::Vector2f(     0,      0), sf::Color::White, sf::Vector2f(0,0)),
        sf::Vertex(sf::Vector2f(     0, size.y), sf::Color::White, sf::Vector2f(0,1)),
        sf::Vertex(sf::Vector2f(size.x, size.y), sf::Color::White, sf::Vector2f(1,1)),
        sf::Vertex(sf::Vector2f(size.x,      0), sf::Color::White, sf::Vector2f(1,0))
    };
    m_vertexBuffer.update(vertices);
}

// --------------------------------------------------------------------------
const sf::Texture& DoubleThreshold::apply(const sf::Texture &texture, float thresholdMajor, float thresholdMinor)
{
    sf::Vector2u currSize = m_target.getSize();
    sf::Vector2u size = texture.getSize();
    if(currSize != size)
    {
        resizeRenderTarget(size);
        currSize = size;
    }

    m_2thresholdShader.setUniform("u_input", texture);
    m_2thresholdShader.setUniform("u_thMajor", thresholdMajor);
    m_2thresholdShader.setUniform("u_thMinor", thresholdMinor);

    m_target.clear();
    m_target.draw(m_vertexBuffer, &m_2thresholdShader);

    return m_target.getTexture();
}

// --------------------------------------------------------------------------
const sf::Texture& DoubleThreshold::getResultAsTexture()
{
    return m_target.getTexture();
}
