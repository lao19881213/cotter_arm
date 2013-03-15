#include "offringaweightcol.h"
#include "offringastman.h"
#include "weightencoder.h"
#include "bytepacker.h"
#include "offringastmanerror.h"

namespace offringastman
{

OffringaWeightColumn::~OffringaWeightColumn()
{
	delete _encoder;
	delete[] _packBuffer;
}

void OffringaWeightColumn::setShapeColumn(const casa::IPosition& shape)
{
	_shape = shape;
}

void OffringaWeightColumn::getArrayfloatV(casa::uInt rowNr, casa::Array<float>* dataPtr)
{
	readCompressedData(rowNr, _packBuffer);
	
	BytePacker::unpack(_bitsPerSymbol, &_symbolBuffer[0], _packBuffer + sizeof(float), _symbolsPerCell);
	
	_encoder->Decode(_dataCopyBuffer, *reinterpret_cast<float*>(_packBuffer), _symbolBuffer);
	
	std::vector<float>::const_iterator j = _dataCopyBuffer.begin();
	for(casa::Array<float>::contiter i = dataPtr->cbegin(); i!= dataPtr->cend(); ++i)
	{
		*i = *j;
		++j;
	}
}

void OffringaWeightColumn::putArrayfloatV(casa::uInt rowNr, const casa::Array<float>* dataPtr)
{
	std::vector<float>::iterator j = _dataCopyBuffer.begin();
	for(casa::Array<float>::const_contiter i = dataPtr->cbegin(); i!= dataPtr->cend(); ++i)
	{
		*j = *i;
		++j;
	}
	_encoder->Encode(*reinterpret_cast<float*>(_packBuffer), _symbolBuffer, _dataCopyBuffer);
	
	BytePacker::pack(_bitsPerSymbol, _packBuffer + sizeof(float), &_symbolBuffer[0], _symbolsPerCell);
	
	writeCompressedData(rowNr, _packBuffer);
}

void OffringaWeightColumn::Prepare()
{
	if(_bitsPerSymbol == 0)
		throw OffringaStManError("bitsPerSymbol not initialized in OffringaWeightCol");
	
	delete _encoder;
	_encoder = new WeightEncoder<float>(1<<_bitsPerSymbol);
	
	_symbolsPerCell = 1;
	for(casa::IPosition::const_iterator i = _shape.begin(); i!=_shape.end(); ++i)
		_symbolsPerCell *= *i;
	
	delete[] _packBuffer;
	_packBuffer = new unsigned char[Stride()];
	_symbolBuffer.resize(_symbolsPerCell);
	_dataCopyBuffer.resize(_symbolsPerCell);
}

} // end of namespace
