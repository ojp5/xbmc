/*
 *      Copyright (C) 2005-2012 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "AmbiPiGrid.h"
#include "utils/log.h"

using namespace PERIPHERALS;

CAmbiPiGrid::~CAmbiPiGrid()
{
  free(m_tiles);
  free(m_tileData.stream);
}

#define SIZE_OF_X_AND_Y_COORDINATE 2
CAmbiPiGrid::CAmbiPiGrid(unsigned int width, unsigned int height) :
  m_width(width),
  m_height(height)
{  
  
  m_numTiles = (m_width * 2) + ((m_height - 2) * 2);
  unsigned int allocSize = m_numTiles * sizeof(Tile);
  m_tiles = (Tile*)malloc(allocSize);
  ZeroMemory(m_tiles, allocSize);


  m_tileData.streamLength = sizeof(m_tileData.streamLength) + (m_numTiles * (SIZE_OF_X_AND_Y_COORDINATE + sizeof(RGB)));
  m_tileData.stream = (BYTE *)malloc(m_tileData.streamLength);

  UpdateTileCoordinates(width, height);
  DumpCoordinates();
}

TileData *CAmbiPiGrid::GetTileData(void)
{
  BYTE *pStream = m_tileData.stream;

  ZeroMemory(pStream, m_tileData.streamLength);

  int tileIndex = m_numTiles - 1;
  Tile* pTile;

  *pStream++ = (BYTE) (m_tileData.streamLength >> 24 & 0xFF);
  *pStream++ = (BYTE) (m_tileData.streamLength >> 16 & 0xFF);
  *pStream++ = (BYTE) (m_tileData.streamLength >> 8 & 0xFF);
  *pStream++ = (BYTE) (m_tileData.streamLength >> 0 & 0xFF);

  while (tileIndex >= 0) {
    pTile = m_tiles + tileIndex;

    *pStream++ = (BYTE)pTile->m_x;
    *pStream++ = (BYTE)pTile->m_y;

    *pStream++ = pTile->m_rgb.r;
    *pStream++ = pTile->m_rgb.g;
    *pStream++ = pTile->m_rgb.b;

    tileIndex--;
  }  

  unsigned int bytesAddedToStream = pStream - m_tileData.stream;

  return &m_tileData;
}

void CAmbiPiGrid::UpdateTileCoordinates(unsigned int width, unsigned int height)
{
  unsigned int start_position_x = width / 2;

  unsigned int zeroBasedY;
  unsigned int zeroBasedX;

  unsigned int x;
  unsigned int y;

  unsigned int tileIndex = 0;

  for (y = 1; y <= height; y++)
  {
    zeroBasedY = y - 1;
    if (y == 1) 
    {
      // ProcessTopEdgeFromCenterToLeft
      for (x = start_position_x; x > 0; x--)
      {
        zeroBasedX = x - 1;
        UpdateSingleTileCoordinates(tileIndex++, zeroBasedX, zeroBasedY);
      }
    } 
    else if (y == height)
    {
      // ProcessBottomEdge
      for (x = 1; x < width; x++)
      {
        zeroBasedX = x - 1;
        UpdateSingleTileCoordinates(tileIndex++, zeroBasedX, zeroBasedY);
      }
    }
    else
    {
      // ProcessLeftEdgeOnly
      zeroBasedX = 0;
      UpdateSingleTileCoordinates(tileIndex++, zeroBasedX, zeroBasedY);
    }
  }

  for (y = height; y >= 1; y--)
  {
    zeroBasedY = y - 1;
    if (y == 1)
    {
      // ProcessTopEdgeFromRightToCenter
      for (x = width; x > start_position_x; x--)
      {
        zeroBasedX = x - 1;
        UpdateSingleTileCoordinates(tileIndex++, zeroBasedX, zeroBasedY);
      }
    }
    else
    {
      // ProcessRightEdgeOnly
      zeroBasedX = width - 1;
      UpdateSingleTileCoordinates(tileIndex++, zeroBasedX, zeroBasedY);
    }
  }

  CLog::Log(LOGDEBUG, "%s - leds positions processed: %d", __FUNCTION__, tileIndex);  
}

void CAmbiPiGrid::UpdateSingleTileCoordinates(unsigned int tileIndex, unsigned int x, unsigned int y)
{
  Tile* pTile = m_tiles + tileIndex;
  pTile->m_x = x;
  pTile->m_y = y;
}

void CAmbiPiGrid::DumpCoordinates(void)
{
  unsigned int tileIndex = 0;
  Tile* pTile;

  while (tileIndex < m_numTiles) {

    pTile = m_tiles + tileIndex;
    CLog::Log(LOGDEBUG, "%s - Coordinates, index: %d, x: %d, y: %d", __FUNCTION__, tileIndex, pTile->m_x, pTile->m_y);  

    tileIndex++;
  }  
}

void CAmbiPiGrid::UpdateSampleRectangles(unsigned int imageWidth, unsigned int imageHeight)
{
  unsigned int tileIndex = 0;
  Tile* pTile;

  unsigned int sampleWidth = imageWidth / m_width;
  unsigned int sampleHeight = imageHeight / m_height;

  CLog::Log(LOGDEBUG, "%s - Updating grid sample rectangles for image, width: %d, height: %d, sampleWidth: %d, sampleHeight: %d", __FUNCTION__, imageWidth, imageHeight, sampleWidth, sampleHeight);  

  while (tileIndex < m_numTiles) {

    pTile = m_tiles + tileIndex;

    DetermineSampleRectangle(pTile, imageWidth, imageHeight, sampleWidth, sampleHeight);
    GrowSampleRectangleInwards(pTile, sampleWidth, sampleHeight);

    CLog::Log(LOGDEBUG, "%s - updated rectangle, x: %d, y: %d, rect: (left: %f, top: %f, right: %f, bottom: %f)",
      __FUNCTION__, 
      pTile->m_x,
      pTile->m_y,
      pTile->m_sampleRect.x1,
      pTile->m_sampleRect.y1,
      pTile->m_sampleRect.x2,
      pTile->m_sampleRect.y2
    );  

    tileIndex++;
  }
}

void CAmbiPiGrid::DetermineSampleRectangle(Tile *pTile, unsigned int imageWidth, unsigned int imageHeight, unsigned int sampleWidth, unsigned int sampleHeight)
{
  pTile->m_sampleRect.SetRect(
    (float)pTile->m_x * sampleWidth,
    (float)pTile->m_y * sampleHeight,
    (float)std::min(imageWidth, (pTile->m_x * sampleWidth) + sampleWidth),
    (float)std::min(imageHeight, (pTile->m_y * sampleHeight) + sampleHeight)
  );
}

void CAmbiPiGrid::GrowSampleRectangleInwards(Tile *pTile, unsigned int sampleWidth, unsigned int sampleHeight)
{
  int multiplier = 2;

  if (pTile->m_y == 0) {
    pTile->m_sampleRect.y2 += (sampleHeight * multiplier);
  } else if (pTile->m_y == m_height - 1) {
    pTile->m_sampleRect.y1 -= (sampleHeight * multiplier);
  }

  if (pTile->m_x == 0) {
    pTile->m_sampleRect.x2 += (sampleWidth * multiplier);
  } else if (pTile->m_x == m_width - 1) {
    pTile->m_sampleRect.x1 -= (sampleWidth * multiplier);
  }
}

#define PLANE_Y 0
#define PLANE_U 1
#define PLANE_V 2

void CAmbiPiGrid::UpdateTilesFromImage(const CScreenshotSurface* pSurface)
{
  unsigned int tileIndex = 0;
  Tile* pTile;

  while (tileIndex < m_numTiles) {

    pTile = m_tiles + tileIndex;
    CalculateAverageColorForTile(pSurface, pTile);
    tileIndex++;
  }  
}

#define STEP 2

void CAmbiPiGrid::CalculateAverageColorForTile(const CScreenshotSurface* pSurface, Tile *pTile) 
{
  AverageRGB averageRgb = { 0, 0, 0 };
  RGB rgb;
  unsigned long int samplesToTake = CalculatePixelsInTile(pTile) / STEP;

  for (int y = (int)pTile->m_sampleRect.y1; y < pTile->m_sampleRect.y2; y++)
  {
    for (int x = (int)pTile->m_sampleRect.x1; x < pTile->m_sampleRect.x2; x += STEP)
    {
      BGRA *pPixel = (BGRA *)(pSurface->m_buffer) + (y * pSurface->m_width) + x;

      rgb.r = pPixel->r;
      rgb.g = pPixel->g;
      rgb.b = pPixel->b;

      UpdateAverageRgb(&rgb, samplesToTake, &averageRgb);

    }
  }

  UpdateAverageColorForTile(pTile, &averageRgb);
#if 0
  CLog::Log(LOGDEBUG, "%s - average color for tile, x: %d, y: %d, RGB: #%02x%02x%02x",
    __FUNCTION__, 
    pTile->m_x, 
    pTile->m_y, 
    (BYTE)averageRgb.r, 
    (BYTE)averageRgb.g,
    (BYTE)averageRgb.b
  );
#endif
}

unsigned long int CAmbiPiGrid::CalculatePixelsInTile(Tile *pTile) 
{
  return (unsigned long int)((pTile->m_sampleRect.y2 - pTile->m_sampleRect.y1) * (pTile->m_sampleRect.x2 - pTile->m_sampleRect.x1));
}

void CAmbiPiGrid::UpdateAverageRgb(const RGB *pRgb, unsigned long int totalSamples, AverageRGB *pAverageRgb)
{
  pAverageRgb->r += ((float)pRgb->r / totalSamples);
  pAverageRgb->g += ((float)pRgb->g / totalSamples);
  pAverageRgb->b += ((float)pRgb->b / totalSamples);
}

void CAmbiPiGrid::UpdateAverageColorForTile(Tile *pTile, const AverageRGB *pAverageRgb)
{
  pTile->m_rgb.r = (BYTE)pAverageRgb->r;
  pTile->m_rgb.g = (BYTE)pAverageRgb->g;
  pTile->m_rgb.b = (BYTE)pAverageRgb->b;
}
