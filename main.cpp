#include <SFML/Graphics.hpp>

#include "analysis/blobAnalysis.hpp"
#include "analysis/conversion.hpp"
#include "analysis/doubleThreshold.hpp"
#include "analysis/filtering.hpp"
#include "analysis/morphology.hpp"
#include "analysis/posterization.hpp"


// -----------------------------------------------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    std::vector<std::string> args;
    if(argc>1) args = std::vector<std::string>(argv+1,argv+argc);

    sf::RenderWindow win(sf::VideoMode(1024, 768), "Image Analysis with SFML", sf::Style::Default, sf::ContextSettings(24));
    win.setFramerateLimit(60);

    // loading
    sf::Texture texture;
    texture.loadFromFile("./demo/original.jpg");
    sf::Sprite sprite; sprite.setTexture(texture);


    // grayscale conversion
    TextureConversion grayscaleConv;
    const sf::Texture& grayscaled = grayscaleConv.computeGrayscale(texture);


    // posterization using K-means algorithm
    Posterization post;
    const sf::Image& post_res = post.apply(grayscaled.copyToImage(), 3);
    sf::Texture post_tex; post_tex.loadFromImage(post_res);


    // Canny filtering procedure
    Gaussian5x5Filter canny_blur; const sf::Texture& tex_cy_blur = canny_blur.apply(grayscaled);
    GradientsMap canny_grads; const sf::Texture& tex_cy_grads = canny_grads.apply(tex_cy_blur);
    LocalMaximaFilter maxima; const sf::Texture& tex_cy_maxima = maxima.apply(tex_cy_grads);
    DoubleThreshold thresholding; const sf::Texture& tex_cy_bin = thresholding.apply(tex_cy_maxima,0.04, 0.03);
    BlobAnalysis blob; const sf::Image& blobImage = blob.apply(tex_cy_bin.copyToImage());
    sf::Texture tex_cy_blob; tex_cy_blob.loadFromImage(blobImage);


    // Some others operations (sharpeness, sobel filter, morphology operations)
    SharpFilter sharpizer;
    const sf::Texture& sharped = sharpizer.apply(grayscaled);
    SobelFilter sobel;
    const sf::Texture& sobeled = sobel.apply(grayscaled);
    Square3x3Morpho dilation, erosion(Morphology::Erosion);
    const sf::Texture& dilated = dilation.apply(post_tex);
    const sf::Texture& eroded = erosion.apply(post_tex);


    // display results
    float tex_width = texture.getSize().x;
    float tex_height = texture.getSize().y;
    while (win.isOpen())
    {
        sf::Event ev;
        while (win.pollEvent(ev)) if (ev.type == sf::Event::Closed) win.close();


        // line #1
        sprite.setTexture(grayscaled,true);
        sprite.setPosition( 0.0 * tex_width, 0.0*tex_height );
        win.draw(sprite);

        sprite.setTexture(post_tex);
        sprite.setPosition( 1.0 * tex_width, 0.0*tex_height );
        win.draw(sprite);

        sprite.setTexture(tex_cy_blur);
        sprite.setPosition( 2.0 * tex_width, 0.0*tex_height );
        win.draw(sprite);

        sprite.setTexture(tex_cy_grads);
        sprite.setPosition( 3.0 * tex_width, 0.0*tex_height );
        win.draw(sprite);


        // line #2
        sprite.setTexture(tex_cy_maxima);
        sprite.setPosition( 0.0 * tex_width, 1.0*tex_height );
        win.draw(sprite);

        sprite.setTexture(tex_cy_bin);
        sprite.setPosition( 1.0 * tex_width, 1.0*tex_height );
        win.draw(sprite);

        sprite.setTexture(tex_cy_blob);
        sprite.setPosition( 2.0 * tex_width, 1.0*tex_height );
        win.draw(sprite);

        sprite.setPosition( 3.0 * tex_width, 1.0*tex_height );
        sprite.setTexture(post_tex);
        win.draw(sprite);
        sprite.setTexture(tex_cy_blob);
        win.draw(sprite);


        // line #3
        sprite.setTexture(sharped,true);
        sprite.setPosition( 0.0 * tex_width, 2.0*tex_height );
        win.draw(sprite);

        sprite.setTexture(sobeled);
        sprite.setPosition( 1.0 * tex_width, 2.0*tex_height );
        win.draw(sprite);

        sprite.setTexture(dilated);
        sprite.setPosition( 2.0 * tex_width, 2.0*tex_height );
        win.draw(sprite);

        sprite.setTexture(eroded);
        sprite.setPosition( 3.0 * tex_width, 2.0*tex_height );
        win.draw(sprite);

        win.display();
    }
   
    return 0;
}


