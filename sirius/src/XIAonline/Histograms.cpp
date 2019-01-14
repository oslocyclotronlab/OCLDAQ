
#include "Histograms.h"

#include "Histogram1D.h"
#include "Histogram2D.h"
#include "Histogram3D.h"

#include <iostream>

Named::Named( const std::string& nm, const std::string& ttl)
    : name( nm )
    , title( ttl )
{
}

// ########################################################################
// ########################################################################

Axis::Axis(const std::string& name, index_t c, bin_t l, bin_t r, const std::string& t )
    : Named( name, t )
    , channels2( c+2 )
    , left( l )
    , right( r )
{
    const double cbw = (right-left)/double(channels2-2);
    binwidth = (bin_t)cbw;
    if( cbw != double(binwidth) )
        std::cout << "non-int binwidth for axis '" << name << "'" << std::endl;
    if( binwidth == 0 )
        std::cout << "zero binwidth for axis '" << name << "'" << std::endl;
    if ( binwidth < 0 )
        std::cout << "negative binwidth for axis '" << name << "'" << std::endl;
}

// ########################################################################
// ########################################################################

Histograms::~Histograms()
{
    /*
    for( map1d_t::iterator it = map1d.begin(); it != map1d.end(); ++it )
        delete it->second;
    for( map2d_t::iterator it = map2d.begin(); it != map2d.end(); ++it )
        delete it->second;
        */
}

// ########################################################################

Histogram1Dp Histograms::Create1D( const std::string& name, const std::string& title,
                                   Axis::index_t c, Axis::bin_t l, Axis::bin_t r, const std::string& xtitle )
{
    Histogram1Dp h(new Histogram1D(name, title, c, l, r, xtitle));
    map1d[ name ] = h;
    return h;
}

// ########################################################################

Histogram2Dp Histograms::Create2D( const std::string& name, const std::string& title,
                                   Axis::index_t ch1, Axis::bin_t l1, Axis::bin_t r1, const std::string& xtitle,
                                   Axis::index_t ch2, Axis::bin_t l2, Axis::bin_t r2, const std::string& ytitle)
{
    Histogram2Dp h(new Histogram2D(name, title, ch1, l1, r1, xtitle, ch2, l2, r2, ytitle));
    map2d[ name ] = h;
    return h;
}

// ########################################################################

Histogram3Dp Histograms::Create3D( const std::string& name, const std::string& title,
                                   Axis::index_t ch1, Axis::bin_t l1, Axis::bin_t r1, const std::string& xtitle,
                                   Axis::index_t ch2, Axis::bin_t l2, Axis::bin_t r2, const std::string& ytitle,
                                   Axis::index_t ch3, Axis::bin_t l3, Axis::bin_t r3, const std::string& ztitle)
{
    Histogram3Dp h(new Histogram3D(name, title, ch1, l1, r1, xtitle, ch2, l2, r2, ytitle, ch3, l3, r3, ztitle));
    map3d[ name ] = h;
    return h;
}

// ########################################################################

void Histograms::ResetAll()
{
    for( map1d_t::iterator it = map1d.begin(); it != map1d.end(); ++it )
        it->second->Reset();
    for( map2d_t::iterator it = map2d.begin(); it != map2d.end(); ++it )
        it->second->Reset();
}

// ########################################################################

Histogram1Dp Histograms::Find1D( const std::string& name )
{
    map1d_t::iterator it = map1d.find( name );
    if( it != map1d.end() )
        return it->second;
    else
        return 0;
}

// ########################################################################

Histogram2Dp Histograms::Find2D( const std::string& name )
{
    map2d_t::iterator it = map2d.find( name );
    if( it != map2d.end() )
        return it->second;
    else
        return 0;
}

// ########################################################################

Histogram3Dp Histograms::Find3D( const std::string& name )
{
    map3d_t::iterator it = map3d.find( name );
    if( it != map3d.end() )
        return it->second;
    else
        return 0;
}

// ########################################################################

void Histograms::Merge(Histograms& other)
{
    for( map1d_t::iterator it = map1d.begin(); it != map1d.end(); ++it ) {
        Histogram1Dp me = it->second;
        Histogram1Dp you = other.Find1D( me->GetName() );
        if( you )
            me->Add( you, 1 );
    }
    for( map2d_t::iterator it = map2d.begin(); it != map2d.end(); ++it ) {
        Histogram2Dp me = it->second;
        Histogram2Dp you = other.Find2D( me->GetName() );
        if( you )
            me->Add( you, 1 );
    }
}

// ########################################################################

Histograms::list1d_t Histograms::GetAll1D()
{
    list1d_t list1d;
    for( map1d_t::iterator it = map1d.begin(); it != map1d.end(); ++it )
        list1d.push_back( it->second );
    return list1d;
}

// ########################################################################

Histograms::list2d_t Histograms::GetAll2D()
{
    list2d_t list2d;
    for( map2d_t::iterator it = map2d.begin(); it != map2d.end(); ++it )
        list2d.push_back( it->second );
    return list2d;
}

// ########################################################################

Histograms::list3d_t Histograms::GetAll3D()
{
    list3d_t list3d;
    for( map3d_t::iterator it = map3d.begin(); it != map3d.end(); ++it )
        list3d.push_back( it->second );
    return list3d;
}
