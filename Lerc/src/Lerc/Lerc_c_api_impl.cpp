/*
Copyright 2016 Esri

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

A local copy of the license and additional notices are located with the
source distribution at:

http://github.com/Esri/lerc/

Contributors:  Thomas Maurer
*/

#include "../Include/Lerc_c_api.h"
#include "../Include/Lerc_types.h"
#include "Lerc.h"

using namespace LercNS;

// -------------------------------------------------------------------------- ;

lerc_status lerc_computeCompressedSize(const void* pData, unsigned int dataType, int nCols, int nRows, int nBands, 
  const unsigned char* pValidBytes, double maxZErr, unsigned int* numBytes)
{
  if (!pData || dataType >= Lerc::DT_Undefined || nCols <= 0 || nRows <= 0 || nBands <= 0 || maxZErr < 0 || !numBytes)
    return (lerc_status)ErrCode::WrongParam;

  BitMask bitMask;
  if (pValidBytes)
  {
    bitMask.SetSize(nCols, nRows);
    bitMask.SetAllValid();

    for (int k = 0, i = 0; i < nRows; i++)
      for (int j = 0; j < nCols; j++, k++)
        if (!pValidBytes[k])
          bitMask.SetInvalid(k);
  }
  const BitMask* pBitMask = pValidBytes ? &bitMask : 0;

  Lerc::DataType dt = (Lerc::DataType)dataType;
  return (lerc_status)Lerc::ComputeCompressedSize(pData, dt, nCols, nRows, nBands, pBitMask, maxZErr, *numBytes);
}

// -------------------------------------------------------------------------- ;

lerc_status lerc_encode(const void* pData, unsigned int dataType, int nCols, int nRows, int nBands, 
  const unsigned char* pValidBytes, double maxZErr,
  unsigned char* pOutBuffer,           // buffer to write to, function will fail if buffer too small
  unsigned int outBufferSize,          // size of output buffer
  unsigned int* nBytesWritten)         // number of bytes written to output buffer
{
  if (!pData || dataType >= Lerc::DT_Undefined || nCols <= 0 || nRows <= 0 || nBands <= 0 || maxZErr < 0 || !pOutBuffer || !outBufferSize || !nBytesWritten)
    return (lerc_status)ErrCode::WrongParam;

  BitMask bitMask;
  if (pValidBytes)
  {
    bitMask.SetSize(nCols, nRows);
    bitMask.SetAllValid();

    for (int k = 0, i = 0; i < nRows; i++)
      for (int j = 0; j < nCols; j++, k++)
        if (!pValidBytes[k])
          bitMask.SetInvalid(k);
  }
  const BitMask* pBitMask = pValidBytes ? &bitMask : 0;

  Lerc::DataType dt = (Lerc::DataType)dataType;
  return (lerc_status)Lerc::Encode(pData, dt, nCols, nRows, nBands, pBitMask, maxZErr, pOutBuffer, outBufferSize, *nBytesWritten);
}

// -------------------------------------------------------------------------- ;

lerc_status lerc_getBlobInfo(const unsigned char* pLercBlob, unsigned int blobSize, 
  unsigned int* infoArray, double* dataRangeArray, int infoArraySize, int dataRangeArraySize)
{
  if (!pLercBlob || !blobSize || !infoArray || infoArraySize < 7 || !dataRangeArray || dataRangeArraySize < 3)
    return (lerc_status)ErrCode::WrongParam;

  Lerc::LercInfo lercInfo;
  ErrCode errCode = Lerc::GetLercInfo(pLercBlob, blobSize, lercInfo);
  if (errCode != ErrCode::Ok)
    return (lerc_status)errCode;

  infoArray[0] = (unsigned int)lercInfo.version;
  infoArray[1] = (unsigned int)lercInfo.dt;
  infoArray[2] = (unsigned int)lercInfo.nCols;
  infoArray[3] = (unsigned int)lercInfo.nRows;
  infoArray[4] = (unsigned int)lercInfo.nBands;
  infoArray[5] = (unsigned int)lercInfo.numValidPixel;
  infoArray[6] = (unsigned int)lercInfo.blobSize;

  dataRangeArray[0] = lercInfo.zMin;
  dataRangeArray[1] = lercInfo.zMax;
  dataRangeArray[2] = lercInfo.maxZError;

  return (lerc_status)ErrCode::Ok;
}

// -------------------------------------------------------------------------- ;

lerc_status lerc_decode(
  const unsigned char* pLercBlob,     // Lerc blob to decode
  unsigned int blobSize,              // size of Lerc blob in bytes
  unsigned char* pValidBytes,         // gets filled if not 0, even if all valid
  int nCols, int nRows, int nBands,   // number of columns, rows, bands
  unsigned int dataType,              // data type of outgoing array
  void* pData)                        // outgoing data array
{
  if (!pLercBlob || !blobSize || !pData || dataType >= Lerc::DT_Undefined || nCols <= 0 || nRows <= 0 || nBands <= 0)
    return (lerc_status)ErrCode::WrongParam;

  BitMask bitMask;
  if (pValidBytes)
  {
    bitMask.SetSize(nCols, nRows);
    bitMask.SetAllInvalid();
  }
  BitMask* pBitMask = pValidBytes ? &bitMask : 0;

  Lerc::DataType dt = (Lerc::DataType)dataType;

  ErrCode errCode = Lerc::Decode(pLercBlob, blobSize, pBitMask, nCols, nRows, nBands, dt, pData);
  if (errCode != ErrCode::Ok)
    return (lerc_status)errCode;

  if (pValidBytes)
  {
    for (int k = 0, i = 0; i < nRows; i++)
      for (int j = 0; j < nCols; j++, k++)
        pValidBytes[k] = bitMask.IsValid(k);
  }

  return (lerc_status)ErrCode::Ok;
}

// -------------------------------------------------------------------------- ;

