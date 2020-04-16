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
static const std::string s_glsl_filter = GLSL_CODE(
    uniform sampler2D u_src;
    uniform float u_matrix[81]; // max = 9x9
    uniform vec2 u_srcsize;
    uniform vec2 u_matrixsize;

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
        vec3 acc = vec3(0.0);

        for(float x=0.0;x<u_matrixsize.x;++x)
        {
            for(float y=0.0;y<u_matrixsize.y;++y)
            {
                vec2 st = vec2(x,y);
                acc += src_value(uv, st) * mat_coef(st);
            }
        }

        gl_FragColor = vec4(acc, 1.0);
    }
);

//--------------------------------------------------------------
Matrix::Matrix()
    : _rowsize(0)
    , _colsize(0)
{
}

//--------------------------------------------------------------
Matrix::Matrix(unsigned int rowsize, unsigned int colsize)
    : _rowsize(rowsize)
    , _colsize(colsize)
{
    _buf.resize(_rowsize*_colsize);
}

//--------------------------------------------------------------
Matrix::~Matrix()
{
}

//--------------------------------------------------------------
float& Matrix::operator()(unsigned int x, unsigned int y)
{
    return _buf[x*_colsize+y];
}

//--------------------------------------------------------------
void Matrix::operator*=(float s)
{
    for(auto& f : _buf) f*=s;
}

//--------------------------------------------------------------
bool Matrix::valid() const
{
    return _rowsize>0 && _colsize>0;
}


//--------------------------------------------------------------
Filter::Filter()
{
    initialize();
}

//--------------------------------------------------------------
Filter::Filter(const Matrix& mat)
{
    initialize();
    setMatrix(mat);
}

//--------------------------------------------------------------
Filter::~Filter()
{
    cleanup();
}

//--------------------------------------------------------------
void Filter::initialize()
{
    _area = sf::VertexBuffer(sf::Quads, sf::VertexBuffer::Static);
    _area.create(4);

    if (!_shader.loadFromMemory(s_glsl_vertex, s_glsl_filter))
    {
        std::cout << "err with filter shader..." << std::endl;
    }
}

//--------------------------------------------------------------
void Filter::cleanup()
{
}

//--------------------------------------------------------------
void Filter::setMatrix(const Matrix& mat)
{
    _matrix = mat;
}

//--------------------------------------------------------------
const sf::Texture& Filter::apply(const sf::Texture& src)
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

        _target.draw(_area, &_shader);
    }

    return texture();
}

//--------------------------------------------------------------
const sf::Texture& Filter::texture() const
{
    return _target.getTexture();
}

//--------------------------------------------------------------
void Filter::resize(const sf::Vector2u& size)
{
    _target.create(size.x,size.y);

    sf::Vertex vertices[] =
    {
        sf::Vertex(sf::Vector2f(     0,      0), sf::Color::White, sf::Vector2f(0,0)),
        sf::Vertex(sf::Vector2f(     0, size.y), sf::Color::White, sf::Vector2f(0,1)),
        sf::Vertex(sf::Vector2f(size.x, size.y), sf::Color::White, sf::Vector2f(1,1)),
        sf::Vertex(sf::Vector2f(size.x,      0), sf::Color::White, sf::Vector2f(1,0))
    };
    _area.update(vertices);
}







// --------------------------------------------------------------------------
static const std::string s_glsl_sobel = GLSL_CODE(
    uniform sampler2D u_src;
    uniform float u_matrix[9]; // 3x3
    uniform vec2 u_matrixsize;
    uniform vec2 u_srcsize;

    float mat_coef(vec2 st)
    {
        int i = int(st.x * u_matrixsize.y + st.y);
        return u_matrix[i];
    }

    vec3 src_value(vec2 uv, vec2 oft)
    {
        vec2 sample_oft = oft - vec2(u_matrixsize.x*0.5);
        vec2 sample_uv = uv + sample_oft/u_srcsize;
        return texture2D(u_src, sample_uv).xyz;
    }

    void main()
    {
        vec2 uv = gl_TexCoord[0].xy;
        uv.y = 1.0 - uv.y;
        float gx = 0.0;
        float gy = 0.0;

        for(float x=0.0;x<u_matrixsize.x;++x)
        {
            for(float y=0.0;y<u_matrixsize.y;++y)
            {
                vec2 st = vec2(x,y);
                gx += src_value(uv, st.xy).x *  mat_coef(st.xy);
                gy += src_value(uv, st.xy).x *  mat_coef(st.yx);
            }
        }

        float pi = 3.141592;

        float grad = sqrt( gx*gx + gy*gy );
        float ori = 0.0;
        if(gx==0.0)
            ori = gy>0.0 ? pi*0.5 : -pi*0.5;
        else
            ori = atan(gy,gx);


        grad = clamp(grad,0.0,1.0);
        ori += pi;
        ori /= (2.0*pi);
        // vec3 gd_map = vec3(grad,ori,0.0);
        vec3 gd_map = vec3(grad);

        gl_FragColor = vec4(gd_map, 1.0);
    }
);


//--------------------------------------------------------------
SobelFilter::SobelFilter()
{
    Matrix kernel(3,3);
    kernel(0,0) =  1.0; kernel(1,0) =  0.0; kernel(2,0) = -1.0;
    kernel(0,1) =  2.0; kernel(1,1) =  0.0; kernel(2,1) = -2.0;
    kernel(0,2) =  1.0; kernel(1,2) =  0.0; kernel(2,2) = -1.0;
    setMatrix(kernel);

    initialize();
}

//--------------------------------------------------------------
void SobelFilter::initialize()
{
    _area = sf::VertexBuffer(sf::Quads, sf::VertexBuffer::Static);
    _area.create(4);

    if (!_shader.loadFromMemory(s_glsl_vertex, s_glsl_sobel))
    {
        std::cout << "err with sobel shader..." << std::endl;
    }
}

//--------------------------------------------------------------
void SobelFilter::cleanup()
{

}



//--------------------------------------------------------------
BlurFilter::BlurFilter()
{
    float m = 1.0/9.0;
    Matrix kernel(3,3);
    kernel(0,0) = m; kernel(1,0) = m; kernel(2,0) = m;
    kernel(0,1) = m; kernel(1,1) = m; kernel(2,1) = m;
    kernel(0,2) = m; kernel(1,2) = m; kernel(2,2) = m;

    setMatrix(kernel);
}


//--------------------------------------------------------------
SharpFilter::SharpFilter()
{
    Matrix kernel(3,3);
    kernel(0,0) =  0.0; kernel(1,0) = -1.0; kernel(2,0) =  0.0;
    kernel(0,1) = -1.0; kernel(1,1) =  5.0; kernel(2,1) = -1.0;
    kernel(0,2) =  0.0; kernel(1,2) = -1.0; kernel(2,2) =  0.0;

    setMatrix(kernel);
}

//--------------------------------------------------------------
Gaussian5x5Filter::Gaussian5x5Filter()
{
    Matrix kernel(5,5);
    kernel(0,0) =  2.0; kernel(1,0) =  4.0; kernel(2,0) =  5.0; kernel(3,0) =  4.0; kernel(4,0) =  2.0;
    kernel(0,1) =  4.0; kernel(1,1) =  9.0; kernel(2,1) = 12.0; kernel(3,1) =  9.0; kernel(4,1) =  4.0;
    kernel(0,2) =  5.0; kernel(1,2) = 12.0; kernel(2,2) = 15.0; kernel(3,2) = 12.0; kernel(4,2) =  5.0;
    kernel(0,3) =  4.0; kernel(1,3) =  9.0; kernel(2,3) = 12.0; kernel(3,3) =  9.0; kernel(4,3) =  4.0;
    kernel(0,4) =  2.0; kernel(1,4) =  4.0; kernel(2,4) =  5.0; kernel(3,4) =  4.0; kernel(4,4) =  2.0;

    kernel *= 1.0/159.0;

    setMatrix(kernel);
}

//--------------------------------------------------------------
Edge3x3Filter::Edge3x3Filter()
{
    Matrix kernel(3,3);
    kernel(0,0) = -1.0; kernel(1,0) = -1.0; kernel(2,0) = -1.0;
    kernel(0,1) = -1.0; kernel(1,1) =  8.0; kernel(2,1) = -1.0;
    kernel(0,2) = -1.0; kernel(1,2) = -1.0; kernel(2,2) = -1.0;

    setMatrix(kernel);
}

//--------------------------------------------------------------
Gradient3x1Filter::Gradient3x1Filter()
{
    Matrix kernel(3,1);
    kernel(0,0) = -1.0; kernel(1,0) =  0.0; kernel(2,0) =  1.0;

    setMatrix(kernel);
}

//--------------------------------------------------------------
Gradient1x3Filter::Gradient1x3Filter()
{
    Matrix kernel(1,3);
    kernel(0,0) = -1.0;
    kernel(0,1) =  0.0;
    kernel(0,2) =  1.0;

    setMatrix(kernel);
}





















// --------------------------------------------------------------------------
static const std::string s_glsl_grad = GLSL_CODE(
    uniform sampler2D u_src;
    uniform float u_matrix[3]; // 3x1 or 1x3
    uniform vec2 u_matrixsize;
    uniform vec2 u_srcsize;

    vec3 src_value(vec2 uv, vec2 oft)
    {
        vec2 sample_oft = oft - vec2(u_matrixsize.x*0.5);
        vec2 sample_uv = uv + sample_oft/u_srcsize;
        return texture2D(u_src, sample_uv).xyz;
    }

    void main()
    {
        vec2 uv = gl_TexCoord[0].xy;
        uv.y = 1.0 - uv.y;
        float gx = 0.0;
        float gy = 0.0;

        for(int f=0;f<3;++f)
        {
            vec2 st_h = vec2(f,1.0);
            vec2 st_v = vec2(1.0,f);
            gx += src_value(uv, st_h).x * u_matrix[f];
            gy += src_value(uv, st_v).x * u_matrix[f];
        }

        float pi = 3.141592;

        float grad = sqrt( gx*gx + gy*gy );
        float ori = 0.0;
        if(gx==0.0)
            ori = gy>0.0 ? pi*0.5 : -pi*0.5;
        else
            ori = atan(gy,gx);


        grad = clamp(grad,0.0,1.0);
        ori += pi;
        ori /= (2.0*pi);
        vec3 gd_map = vec3(grad,ori,0.0);

        gl_FragColor = vec4(gd_map, 1.0);
    }
);



//--------------------------------------------------------------
GradientsMap::GradientsMap()
{
    Matrix kernel(3,1);
    kernel(0,0) = -1.0; kernel(1,0) =  0.0; kernel(2,0) =  1.0;
    setMatrix(kernel);

    initialize();
}

//--------------------------------------------------------------
void GradientsMap::initialize()
{
    _area = sf::VertexBuffer(sf::Quads, sf::VertexBuffer::Static);
    _area.create(4);

    if (!_shader.loadFromMemory(s_glsl_vertex, s_glsl_grad))
    {
        std::cout << "err with gradients map shader..." << std::endl;
    }
}

//--------------------------------------------------------------
void GradientsMap::cleanup()
{
}




// --------------------------------------------------------------------------
static const std::string s_glsl_maxima = GLSL_CODE(
    uniform sampler2D u_src;
    uniform float u_matrix[9]; // not used
    uniform vec2 u_matrixsize;
    uniform vec2 u_srcsize;

    float gradIntensity(vec2 uv, vec2 oft)
    {
        vec2 sample_uv = uv + oft/u_srcsize;
        return texture2D(u_src, sample_uv).x;
    }

    float gradOrientation(vec2 uv, vec2 oft)
    {
        vec2 sample_uv = uv + oft/u_srcsize;
        float n = texture2D(u_src, sample_uv).y;

        float pi = 3.141592;
        return n * (2.0*pi) - pi;
    }

    vec2 gradDir(vec2 uv, vec2 oft)
    {
        float o = gradOrientation(uv, oft);
        return vec2(cos(o),sin(o));
    }

    void main()
    {
        vec2 uv = gl_TexCoord[0].xy;
        uv.y = 1.0 - uv.y;

        float local_i = gradIntensity(uv, vec2(0.0));
        vec2 local_d = gradDir(uv, vec2(0.0));

        float c_i1 = gradIntensity(uv, -local_d);
        float c_i2 = gradIntensity(uv, local_d);

        if(c_i1 > local_i) local_i = 0.0;
        if(c_i2 > local_i) local_i = 0.0;

        // vec3 color = texture2D(u_src, uv).xyz;
        // color.x = local_i;
        // color.y = 0.0;

        vec3 color = vec3(local_i);

        gl_FragColor = vec4(color, 1.0);
    }
);



//--------------------------------------------------------------
LocalMaximaFilter::LocalMaximaFilter()
{
    Matrix kernel(3,3); // not used
    setMatrix(kernel);

    initialize();
}

//--------------------------------------------------------------
void LocalMaximaFilter::initialize()
{
    _area = sf::VertexBuffer(sf::Quads, sf::VertexBuffer::Static);
    _area.create(4);

    if (!_shader.loadFromMemory(s_glsl_vertex, s_glsl_maxima))
    {
        std::cout << "err with local maxima shader..." << std::endl;
    }
}

//--------------------------------------------------------------
void LocalMaximaFilter::cleanup()
{
}
