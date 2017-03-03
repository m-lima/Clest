#include "cube_marcher.hpp"

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/spin_mutex.h>

namespace {
  inline float getValue(uint32_t smaller, uint32_t bigger, uint32_t iso) {
    if (smaller > iso) {
      if (bigger <= iso) {
        return (smaller - iso) / static_cast<float>(smaller - bigger);
      } else {
        return -1.0f;
      }
    } else {
      if (bigger > iso) {
        return (iso - smaller) / static_cast<float>(bigger - smaller);
      } else {
        return -1.0f;
      }
    }
  }

  class VertexGenerator {
  private:
    const uint16_t sizeX;
    const uint16_t sizeY;
    const uint16_t sizeZ;
    const uint16_t sizeXZ = sizeX * sizeZ;
    const uint16_t sizeXY = sizeX * sizeY;
    const uint16_t sizeYZ = sizeY * sizeZ;
    
    const uint32_t * grid = mGrid->cDataRaw()->data();
    const uint32_t * indices = mVertexIndices.data();

    tbb::spin_mutex * const mutex;

  public:
    VertexGenerator(uint16_t sizeX,
                    uint16_t sizeY,
                    uint16_t sizeZ,
                    const uint32_t * const grid,
                    ) = default;

    void operator()(uint16_t z) {
      for (uint16_t y = 0; y < sizeY - 1; ++y) {
        for (uint16_t x = 0; x < sizeX - 1; ++x) {
        }
      }
      //auto x = index / mSizeYZ;
      //auto y = (index / mSizeZ) % mSizeY;
      //auto z = index % mSizeZ;

      //if (x == 0 || y == 0 || z == 0) {
      //  return;
      //}

      size_t
      uint32_t v4 = grid[(index - sizeZ) - sizeYZ];
      uint32_t v5 = grid[index - sizeZ];
      uint32_t v6 = grid[index];
      uint32_t v7 = grid[index - sizeYZ];
      index--;
      uint32_t v0 = grid[(index - sizeZ) - sizeYZ];
      uint32_t v1 = grid[index - sizeZ];
      uint32_t v2 = grid[index];
      uint32_t v3 = grid[index - sizeYZ];

      float value;
      size_t vertexIndex;
      cube::Vertex vertex{ x, y, z, x, y, z, 0, 0, 0 };

      // E: 5
      if ((value = getValue(v5, v6, iso)) != -1.0f) {
        vertex.y += value;

        vertexIndex = mVerticesY.size();
        mVerticesY.emplace_back(vertex);

        indices[z * mSizeXY + y] = vertexIndex;
        vertex.y = y;
      }

      // E: 6
      if ((value = getValue(v7, v6, iso)) != -1.0f) {
        vertex.x += value;

        vertexIndex = mVerticesX.size();
        mVerticesX.emplace_back(vertex);

        indices[z * mSizeXY + y * mSizeYZ + x] = vertexIndex;
        vertex.x = x;
      }

      // E: 10
      if ((value = getValue(v2, v6, iso)) != -1.0f) {
        vertex.z += value;

        mutex->lock();
        vertexIndex = mVerticesZ.size();
        mVerticesZ.emplace_back(vertex);
        mutex->unlock();

        indices[z] = vertexIndex;
        vertex.z = z;
      }
    }
  };
}

namespace grid {

  void CubeMarcher::generateIntersections(uint32_t iso) {

    tbb::spin_mutex mutexX;
    tbb::spin_mutex mutexY;
    tbb::spin_mutex mutexZ;

    auto length = mGrid->cDataRaw()->size();
    const auto grid = mGrid->cDataRaw()->data();
    const auto indices = mVertexIndices.data();

    tbb::parallel_for(uint16_t(0), uint16_t(mSizeZ - 1), VertexGenerator());
  }

  void CubeMarcher::march(uint32_t iso) {
    generateIntersections(iso);
  }

  void CubeMarcher::save(const std::string & name) {}

}
