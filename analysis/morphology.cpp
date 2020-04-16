#include "morphology.hpp"

#include "filtering.hpp"

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
static const std::string s_glsl_morpho = GLSL_CODE(
    uniform sampler2D u_src;
    uniform float u_matrix[81]; // max = 9x9
    uniform vec2 u_srcsize;
    uniform vec2 u_matrixsize;
    uniform int u_optype;

    float mat_coef(vec2 st)
    {
        int i = int(st.x * u_matrixsize.y + st.y);
        return u_matrix[i];
    }

    vec3 src_value(vec2 uv, vec2 oft)
    {
        vec2 sample_oft = oft - u_matrixsize*0.5;
        vec2 sample_uv = uv + sample_oft/u_srcsize;
        return texture2D(u_src, sample_uv).xyz;
    }

    void main()
    {
        vec2 uv = gl_TexCoord[0].xy;
        uv.y = 1.0 - uv.y;
        vec3 acc = (u_optype==1) ? vec3(1.0) : vec3(0.0);

        for(float x=0.0;x<u_matrixsize.x;++x)
        {
            for(float y=0.0;y<u_matrixsize.y;++y)
            {
                vec2 st = vec2(x,y);
                float coef = mat_coef(st);
                vec3 r = src_value(uv, st) * coef;
                if(coef != 0.0) acc = (u_optype==1) ? min(acc,r) : max(acc,r);
            }
        }

        gl_FragColor = vec4(acc, 1.0);
    }
);


//--------------------------------------------------------------
Morphology::Morphology(MorphType t)
    : _type(t)
{
    initialize();
}

//--------------------------------------------------------------
Morphology::Morphology(const Matrix& mat, MorphType t)
    : _type(t)
{
    initialize();
    setMatrix(mat);
}

//--------------------------------------------------------------
Morphology::~Morphology()
{
    cleanup();
}

//--------------------------------------------------------------
void Morphology::initialize()
{
    _area = sf::VertexBuffer(sf::Quads, sf::VertexBuffer::Static);
    _area.create(4);

    if (!_shader.loadFromMemory(s_glsl_vertex, s_glsl_morpho))
    {
        std::cout << "err with morphology shader..." << std::endl;
    }
}

//--------------------------------------------------------------
void Morphology::cleanup()
{
}

//--------------------------------------------------------------
void Morphology::setMatrix(const Matrix& mat)
{
    _matrix = mat;
}

//--------------------------------------------------------------
const sf::Texture& Morphology::apply(const sf::Texture& src)
{
    if(_target.getSize() != src.getSize()) resize(src.getSize());

    if(_matrix.valid())
    {
        sf::Vector2f srcsize(src.getSize().x, src.getSize().y);
        sf::Vector2f matsize(_matrix.rowSize(),_matrix.colSize());

        _shader.setUniform("u_src", src);
        _shader.setUniform("u_srcsize", srcsize);
        _shader.setUniformArray("u_matrix", _matrix.data(), _matrix.size());
        _shader.setUniform("u_matrixsize", matsize);

        if(_type==Dilation) _shader.setUniform("u_optype", 0);
        if(_type==Erosion) _shader.setUniform("u_optype", 1);

        _target.draw(_area, &_shader);
    }

    return texture();
}

//--------------------------------------------------------------
const sf::Texture& Morphology::texture() const
{
    return _target.getTexture();
}

//--------------------------------------------------------------
void Morphology::resize(const sf::Vector2u& size)
{
    _target.create(size.x,size.y);
    _subtarget.create(size.x,size.y);

    sf::Vertex vertices[] =
    {
        sf::Vertex(sf::Vector2f(     0,      0), sf::Color::White, sf::Vector2f(0,0)),
        sf::Vertex(sf::Vector2f(     0, size.y), sf::Color::White, sf::Vector2f(0,1)),
        sf::Vertex(sf::Vector2f(size.x, size.y), sf::Color::White, sf::Vector2f(1,1)),
        sf::Vertex(sf::Vector2f(size.x,      0), sf::Color::White, sf::Vector2f(1,0))
    };
    _area.update(vertices);
}



//--------------------------------------------------------------
Cross3x3Morpho::Cross3x3Morpho(MorphType t)
    : Morphology(t)
{
    Matrix kernel(3,3);
    kernel(0,0) = 0.0; kernel(1,0) = 1.0; kernel(2,0) = 0.0;
    kernel(0,1) = 1.0; kernel(1,1) = 1.0; kernel(2,1) = 1.0;
    kernel(0,2) = 0.0; kernel(1,2) = 1.0; kernel(2,2) = 1.0;

    setMatrix(kernel);
}


//--------------------------------------------------------------
Square3x3Morpho::Square3x3Morpho(MorphType t)
    : Morphology(t)
{
    Matrix kernel(3,3);
    kernel(0,0) = 1.0; kernel(1,0) = 1.0; kernel(2,0) = 1.0;
    kernel(0,1) = 1.0; kernel(1,1) = 1.0; kernel(2,1) = 1.0;
    kernel(0,2) = 1.0; kernel(1,2) = 1.0; kernel(2,2) = 1.0;

    setMatrix(kernel);
}
