//==================================================================================================
//
//  Copyright(c)  2013 - 2015  Naïo Technologies
//
//  This program is free software: you can redistribute it and/or modify it under the terms of the
//  GNU General Public License as published by the Free Software Foundation, either version 3 of
//  the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
//  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along with This program.
//  If not, see <http://www.gnu.org/licenses/>.
//
//==================================================================================================

#ifndef CLASSEXTRACTION_HPP
#define CLASSEXTRACTION_HPP

//==================================================================================================
// I N C L U D E   F I L E S

#include <HTBitmap.hpp>
#include <CLArray.h>
#include <CLDynArray.h>


//==================================================================================================
// F O R W A R D   D E C L A R A T I O N S

//==================================================================================================
// C O N S T A N T S

//==================================================================================================
// C L A S S E S

class Histogram
{
//--Methods-----------------------------------------------------------------------------------------
public:
	Histogram( const size_t size )
		: histogram_( size, 0 )
	{ }

	~Histogram()
	{ }

	size_t size() const
	{
		return histogram_.size();
	}

	void from_channel( const cv::Mat& src )
	{
		for( int32_t i = 0; i < src.rows; ++i )
		{
			for( int32_t j = 0; j < src.cols; ++j )
			{
				size_t index = static_cast<size_t>(src.at<uint8_t>( i, j ));
				++histogram_[index];
			}
		}
	}

	/// Return the byte at a specific index without boundaries checking.
	uint32_t operator[]( const size_t index ) const
	{
		return histogram_[index];
	}

	uint32_t& operator[]( const size_t index )
	{
		return histogram_[index];
	}

	/// Return the byte at a specific index with boundaries checking
	uint32_t at( const size_t index ) const
	{
		return histogram_.at( index );
	}

	uint32_t& at( const size_t index )
	{
		return histogram_.at( index );
	}

	uint32_t maximum( const size_t begin, const size_t end ) const
	{
		auto it = std::max_element( histogram_.cbegin() + begin, histogram_.cbegin() + end );
		return *it;
	}

	std::vector<uint32_t>::iterator begin()
	{
		return histogram_.begin();
	}

	std::vector<uint32_t>::iterator end()
	{
		return histogram_.end();
	}

	const std::vector<uint32_t>::const_iterator cbegin() const
	{
		return histogram_.cbegin();
	}

	const std::vector<uint32_t>::const_iterator cend() const
	{
		return histogram_.cend();
	}

	std::vector<uint32_t>::reverse_iterator rbegin()
	{
		return histogram_.rbegin();
	}

	std::vector<uint32_t>::reverse_iterator rend()
	{
		return histogram_.rend();
	}

	const std::vector<uint32_t>::const_reverse_iterator crbegin() const
	{
		return histogram_.crbegin();
	}

	const std::vector<uint32_t>::const_reverse_iterator crend() const
	{
		return histogram_.crend();
	}


//--Data members------------------------------------------------------------------------------------
private:
	std::vector<uint32_t> histogram_;
};

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>


void showHistogramWithThresholds( const Histogram& histogram, const std::string& windowName,
                                  const std::vector<uint32_t>& thresholds )
{
	int32_t bins{ 256 };    // number of bins
	cv::Mat hist;      // array for storing the histograms
	cv::Mat canvas;    // images for displaying the histogram
	int32_t hmax{ };        // peak value for each histogram

	hist = cv::Mat::zeros( 1, bins, CV_32SC1);

	for( size_t i = 0; i < histogram.size(); ++i )
	{
		hist.at<int32_t>( i ) = histogram[i];
	}

	for( int32_t j = 0; j < bins - 1; ++j )
	{
		hmax = hist.at<int32_t>( j ) > hmax ? hist.at<int32_t>( j ) : hmax;
	}

	canvas = cv::Mat::ones( 125, bins, CV_8UC3);

	for( int j = 0, rows = canvas.rows; j < bins - 1; j++ )
	{
		if( std::find( thresholds.begin(), thresholds.end(), j ) != thresholds.end())
		{
			line( canvas, cv::Point( j, 0 ), cv::Point( j, hmax ), cv::Scalar( 0, 255, 0 ), 1, 8 );
		}
		else
		{
			line( canvas,
			      cv::Point( j, rows ),
			      cv::Point( j, rows - (hist.at<int32_t>( j ) * rows / hmax)),
			      cv::Scalar( 255, 255, 255 ),
			      1, 8, 0 );
		}
	}

	cv::imshow( windowName, canvas );
}

void showHistogram( const Histogram& histogram, const std::string& windowName )
{
	int32_t bins{ 256 };    // number of bins
	cv::Mat hist;      // array for storing the histograms
	cv::Mat canvas;    // images for displaying the histogram
	int32_t hmax{ };        // peak value for each histogram

	hist = cv::Mat::zeros( 1, bins, CV_32SC1);

	for( size_t i = 0; i < histogram.size(); ++i )
	{
		hist.at<int32_t>( i ) = histogram[i];
	}

	for( int32_t j = 0; j < bins - 1; ++j )
	{
		hmax = hist.at<int32_t>( j ) > hmax ? hist.at<int32_t>( j ) : hmax;
	}

	canvas = cv::Mat::ones( 125, bins, CV_8UC3);

	for( int j = 0, rows = canvas.rows; j < bins - 1; j++ )
	{
		line( canvas,
		      cv::Point( j, rows ),
		      cv::Point( j, rows - (hist.at<int32_t>( j ) * rows / hmax)),
		      cv::Scalar( 255, 255, 255 ), 1, 8, 0 );
	}

	cv::imshow( windowName, canvas );
}

void showHistogramRange( const std::string& windowName, const Histogram& histogram, size_t begin,
                         size_t end )
{
	int32_t bins{ 256 };    // number of bins
	cv::Mat hist;           // array for storing the histograms
	cv::Mat canvas;         // images for displaying the histogram
	int32_t hmax{ };        // peak value for each histogram

	hist = cv::Mat::zeros( 1, bins, CV_32SC1);

	for( size_t i = 0; i < histogram.size(); ++i )
	{
		hist.at<int32_t>( i ) = histogram[i];
	}

	for( int32_t j = 0; j < bins - 1; ++j )
	{
		hmax = hist.at<int32_t>( j ) > hmax ? hist.at<int32_t>( j ) : hmax;
	}

	canvas = cv::Mat::ones( 125, bins, CV_8UC3);

	for( int j = 0, rows = canvas.rows; j < bins - 1; j++ )
	{
		line( canvas,
		      cv::Point( j, rows ),
		      cv::Point( j, rows - (hist.at<int32_t>( j ) * rows / hmax)),
		      cv::Scalar( 255, 255, 255 ),
		      1, 8, 0 );
	}

	cv::imshow( windowName, canvas );
}

class ClassExtraction
{
//--Methods-----------------------------------------------------------------------------------------
public:
	ClassExtraction()
	{ }

	~ClassExtraction()
	{ }

	// Computes a control point position based on  symmetries of 2 adjacents
	// points BB n-1 et BBn+1
	//#define ctrlpt( pt, pt1, pt2 )  ( mil( mil(pt,pt1), sym(pt,pt2) ) )
	//#define sym( a, b ) ( a - ((a-b)*acoef) / 100 )     // symetric a  and  b  integers
	//#define mil( a, b ) ( (a+b) / 2 )                   // middle a and b integers

	int32_t middle( const int32_t a, const int32_t b ) const
	{
		return static_cast<int32_t>(std::round((a + b) / 2 ));
	}

	int32_t symetric( const int32_t a, const int32_t b, const int32_t coefficent ) const
	{
		return a - ((a - b) * coefficent) / 100;
	}

	int32_t control_point( const int32_t point, const int32_t point1, const int32_t point2,
	                       const int32_t coef )
	{
		return middle( middle( point, point1 ), symetric( point, point2, coef ));
	}

	double CriteryKittler( const double p1, const double p2, const double sig1, const double& sig2 )
	{
		return 1 + (p1 * log( sig1 / cl::math::square( p1 ))) +
		       (p2 * log( sig2 / cl::math::square( p2 )));
	}

	double CriteryOtsu( const double& p1, const double& p2, const double& mu1, const double& muT )
	{
		return -cl::math::square( muT * p1 - mu1 ) / (p1 * p2);
	}

	double CriteryPal( const double& p1, const double& p2, const double& mu1, const double& mu2,
	                   const double& muT )
	{
		return (muT - p1 * (log( p1 ) + mu1 * log( mu1 )) - p2 * (log( p2 ) + mu2 * log( mu2 )));
	}

	/// Smoothing with Bezier's curves
	/// Smoothing algorithm computes Bezier control points acoef is the smoothing factor takes care
	/// of points 0 and NB2(or NB-1) when they are at the same location (closed curve)
	///
	void BezierContourSmoothing( const Histogram& contourX, Histogram& contourY, uint32_t acoef )
	{
		if( contourX.size() < 3 )
		{
			return;
		}

		size_t NB2{ contourX.size() * 3 };
		std::vector <int32_t> BBy( NB2, 0 );

		for( size_t i = 0; i < contourX.size(); ++i )
		{
			BBy[i * 3] = contourX[i];
		}

		// Computes control points of the source (edges) excluding end points
		for( int32_t j = 1; j < contourX.size() - 1; ++j )
		{
			int32_t i = j * 3;
			BBy[i - 1] = control_point( BBy[i], BBy[i - 3], BBy[i + 3], acoef );
			BBy[i + 1] = control_point( BBy[i], BBy[i + 3], BBy[i - 3], acoef );
		}

		// first element i=0
		if( BBy[0] == BBy[NB2 - 3] ) // Closed curve
		{
			BBy[NB2 - 1] = control_point( BBy[0], BBy[NB2 - 3], BBy[3], acoef );
			BBy[0 + 1] = control_point( BBy[0], BBy[3], BBy[NB2 - 3], acoef );

			// last element  NB2-3
			BBy[NB2 - 4] = control_point( BBy[NB2 - 3], BBy[NB2 - 6], BBy[0], acoef );
			BBy[NB2 - 2] = control_point( BBy[NB2 - 3], BBy[0], BBy[NB2 - 6], acoef );
		}
		else // unclosed curve
		{
			BBy[1] = BBy[0];
			BBy[NB2 - 4] = BBy[NB2 - 6];
			BBy[NB2 - 2] = BBy[NB2 - 1] = BBy[NB2 - 3];
		}

		for( int32_t i = 0, j = 0; i < NB2; i += 3, ++j )
		{
			double v2 = (BBy[i] + BBy[i + 1] + BBy[i + 2]) / 3.0;
			contourY[j] = static_cast<uint32_t>(std::round( v2 ));
		}
	}

	void GetRangeOfInterest( const Histogram& hist, uint& begin, uint& end, const double ratio )
	{
		// Get the max vector of the histogram
		const uint32_t max = hist.maximum( begin, end );

		// Round the coeficient to get a usable 'unsigned int' value
		const uint coef = std::round( ratio * max );

		// Get the lower bound and upper bound according to the coeficient
		// note: we could also use  std::bind2nd( std::greater<int>(), coef )
		auto lower = std::find_if( hist.cbegin() + begin, hist.cbegin() + end,
		                           [ & ]( const uint& elem ) -> bool
		                           { return elem >= coef; } );

		auto upper = std::find_if( hist.crbegin() + (hist.size() - end),
		                           hist.crbegin() + (hist.size() - begin),
		                           [ & ]( const uint& elem ) -> bool
		                           { return elem >= coef; } );

		// Get the number of elements between the boundaries
		uint count = std::distance( lower, upper.base());
		assert( count > 0 and "There should be at least one element within the boundaries" );

		assert( lower - hist.cbegin() >= begin );
		assert( upper.base() - hist.cbegin() <= end );

		begin = lower - hist.cbegin();
		end = upper.base() - hist.cbegin();
	}

	/// Histogram normalization
	void VectorProba( const Histogram& histogram, std::vector<double>& prob, const uint32_t begin,
	                  const uint32_t end )
	{
		assert( histogram.size() == prob.size() && "Histogram should have the same size" );

		double
			sumElems = std::accumulate( histogram.cbegin() + begin, histogram.cbegin() + end, 0 );

		for( size_t i = begin; i < end; ++i )
		{
			prob[i] = static_cast<double>(histogram[i]) / sumElems;
		}
	}

	double Mean( const std::vector<double>& prob, const uint32_t begin, const uint32_t end )
	{
		double mean{ };
		for( uint32_t i = begin; i < end; ++i )
		{
			mean += (i * prob[i]);
		}
		return mean;
	}

	double Momentum( const std::vector<double>& prob, const double mean, int Grade,
	                 const uint32_t begin, const uint32_t end )
	{
		double result{ };
		for( uint32_t i = begin; i < end; ++i )
		{
			result += cl::math::Pow( i - mean, static_cast<double>(Grade)) * prob[i];
		}
		return result;
	}

	double ProbAcum( const std::vector<double>& prob, const uint32_t begin, const uint32_t end )
	{
		double proba{ };
		for( uint32_t i = begin; i < end; ++i )
		{
			proba += prob[i];
		}
		return proba;
	}

	void ReduceMin( const Histogram& hist, std::vector<uint32_t>& thresholds,
	                uint32_t SeuilsSouhaites )
	{
		size_t No{ thresholds.size() };
		uint32_t min{ 255 };

		// Prob. entre Intervals
		std::vector<uint32_t> prob( 255 );

		for( uint32_t i = 0, j = 0; i < min; ++i )
		{
			if( thresholds[j] < i && j < No )
			{
				++j;
			}

			prob[j] += hist[i];
		}

		while( No > SeuilsSouhaites )
		{
			uint32_t min{ };

			for( uint32_t i = 1; i <= No; ++i )
			{
				if( prob[i] < prob[min] )
				{
					min = i; // l'area minimum
				}
			}

			if( min == 0 )   // Au debut=> Elimination du premier seuil
			{
				prob[min] += prob[min + 1];
				for( uint32_t i = 1; i < No; ++i )
				{
					prob[i] = prob[i + 1];
					thresholds[i - 1] = thresholds[i];
				}
			}
			else if( min == No ) // A la fin=>Elimination du dernier seuil
			{
				prob[min - 1] += prob[min];
			}
			else // Au milieu
			{
				if( prob[min + 1] > prob[min - 1] )
				{
					prob[min - 1] += prob[min];
					for( uint32_t i = min; i < No; ++i )
					{
						prob[i] = prob[i + 1];
						thresholds[i - 1] = thresholds[i];  // Elimination du seuil anterieur
					}
				}
				else
				{
					prob[min] += prob[min + 1];
					for( uint32_t i = min + 1; i < No; ++i )
					{
						prob[i] = prob[i + 1];
						thresholds[i - 1] = thresholds[i];  // Elimination du seuil actuel
					}
				}
			}
			--No;
		}
	}
	//
	//#define DEBUG_MAZOUT

	void compute_thresholds( const Histogram& histogram, uint32_t begin, uint32_t end,
	                         std::vector<uint32_t>& tresholds )
	{
		int32_t seuil{ -1 };
		double Si1{ }, Si2{ }, j{ };
		double p1{ }, p2{ };
		double sig1{ }, sig2{ }, control{ cl::math::max_limit<double>() };

		// Probability array
		std::vector<double> prob( 255, 0 );

#ifdef DEBUG_MAZOUT
		cl::print_line();
		cl::print_line( "before ", begin, " ", end );
#endif

		// Shrink the histogram and keep the interesting values
		//GetRangeOfInterest( histogram, begin, end, 0.08 );

		//cl::print_line( "after GetRangeOfInterest ", begin, " ", end );

		// Normalized histogram
		VectorProba( histogram, prob, begin, end );

		//cl::print_container( prob );

		double muT = Mean( prob, begin, end );
		double sigT = Momentum( prob, muT, 2, begin, end );

		while( histogram[begin] == 0 )
		{
			++begin;
		}
		while( histogram[end] == 0 )
		{
			--end;
		}

#ifdef DEBUG_MAZOUT
		cl::print_line( "after zeros begin: ", begin, " end: ", end );
		#endif

		for( uint32_t i = begin; i < end; ++i )
		{
			double proba1 = ProbAcum( prob, begin, i );
			double proba2 = 1 - proba1;

			if( !cl::math::is_zero( proba1 ) && !cl::math::is_zero( proba2 ))
			{
				double mu1 = Mean( prob, begin, i ) / proba1;
				sig1 = Momentum( prob, mu1, 2, begin, i ) / proba1;

				double mu2 = Mean( prob, i, end ) / proba2;
				sig2 = Momentum( prob, mu2, 2, i, end ) / proba2;

				if((sig1 <= 0.0) || (sig2 <= 0.0))
				{
					//cl::print_line( " STOP " );
					continue;
				}

				j = CriteryKittler( proba1, proba2, sig1, sig2 );
				//j = CriteryOtsu( proba1, proba2, mu1, muT);
				//j = CriteryPal( proba1, proba2, mu1, mu2, muT);

#ifdef DEBUG_MAZOUT
				cl::print_line( "i: ", i, " -  ", j, " ", proba1, " ", proba2, " ", sig1, " ", sig2 );
#endif

				if( j < control ) // Trouver les valeur minimun
				{
#ifdef DEBUG_MAZOUT
					cl::print_line( " new minimum ", j, " at ", i );
					#endif
					seuil = i; // On choisis le minimum1
					control = j;
					p1 = proba1;
					p2 = proba2;
					Si1 = sig1;
					Si2 = sig2;
				}
			}
		}

		//cl::print_line( seuil );
		//exit(0);

		// Definitions de variances inter/extra-classe
		sig1 = Si1 * p1 + Si2 * p2;
		sig2 = sigT - sig1;
		if( seuil != -1 )
		{

#ifdef DEBUG_MAZOUT
			cl::print_line( muT, " ", sigT );
			cl::print_line( 1.075 * sig1, " ", 1.075 * sig2 );
#endif

			if((1.075 * sig1 < sigT) && (1.075 * sig2 < sigT))
			{
#ifdef DEBUG_MAZOUT
				cl::print_line( " -- we have a treshold at: ", seuil );
				#endif
				tresholds.push_back( static_cast<uint32_t>(seuil));
			}
			else
			{
#ifdef DEBUG_MAZOUT
				cl::print_line( " -- we don't have any treshold" );
				#endif
			}
			if((sigT > 1.5 * sig1) || (sigT > 1.5 * sig2)) // Variance inter_clase
			{
				if( Si1 > Si2 )
				{

#ifdef DEBUG_MAZOUT
					cl::print_line( "left: ", begin, " ", seuil );
					#endif
					compute_thresholds( histogram, begin, seuil, tresholds );
				}
				else
				{
#ifdef DEBUG_MAZOUT
					cl::print_line( "right: ", seuil, " ", end );
					#endif
					compute_thresholds( histogram, seuil, end, tresholds );
				}
			}

		}
	}

	/// Compute the vector of class separation
	void thresholding( const cv::Mat& channel, std::vector<uint32_t>& thresholds )
	{
		Histogram histogram( 255 );
		histogram.from_channel( channel );

		Histogram smoothed( 255 );
		BezierContourSmoothing( histogram, smoothed, 50 );

		compute_thresholds( smoothed, 0, 255, thresholds );

		std::sort( thresholds.begin(), thresholds.end());

		// Reduction des minima selon le critere
		ReduceMin( smoothed, thresholds, 5 );
		thresholds.resize( 5 );

		cl::print_container( thresholds );

#ifdef DEBUG_MAZOUT
		exit(0);
		#endif

		showHistogramWithThresholds( histogram, "initial", thresholds );
	}

	void compute_classes( const cv::Mat& channel )
	{
		std::vector<uint32_t> thresholds;
		thresholding( channel, thresholds );
	}

//--Data members------------------------------------------------------------------------------------
private:
};


//==================================================================================================
// I N L I N E   F U N C T I O N S   C O D E   S E C T I O N

#endif  // CLASSEXTRACTION_HPP
