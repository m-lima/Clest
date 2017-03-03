#pragma once

#include <string>

#include "grid_file.hpp"
#include <clest/ostream.hpp>

namespace cube {

  struct Vertex {
    float x, y, z;
    float nx, ny, nz;
    grid::Color color;
  };

  struct Triangle {
    unsigned int v1, v2, v3;
  };

}

namespace grid {
  class CubeMarcher {
  public:
    CubeMarcher(const grid::GridFile * const grid) :
      mGrid(grid) {}

    inline void setOriginalStyle(bool original) {
      mOriginalStyle = original;
    }

    inline bool isOriginalStyle() {
      return mOriginalStyle;
    }

    inline void march(float iso) {
      if (iso >= 0.0 && iso <= 100.0) {
        march(static_cast<uint32_t>(mGrid->maxValue * 0.01 * iso));
      } else {
        throw clest::Exception("Iso should be between 0% and 100%");
      }
    }
    
    void march(uint32_t iso);
    void save(const std::string & name);

  private:
    void generateIntersections(uint32_t iso);
   
    const grid::GridFile * const mGrid;
    bool mOriginalStyle = false;
  };
}
