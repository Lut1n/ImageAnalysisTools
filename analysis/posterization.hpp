#ifndef POSTERIZATION_HPP
#define POSTERIZATION_HPP

#include <SFML/Graphics.hpp>

// --------------------------------------------------------------------------
// Helper class - give functions for posterization using K-means algorithm
class Posterization
{
public:
    Posterization();
    virtual ~Posterization();

    void initialize();
    void cleanup();

    // compute a posterized image from a input and a K parameter
    const sf::Image& apply( const sf::Image& texture, int K = 255 );

    // get result texture
    const sf::Image& getResultAsImage();

protected:

    // resize render target
    void resizeRenderTarget(const sf::Vector2u& size);

    sf::Image m_target;         // target renderTexture
};

#endif // POSTERIZATION_HPP
