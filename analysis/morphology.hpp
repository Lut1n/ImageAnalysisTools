#ifndef MORPHOLOGY_HPP
#define MORPHOLOGY_HPP

#include <SFML/Graphics.hpp>

#include "filtering.hpp"

//--------------------------------------------------------------
// Define a morphology operator to apply on Texture
class Morphology
{
public:

    enum MorphType
    {
        Dilation,
        Erosion,
        Opening,
        Closing
    };

    Morphology(MorphType t = Dilation);
    Morphology(const Matrix& mat, MorphType t = Dilation);
    virtual ~Morphology();

    virtual void initialize();
    virtual void cleanup();

    void setMatrix(const Matrix& mat);

    virtual const sf::Texture& apply(const sf::Texture& src);

    const sf::Texture& texture() const;

protected:
    void resize(const sf::Vector2u& size);

    sf::RenderTexture _target, _subtarget;
    sf::VertexBuffer _area;
    sf::Shader _shader;
    Matrix _matrix;
    MorphType _type;
};

//--------------------------------------------------------------
struct Cross3x3Morpho : public Morphology
{
    Cross3x3Morpho(MorphType t = Dilation);
};

//--------------------------------------------------------------
struct Square3x3Morpho : public Morphology
{
    Square3x3Morpho(MorphType t = Dilation);
};

#endif // MORPHOLOGY_HPP
