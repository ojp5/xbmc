#pragma once
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

#include "system.h"

#include "Screenshot.h"
#include "guilib/Geometry.h"

#include "cores/VideoRenderers/BaseRenderer.h"

namespace PERIPHERALS
{
  struct AverageRGB {
    float r,g,b;
  };

  struct RGB {
    BYTE r,g,b;
  };

  struct BGRA {
    BYTE b,g,r,a;
  };

  struct Tile
  {
    unsigned int      m_x;
    unsigned int      m_y;
    CRect             m_sampleRect;
    RGB               m_rgb;
  };

  struct TileData {
    BYTE *stream;
    DWORD streamLength; 
  };

  class CAmbiPiGrid
  {
  public:
    CAmbiPiGrid(unsigned int width, unsigned int height);
    ~CAmbiPiGrid(void);
    void UpdateSampleRectangles(unsigned int imageWidth, unsigned int imageHeight);
    void UpdateTilesFromImage(const CScreenshotSurface* pImage);
    TileData *GetTileData(void);

  protected:
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_numTiles;
    Tile* m_tiles;
    TileData m_tileData;

    void UpdateTileCoordinates(unsigned int width, unsigned int height);    

  private:
    unsigned long int CalculatePixelsInTile(Tile *pTile);
    void UpdateSingleTileCoordinates(unsigned int leftTileIndex, unsigned int x, unsigned int y);
    void DumpCoordinates(void);
    void CalculateAverageColorForTile(const CScreenshotSurface* pSurface, Tile *pTile);
    void UpdateAverageColorForTile(Tile *pTile, const AverageRGB *pAverageRgb);
    void UpdateAverageRgb(const RGB *pRgb, unsigned long int numPixels, AverageRGB *pAverageRgb);
  };
}
