#include "blobAnalysis.hpp"

#include <iostream>
#include <cmath>
#include <list>

// --------------------------------------------------------------------------
BlobAnalysis::BlobAnalysis()
{
    initialize();
}

// --------------------------------------------------------------------------
BlobAnalysis::~BlobAnalysis()
{
    cleanup();
}

// --------------------------------------------------------------------------
void BlobAnalysis::initialize()
{
}

// --------------------------------------------------------------------------
void BlobAnalysis::cleanup()
{
}

// --------------------------------------------------------------------------
bool checkBound(const sf::Vector2i& pos, const sf::Vector2u& size)
{
    return pos.x>=0 && pos.x<(int)size.x && pos.y>=0 && pos.y<(int)size.y;
}

// --------------------------------------------------------------------------
const sf::Image& BlobAnalysis::apply( const sf::Image& input)
{
    // reset analysis data
    sf::Vector2u size = input.getSize();
    m_image.create(size.x,size.y,sf::Color(0,0,0,0));
    m_result.clear();

    std::list<sf::Vector2i> queue;
    unsigned int curr_label = 0;

    // search for un-scanned pixel to add in queue
    for(int x=0;x<(int)size.x;++x) for(int y=0;y<(int)size.y;++y)
    {
        sf::Vector2i position(x,y);
        unsigned int l = m_image.getPixel(x,y).toInteger();
        sf::Color value = input.getPixel(x,y);

        // if valid pixel with non label
        if(value.r > 200 && l==0)
        {
            // set curr label
            Group n; n.label = ++curr_label;

            // update image, add to queue and record result
            m_image.setPixel(x,y,sf::Color(n.label));
            queue.push_back(position);
            sf::Vector2i rpos(position.x,size.y-position.y);  // inverse Y
            n.position.push_back(rpos);
            m_result.push_back(n);
        }


        // if pixel wait in queue, check connected
        while( !queue.empty() )
        {
            // unqueue
            sf::Vector2i qpos = queue.front();
            queue.pop_front();

            // scan neighbors pixels
            for(int oftx=-1;oftx<=1;++oftx) for(int ofty=-1;ofty<=1;++ofty)
            {
                sf::Vector2i position2 = qpos + sf::Vector2i(oftx,ofty);

                if( checkBound(position2, size) )
                {
                    sf::Color value2 = input.getPixel(position2.x,position2.y);
                    unsigned int l2 = m_image.getPixel(position2.x,position2.y).toInteger();

                    // if valid pixel with non label
                    if(value2.r > 50 && l2==0)
                    {
                        // set curr label
                        l2 = curr_label;

                        // update image, add to queue and record result
                        m_image.setPixel(position2.x,position2.y,sf::Color(l2));
                        queue.push_back(position2);

                        sf::Vector2i rpos(position2.x,size.y-position2.y);  // inverse Y
                        m_result.back().position.push_back(rpos);
                    }
                }

            }// scan neighbors pixels
        }

    }

    // visualization pass
    for(int x=0;x<(int)size.x;++x) for(int y=0;y<(int)size.y;++y)
    {
        float l = float(m_image.getPixel(x,y).toInteger());
        l /= float(curr_label);
        l *= 16777215; // 255.0;
        m_image.setPixel(x,y,sf::Color(int(l)));
    }


    return m_image;
}

// --------------------------------------------------------------------------
const sf::Image& BlobAnalysis::getResultAsImage()
{
    return m_image;
}
