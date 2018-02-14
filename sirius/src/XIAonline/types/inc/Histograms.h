/* -*- c++ -*-
 *
 *  Created on: 14.03.2010
 *      Author: Alexander Bürger
 */

#ifndef HISTOGRAMS_H_
#define HISTOGRAMS_H_

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <cmath>

// ########################################################################

//! A named object with a title.
class Named {
protected:
    //! Construct a 1D histogram.
    Named( const std::string& name, /*!< The name of the object. */
           const std::string& title /*!< The title of the object. */);

public:
    //! Get the name of the object.
    /*! \return The object's name.
     */
    std::string GetName() const
        { return name;  }

    //! Get the title of the object.
    /*! \return The object's title.
     */
    std::string GetTitle() const
        { return title; }

private:
    //! The name of the object.
    const std::string name;

    //! The title of the object;
    const std::string title;
};

// ########################################################################
// ########################################################################

//! A histogram axis.
/*! The bin numbering convention is like in ROOT:
 *  <ul>
 *  <li>bin 0 = underflow bin</li>
 *  <li>bins [ 1, GetBinCount() ] = regular bins</li>
 *  <li>bin GetBinCount()+1 = overflow bin</li>
 *  </ul>
 */
class Axis : public Named {
public:
    //! The type used to index bins.
    typedef long long index_t;

    //! The type used to delimit bins.
    typedef double bin_t;
    
    //! Construct an axis.
    Axis( const std::string& hist_name, /*!< Name of the histogram using this axis */
          index_t channels,             /*!< The number of regular bins. */
          bin_t left,                   /*!< The lower edge of the lowest bin. */
          bin_t right,                  /*!< The upper edge of the highest bin. */
          const std::string& title      /*!< The title of the axis. */);

    //! Get the lower egde of the lowest regular bin.
    /*! \return The lower egde of the lowest regular bin.
     */
    bin_t GetLeft() const
        { return left;  }

    //! Get the upper egde of the highest regular bin.
    /*! \return The upper egde of the highest regular bin.
     */
    bin_t GetRight() const
        { return right; }

    //! Get the width of a regular bin.
    /*! \return The width of a regular bin.
     */
    bin_t GetBinWidth() const
        { return binwidth; }

    //! Get the number of regular bins.
    /*! \return The number of regular bins.
     */
    index_t GetBinCount() const
        { return channels2-2; }

    //! Get the number of bins including the overflow bins.
    /*! \return The total number of bins.
     */
    index_t GetBinCountAll() const
        { return channels2; }

    //! Find a bin number.
    /*! \return The number of the bin.
     */
    index_t FindBin(bin_t x) const
        {   index_t bin;
            if( x < left )
                bin = 0;
            else if( x < right )
                bin = 1 + std::floor((x-left)/binwidth);
            else
                bin = channels2-1;
            return bin;
        }

private:
    //! The number of bins including the overflow bins.
    index_t channels2;

    //! The lower edge of the lowest regular bin.
    bin_t left;

    //! The upper edge of the highest regular bin.
    bin_t right;

    //! The width of a bin.
    bin_t binwidth;
};

// ########################################################################
// ########################################################################

class Histogram1D;
class Histogram2D;
class Histogram3D;

typedef std::shared_ptr<Histogram1D> Histogram1Dp;
typedef std::shared_ptr<Histogram2D> Histogram2Dp;
typedef std::shared_ptr<Histogram3D> Histogram3Dp;

//! A set of histograms.
class Histograms {
public:
    //! A list of 1D histograms.
    typedef std::vector<Histogram1Dp> list1d_t;

    //! A list of 2D histograms.
    typedef std::vector<Histogram2Dp> list2d_t;

    //! A list of 3D histograms.
    typedef std::vector<Histogram3Dp> list3d_t;

    //! Deletes all histograms.
    ~Histograms();

    
    //! Create a 1D histogram.
    /*! It will be added to this set of histograms and deleted when the set is destroyed.
     *
     * \return the new histogram.
     */
    Histogram1Dp Create1D( const std::string& name,  /*!< The name of the new histogram. */
                           const std::string& title, /*!< The title of teh new histogram. */
                           Axis::index_t channels,   /*!< The number of regular bins. */
                           Axis::bin_t left,         /*!< The lower edge of the lowest bin.  */
                           Axis::bin_t right,        /*!< The upper edge of the highest bin. */
                           const std::string& xtitle /*!< The title of the x axis. */);

    //! Create a 2D histogram.
    /*! It will be added to this set of histograms and deleted when the set is destroyed.
     *
     * \return the new histogram.
     */
    Histogram2Dp Create2D( const std::string& name,   /*!< The name of the new histogram. */
                           const std::string& title,  /*!< The title of teh new histogram. */
                           Axis::index_t xchannels,   /*!< The number of regular bins on the x axis. */
                           Axis::bin_t xleft,         /*!< The lower edge of the lowest bin on the x axis. */
                           Axis::bin_t xright,        /*!< The upper edge of the highest bin on the x axis. */
                           const std::string& xtitle, /*!< The title of the x axis. */
                           Axis::index_t ychannels,   /*!< The number of regular bins on the y axis. */
                           Axis::bin_t yleft,         /*!< The lower edge of the lowest bin on the y axis. */
                           Axis::bin_t yright,        /*!< The upper edge of the highest bin on the y axis. */
                           const std::string& ytitle  /*!< The title of the y axis. */);

    //! Create a 3D histogram.
    /*! It will be added to this set of histograms and deleted when the set is destroyed.
     *
     * \return the new histogram.
     */
    Histogram3Dp Create3D( const std::string& name,   /*!< The name of the new histogram. */
                           const std::string& title,  /*!< The title of teh new histogram. */
                           Axis::index_t xchannels,   /*!< The number of regular bins on the x axis. */
                           Axis::bin_t xleft,         /*!< The lower edge of the lowest bin on the x axis. */
                           Axis::bin_t xright,        /*!< The upper edge of the highest bin on the x axis. */
                           const std::string& xtitle, /*!< The title of the x axis. */
                           Axis::index_t ychannels,   /*!< The number of regular bins on the y axis. */
                           Axis::bin_t yleft,         /*!< The lower edge of the lowest bin on the y axis. */
                           Axis::bin_t yright,        /*!< The upper edge of the highest bin on the y axis. */
                           const std::string& ytitle, /*!< The title of the y axis. */
                           Axis::index_t zchannels,   /*!< The number of regular bins on the y axis. */
                           Axis::bin_t zleft,         /*!< The lower edge of the lowest bin on the y axis. */
                           Axis::bin_t zright,        /*!< The upper edge of the highest bin on the y axis. */
                           const std::string& ztitle  /*!< The title of the y axis. */);

    //! Get a list of all 1D histograms.
    list1d_t GetAll1D();

    //! Get a list of all 2D histograms.
    list2d_t GetAll2D();

    //! Get a list of all 3D histograms.
    list3d_t GetAll3D();

    //! Call Reset() on all histograms.
    void ResetAll();

    //! Find a specific 1D histogram.
    /*! \return the histogram, or 0 if not found.
     */
    Histogram1Dp Find1D( const std::string& name /*!< The name of the histogram to search. */);

    //! Find a specific 2D histogram.
    /*! \return the histogram, or 0 if not found.
     */
    Histogram2Dp Find2D( const std::string& name /*!< The name of the histogram to search. */);

    //! Find a specific 3D histogram.
    /*! \return the histogram, or 0 if not found.
     */
    Histogram3Dp Find3D( const std::string& name /*!< The name of the histogram to search. */);

    //! Add all the histograms from other to this set's histograms.
    /*! For each of the histograms of this set, add the contents of the same histogram in other. */
    void Merge(Histograms& other /*!< The set of histograms to add. */);

private:
    //! Type for the map of histogram names to 1D histograms.
    typedef std::map<std::string, Histogram1Dp> map1d_t;

    //! The map of histogram names to 1D histograms.
    map1d_t map1d;

    //! Type for the map of histogram names to 2D histograms.
    typedef std::map<std::string, Histogram2Dp> map2d_t;

    //! The map of histogram names to 2D histograms.
    map2d_t map2d;

    //! Type for the map of histogram names to 3D histograms.
    typedef std::map<std::string, Histogram3Dp> map3d_t;

    //! The map of histogram names to 3D histograms.
    map3d_t map3d;
};

#endif /* HISTOGRAMS_H_ */
