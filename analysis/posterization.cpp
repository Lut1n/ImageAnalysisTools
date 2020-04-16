#include "posterization.hpp"

#include <iostream>

// --------------------------------------------------------------------------
std::vector<int> histo(const sf::Image& img)
{
    std::vector<int> hist(255,0);
    for(int x=0;x<(int)img.getSize().x;++x) for(int y=0;y<(int)img.getSize().y;++y)
    {
        hist[ img.getPixel(x,y).r ]++;
    }

    // for(int i=0;i<255;++i)
    // {
    //     float v = float(hist[i]) / float(img.getSize().x*img.getSize().y);
    //     for(int k=0;k<v*100.0;++k) std::cout << ".";
    //     std::cout << std::endl;
    // }


    return hist;
}

// --------------------------------------------------------------------------
int meanAt(const std::vector<int>& h, int i)
{
    int n = h.size();

    int mn = std::max(i-3,0);
    int mx = std::min(i+3,n-1);

    int res = 0;
    for(int j=mn;j<=mx;++j) res += h[j];
    return res / (1+mx-mn);
}

// --------------------------------------------------------------------------
std::vector<int> maxima(const std::vector<int>& h)
{
    std::vector<int> maxsIndex;
    std::vector<int> maxsValue;

    int m0 = 0;
    int m1 = meanAt(h,0);
    int m2 = 0;

    for(int i=0;i<255;++i)
    {
        m2 = meanAt(h,i+1);

        if(m1>m0 && m1>m2)
        {
            maxsIndex.push_back(i);
            maxsValue.push_back(h[i]);
        }

        m0 = m1;
        m1 = m2;
    }

    // sorting
    std::vector<int> maxs(maxsIndex.size());
    std::vector<bool> taken(maxsIndex.size(),false);
    for(int i=0;i<(int)maxs.size();++i)
    {
        int mx = -1;
        for(int j=0;j<(int)maxsIndex.size();++j)
        {
            if( taken[j]==false && (mx==-1 || maxsValue[j]>maxsValue[mx]) ) mx = j;
        }
        maxs[i] = maxsIndex[mx];
        taken[mx] = true;
    }

    return maxs;
}

// --------------------------------------------------------------------------
Posterization::Posterization()
{
    initialize();
}

// --------------------------------------------------------------------------
Posterization::~Posterization()
{
    cleanup();
}

// --------------------------------------------------------------------------
void Posterization::initialize()
{
}

// --------------------------------------------------------------------------
void Posterization::cleanup()
{
}

// --------------------------------------------------------------------------
void Posterization::resizeRenderTarget(const sf::Vector2u& size)
{
    m_target.create(size.x,size.y, sf::Color::Transparent);
}

// --------------------------------------------------------------------------
const sf::Image& Posterization::apply(const sf::Image &input, int K)
{
    std::vector<int> hist = histo(input);
    std::vector<int> mxs = maxima( hist );
    std::cout << std::endl;

    K = std::min((int)mxs.size(),K);

    std::srand(time(NULL));

    sf::Vector2u currSize = m_target.getSize();
    sf::Vector2u size = input.getSize();
    if(currSize != size)
    {
        resizeRenderTarget(size);
        currSize = size;
    }

    std::vector<int> k_colors(K);
    std::vector<int> k_next(K);
    std::vector<int> k_npx(K);

    // init
    for(int i=0;i<K;++i) k_colors[i] = mxs[i];

    int last_shift_max = 1000;
    int ite = 0;

    while(last_shift_max > 0)
    {
        ite++;
        for(int k=0;k<K;++k) { k_next[k]=0; k_npx[k]=0; }

        // scan pixels and associate it to closest mean
        for(int x=0;x<(int)size.x;++x) for(int y=0;y<(int)size.y;++y)
        {
            sf::Color px = input.getPixel(x,y);

            // find closest mean
            int cd = 1000;
            int b = 0;
            int cc = 0;
            for(int k=0;k<K;++k)
            {
                int d = std::abs(k_colors[k] - px.r);
                if(d < cd) { b=k; cd=d; cc=px.r; }
            }
            k_next[b] += cc;
            k_npx[b]++;
        }

        // update means
        last_shift_max = 0;
        for(int k=0;k<K;++k)
        {
            if(k_npx[k]==0){k_colors[k]=0; continue;}
            int last_color = k_colors[k];
            k_colors[k] = k_next[k]/k_npx[k];
            last_shift_max = std::max(last_shift_max, std::abs(last_color-k_colors[k]));
        }
    }
    // std::cout << "k-mean convergenced in " << ite << " iterations" << std::endl;

    // generate result
    for(int x=0;x<(int)size.x;++x) for(int y=0;y<(int)size.y;++y)
    {
        sf::Color px = input.getPixel(x,y);

        // find closest mean
        int cd = 1000;
        int b = 0;
        for(int k=0;k<K;++k)
        {
            int d = std::abs(k_colors[k] - px.r);
            if(d < cd) { b=k; cd=d; }
        }

        px = sf::Color(k_colors[b],k_colors[b],k_colors[b]);
        m_target.setPixel(x,y,px);
    }


    return m_target;
}

// --------------------------------------------------------------------------
const sf::Image& Posterization::getResultAsImage()
{
    return m_target;
}
