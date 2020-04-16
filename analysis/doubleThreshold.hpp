#ifndef BINARIZATION_HPP
#define BINARIZATION_HPP

#include <SFML/Graphics.hpp>

// --------------------------------------------------------------------------
// Helper class - give functions for thresholding texture
class DoubleThreshold
{
public:
    DoubleThreshold();
    virtual ~DoubleThreshold();

    void initialize();
    void cleanup();

    // compute a 3-values texture from a input and two thresholds
    const sf::Texture& apply( const sf::Texture& texture, float thresholdMajor = 0.5, float thresholdMinor = 0.1 );

    // get result texture
    const sf::Texture& getResultAsTexture();

protected:

    // resize render target
    void resizeRenderTarget(const sf::Vector2u& size);

    sf::RenderTexture m_target;         // target renderTexture
    sf::VertexBuffer m_vertexBuffer;    // target area
    sf::Shader m_2thresholdShader;        // shader for thresholding
};

#endif // BINARIZATION_HPP
