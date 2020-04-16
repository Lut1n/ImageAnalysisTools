#ifndef TEXTURE_CONVERSION_HPP
#define TEXTURE_CONVERSION_HPP

#include <SFML/Graphics.hpp>

// --------------------------------------------------------------------------
// Helper class - give functions for texture conversion
class TextureConversion
{
public:
    TextureConversion();
    virtual ~TextureConversion();

    void initialize();
    void cleanup();

    // compute a grayscale texture from a given texture
    const sf::Texture& computeGrayscale( const sf::Texture& texture );

    // compute a resized texture from a given texture
    const sf::Texture& computeResizing( const sf::Texture& texture, const sf::Vector2u& newsize );

    // get result texture
    const sf::Texture& getResultAsTexture();

protected:

    // resize render target
    void resizeRenderTarget(const sf::Vector2u& size);

    sf::RenderTexture m_target;         // target renderTexture
    sf::VertexBuffer m_vertexBuffer;    // target area
    sf::Shader m_grayscaleShader;       // shader for grayscale
    sf::Shader m_resizeShader;          // shader for resizing
};

#endif // TEXTURE_CONVERSION_HPP
