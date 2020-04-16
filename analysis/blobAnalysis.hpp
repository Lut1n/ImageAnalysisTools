#ifndef BLOB_ANALYSIS_HPP
#define BLOB_ANALYSIS_HPP

#include <SFML/Graphics.hpp>

// --------------------------------------------------------------------------
// Helper class - give functions for connected-component analysis on image
class BlobAnalysis
{
public:

    struct Group
    {
        std::vector<sf::Vector2i> position;
        unsigned int label;
    };

    BlobAnalysis();
    virtual ~BlobAnalysis();

    void initialize();
    void cleanup();

    // compute a connectivity check from a input
    const sf::Image& apply( const sf::Image& image);

    // get result image
    const sf::Image& getResultAsImage();

    const std::vector<Group>& getResult() {return m_result;}

protected:
    sf::Image m_image;                  // target renderTexture
    std::vector<Group> m_result;        // analysis result;
};

#endif // BLOB_ANALYSIS_HPP
