#ifndef FILTERING_HPP
#define FILTERING_HPP

#include <SFML/Graphics.hpp>

//--------------------------------------------------------------
// Define a matrix. Can be used by Filter or Morphology
class Matrix
{
public:
    Matrix();
    Matrix(unsigned int rowsize, unsigned int colsize);
    virtual ~Matrix();

    float& operator()(unsigned int x, unsigned int y);
    void operator*=(float s);

    bool valid() const;

    unsigned int size() {return _buf.size();}
    const float* data() const {return _buf.data();}

    unsigned int rowSize() const {return _rowsize;}
    unsigned int colSize() const {return _colsize;}

protected:
    std::vector<float> _buf;
    unsigned int _rowsize;
    unsigned int _colsize;
};

//--------------------------------------------------------------
// Define a filter operator to apply on Texture
class Filter
{
public:
    Filter();
    Filter(const Matrix& mat);
    virtual ~Filter();

    virtual void initialize();
    virtual void cleanup();

    void setMatrix(const Matrix& mat);

    virtual const sf::Texture& apply(const sf::Texture& src);

    const sf::Texture& texture() const;

protected:
    void resize(const sf::Vector2u& size);

    sf::RenderTexture _target;
    sf::VertexBuffer _area;
    sf::Shader _shader;
    Matrix _matrix;
};



//--------------------------------------------------------------
class SobelFilter : public Filter
{
public:
    SobelFilter();

    void initialize() override;
    void cleanup() override;
};

//--------------------------------------------------------------
struct BlurFilter : public Filter
{
    BlurFilter();
};

//--------------------------------------------------------------
struct SharpFilter : public Filter
{
    SharpFilter();
};

struct Gaussian5x5Filter : public Filter
{
    Gaussian5x5Filter();
};

//--------------------------------------------------------------
struct Edge3x3Filter : public Filter
{
    Edge3x3Filter();
};

//--------------------------------------------------------------
struct Gradient3x1Filter : public Filter
{
    Gradient3x1Filter();
};

//--------------------------------------------------------------
struct Gradient1x3Filter : public Filter
{
    Gradient1x3Filter();
};

//--------------------------------------------------------------
struct GradientsMap : public Filter
{
    GradientsMap();

    void initialize() override;
    void cleanup() override;
};


//--------------------------------------------------------------
struct LocalMaximaFilter : public Filter
{
    LocalMaximaFilter();

    void initialize() override;
    void cleanup() override;
};

#endif // FILTERING_HPP
