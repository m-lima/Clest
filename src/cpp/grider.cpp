#ifdef MODE_GRIDER

#include "las/GridFile.hpp"

int main() {
  las::LASFile<3> las("D:/Users/Marcelo/Documents/PointCloud/Liebas/Spool Dense/Railing.las");
  las.loadHeaders();
  las.loadData();

  grid::GridFile grid(las, 256, 256, 256);

  return 0;
}

#endif