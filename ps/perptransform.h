#ifndef PERP_TRANSFORM_H
#define PERP_TRANSFORM_H

#include "windowfunction.h"

#include "aocommon/uvector.h"

#include <cmath>
#include <cstring>

class PerpTransform
{
public:
	PerpTransform(size_t outSize) : _outSize(outSize) { }
	
	struct PerpData
	{
		ao::uvector<double> values, weights;
	};
	
	void Transform(PerpData& output, const PerpData& input)
	{
		double totalWeight = 0.0;
		output.values.assign(_outSize, 0);
		
		size_t size = output.values.size()*2 > input.values.size() ? input.values.size() : output.values.size()*2;
		for(size_t i=0; i!=size; ++i)
		{
			double value = input.values[i], weight = input.weights[i];
			if(value != 0.0 && weight != 0.0)
			{
				double window = WindowFunction::blackmanNutallWindow(input.values.size()*2-1, i+input.values.size());
				transformSample(output, i, value * weight * window);
				totalWeight += weight * window;
			}
		}
		
		// Factor of two because of symmetric values (see transformSample()).
		double weightFact = 2.0 / totalWeight;
		for(size_t j=0; j!=output.values.size(); ++j)
		{
			output.values[j] *= weightFact;
		}
		output.weights.assign(_outSize, totalWeight);
	}
	
private:
	void transformSample(PerpData& destination, double x, double f)
	{
		// Calculate F(u) = \int w f(x) e ^ [ -i 2 pi x u / n ], for all u's and the given x, w and f(x).
		double
			phaseFactor = -2.0 * M_PI * x,
			/* The factor 0.5 is here because we do not need the second half of the
			 * frequencies. */
			uFactor = 0.5/destination.values.size(),
			u = 0.0;
		
		for(size_t ui=0; ui!=destination.values.size(); ++ui)
		{
			// We ignore a factor of two (which arised because we are also adding symmetric sample), but
			// is done later
			destination.values[ui] += f * cos(phaseFactor * u);
			u += uFactor;
		}
	}
	
	size_t _outSize;
};

#endif
