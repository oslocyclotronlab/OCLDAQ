

#include "Histogram3D.h"

#include <iostream>

#ifdef H3D_USE_BUFFER
const unsigned int Histogram3D::buffer_max;
#endif // H3D_USE_BUFFER

// ########################################################################

Histogram3D::Histogram3D( const std::string& name, const std::string& title,
                          Axis::index_t ch1, Axis::bin_t l1, Axis::bin_t r1, const std::string& xt, 
                          Axis::index_t ch2, Axis::bin_t l2, Axis::bin_t r2, const std::string& yt,
                          Axis::index_t ch3, Axis::bin_t l3, Axis::bin_t r3, const std::string& zt)
    : Named( name, title )
    , xaxis( name+"_xaxis", ch1, l1, r1, xt )
    , yaxis( name+"_yaxis", ch2, l2, r2, yt )
    , zaxis( name+"_zaxis", ch3, l3, r3, zt )
#ifndef USE_CUBE
    , data( 0 )
#else
    , cube( 0 )
#endif
{
#ifdef H2D_USE_BUFFER
    buffer.reserve(buffer_max);
#endif /* H2D_USE_BUFFER */

#ifndef USE_CUBE
    data = new data_t[xaxis.GetBinCountAll()*yaxis.GetBinCountAll()*zaxis.GetBinCountAll()];
#else
    cube = new data_t**[zaxis.GetBinCountAll()];
    for(int z=0; z<zaxis.GetBinCountAll(); ++z){
        cube[z] = new data_t*[yaxis.GetBinCountAll()];
        for (int y=0; y<yaxis.GetBinCountAll(); ++y){
        	cube[z][y] new data_t[xaxis.GetBinCountAll()];
        }
    }
#endif
    Reset();
}

// ########################################################################

Histogram3D::~Histogram3D()
{
#ifndef USE_CUBE
	delete data;
#else
	for (int z=0 ; z<zaxis.GetBinCountAll(); ++z){
		for (int y=0; y<yaxis.GetBinCountAll(); ++y){
			delete cube[z][y];
		}
		delete cube[z];
	}
	delete cube;
#endif // USE_CUBE
}

// ########################################################################

void Histogram3D::Add(const Histogram3Dp other, data_t scale)
{
    if( !other 
        || other->GetName() != GetName()
        || other->GetAxisX().GetLeft() != xaxis.GetLeft()
        || other->GetAxisX().GetRight() != xaxis.GetRight()
        || other->GetAxisX().GetBinCount() != xaxis.GetBinCount()
        || other->GetAxisY().GetLeft() != yaxis.GetLeft()
        || other->GetAxisY().GetRight() != yaxis.GetRight()
        || other->GetAxisY().GetBinCount() != yaxis.GetBinCount()
        || other->GetAxisZ().GetLeft() != zaxis.GetLeft()
        || other->GetAxisZ().GetRight() != zaxis.GetRight()
        || other->GetAxisZ().GetBinCount() != zaxis.GetBinCount() )
        return;

#ifdef H3D_USE_BUFFER
    other->FlushBuffer();
    FlushBuffer();
#endif /* H3D_USE_BUFFER */

#ifndef USE_CUBE
    for(int i=0; i<xaxis.GetBinCountAll()*yaxis.GetBinCountAll()*zaxis.GetBinCountAll(); ++i)
        data[i] += scale * other->data[i];
#else
    for(int z=0; z<zaxis.GetBinCountAll(); ++z)
	    for(int y=0; y<yaxis.GetBinCountAll(); ++y )
    	    for(int x=0; x<xaxis.GetBinCountAll(); ++x )
        	    cube[z][y][x] += scale*other->cube[z][y][x];
#endif
}

// ########################################################################

Histogram3D::data_t Histogram3D::GetBinContent(Axis::index_t xbin, Axis::index_t ybin, Axis::index_t zbin)
{
#ifdef H3D_USE_BUFFER
    if( !buffer.empty() )
        FlushBuffer();
#endif /* H3D_USE_BUFFER */

    if( xbin>=0 && xbin<xaxis.GetBinCountAll() && ybin>=0 && ybin<yaxis.GetBinCountAll() && zbin>=0 && zbin<zaxis.GetBinCountAll() ) {
#ifndef USE_CUBE
        return data[xaxis.GetBinCountAll()*yaxis.GetBinCountAll()*zbin + xaxis.GetBinCountAll()*ybin + xbin];
#else
        return cube[zbin][ybin][xbin];
#endif
    } else
        return 0;
}

// ########################################################################

void Histogram3D::SetBinContent(Axis::index_t xbin, Axis::index_t ybin, Axis::index_t zbin, data_t c)
{
#ifdef H3D_USE_BUFFER
    if( !buffer.empty() )
        FlushBuffer();
#endif /* H3D_USE_BUFFER */

    if( xbin>=0 && xbin<xaxis.GetBinCountAll() && ybin>=0 && ybin<yaxis.GetBinCountAll() && zbin>=0 && zbin<zaxis.GetBinCountAll() ) {
#ifndef USE_CUBE
        data[xaxis.GetBinCountAll()*yaxis.GetBinCountAll()*zbin + xaxis.GetBinCountAll()*ybin + xbin] = c;
#else
        cube[zbin][ybin][xbin] = c;
#endif
    }
}

// ########################################################################

void Histogram3D::FillDirect(Axis::bin_t x, Axis::bin_t y, Axis::bin_t z, data_t weight)
{
    const int xbin = xaxis.FindBin( x );
    const int ybin = yaxis.FindBin( y );
    const int zbin = zaxis.FindBin( z );
#ifndef USE_CUBE
    data[xaxis.GetBinCountAll()*yaxis.GetBinCountAll()*zbin + xaxis.GetBinCountAll()*ybin + xbin] += weight;
#else
    cube[zbin][ybin][xbin] += weight;
#endif
    entries += 1;
}

// ########################################################################

#ifdef H3D_USE_BUFFER
void Histogram3D::FlushBuffer()
{
    if( !buffer.empty() ) {
        for(buffer_t::const_iterator it=buffer.begin(); it<buffer.end(); ++it)
            FillDirect(it->x, it->y, it->z, it->w);
        buffer.clear();
    }
}
#endif /* H3D_USE_BUFFER */

// ########################################################################

void Histogram3D::Reset()
{
#ifdef H3D_USE_BUFFER
    buffer.clear();
#endif /* H3D_USE_BUFFER */
    for (int z=0; z<zaxis.GetBinCountAll(); ++z )
    	for(int y=0; y<yaxis.GetBinCountAll(); ++y )
        	for(int x=0; x<xaxis.GetBinCountAll(); ++x )
            	SetBinContent( x, y, z, 0 );
    entries = 0;
}

// ########################################################################
// ########################################################################

//#define TEST_HISTOGRAM3D
#ifdef TEST_HISTOGRAM3D

//#include "RootWriter.h"
//#include <TFile>

int main(int argc, char* argv[])
{
    Histogram3D h("ho", "hohoho", 10,0,10,"xho", 10,0,40, "yho", 10,0,20, "zho");
    h.Fill( 3,20,10, 7);
    h.Fill( 4,19,3, 6);
    h.Fill( 5,-2,13,1 );
    h.Fill( -1,-1,3, 10 );

    for (int iz=0 ; iz<12; ++iz){
    	std::cout << "z=" << iz << std::endl;
    	for(int iy=11; iy>=0; --iy) {
        	for(int ix=0; ix<12; ++ix)
            	std::cout << h.GetBinContent(ix, iy, iz) << ' ';
        	std::cout << std::endl;
    	}
    }
}

#endif // TEST_HISTOGRAM3D
