#include <string>
#include <fstream>
#include <fmt/ostream.h>

#include "LASFile.hpp"

namespace {

  /// Pads the bytes from the header with zero in case the LAS
  /// version being used has a smaller size
  void _cleanupHeader(las::PublicHeader & header) {

    // Check if it is smaller than the maximum size
    if (header.headerSize < sizeof(las::PublicHeader)) {

      // Treat as bytes
      char * rawBytes = reinterpret_cast<char*>(&header);

      // Pad with zeros
      for (int i = header.headerSize + 1; i < sizeof(las::PublicHeader); i++) {
        rawBytes[i] = 0;
      }
    }
  }

  /// Loads the points from the LAS file into memory and returns
  /// the cardinality of the set of points loaded
  ///
  /// The funtion splits of before the main loop wheter a chunk
  /// is being loaded or the whole data
  ///
  /// The chunk can be defined as coordinates or as a cap by
  /// using `max`. Note that there is no `min`
  template <typename T>
  uint64_t _loadData(
    uint16_t typeSize,
    std::ifstream & in,
    std::vector<T> & container,
    uint64_t max,
    const las::Limits<uint32_t> & limits
  ) {
    constexpr uint16_t BUFFER_SIZE = 8192;

    // If `BUFFER_SIZE` cannot hold a single `T` element, throw 
    if (BUFFER_SIZE < typeSize) {
      throw std::runtime_error(
        fmt::format("BUFFER_SIZE ({}) is too small to fit typeSize ({})",
                    BUFFER_SIZE,
                    typeSize));
    }

    // Clean up the container
    // Even if loading chunks, the memory is supposed to be capped
    container = std::vector<T>();
    container.reserve(max);

    // Prepare for reading
    uint64_t count = 0;
    uint64_t iCount = 0;
    T *base;
    uint16_t blockSize = BUFFER_SIZE - (BUFFER_SIZE % typeSize);
    char data[BUFFER_SIZE];

    // Split if using coordinate-based chunking
    if (limits.isMaxed()) { // Not chunked

      // While the stream is healthy and the `max` limit has not been reached
      while (count < max && in.good()) {

        // Load bytes into memory
        in.read(data, blockSize);

        // Iterate the buffer in `sizeof(T)` steps
        for (size_t i = 0; i < blockSize && count < max; i += typeSize) {
          base = reinterpret_cast<T*>(data + i);
          count++;
          container.push_back(*base);
          iCount++;
        }
      }
    } else { // Chunked
      while (count < max && in.good()) {

        // Load bytes into memory
        in.read(data, blockSize);

        // Iterate the buffer in `sizeof(T)` steps
        for (size_t i = 0; i < blockSize && count < max; i += typeSize) {
          base = reinterpret_cast<T*>(data + i);
          count++;

          // Only insert if it is within limits
          if (limits.isOutside(base->x, base->y, base->z)) { continue; }

          container.push_back(*base);
          iCount++;
        }
      }
    }
    container.shrink_to_fit();
    return iCount;
  }
}

namespace las {

  /// Trivial constructor
  template <typename T>
  LASFile<T>::LASFile(const std::string & file)
    : filePath(std::move(file)) {}

  /// Loads the public and variable length record from file
  template <typename T>
  void LASFile<T>::loadHeaders() {
    // Cannot proceed if the file is not readable
    std::ifstream fileStream(filePath,
                             std::ifstream::in | std::ifstream::binary);
    if (!fileStream.is_open()) {
      throw std::runtime_error(
        fmt::format("Could not open file {}", filePath));
    }

    // Read directly into the variable
    fileStream.read(reinterpret_cast<char*>(&publicHeader),
                    sizeof(PublicHeader));

    // Cleanup, since not all headers have the same length
    _cleanupHeader(publicHeader);

    // Seek to the end of the actual header
    fileStream.seekg(publicHeader.headerSize);

    // Load variable length records
    if (publicHeader.numberOfVariableLengthRecords > 0) {

      // Pre-load the `std::vector` so that a direct reference can be used
      recordHeaders.resize(publicHeader.numberOfVariableLengthRecords);

      for (auto & header : recordHeaders) {

        // Read the raw base size. Note that `sizeof()` cannot be used here
        // because of the variable length `std::vector` memory position
        fileStream.read(reinterpret_cast<char*>(&header),
                        RecordHeader::RAW_SIZE);

        // Establish how many bytes to read for this variable record
        uint16_t bytesToRead = header.recordLengthAfterHeader;

        // Pre-load the vector and read straight into it
        header.data.resize(bytesToRead);
        fileStream.read(header.data.data(), bytesToRead);
      }
    }

    // Establish the actual point data count
    // Depending on the version of the LAS, it could be stored in different
    // variables
    _pointDataCount = publicHeader.legacyNumberOfPointRecords > 0
      ? publicHeader.legacyNumberOfPointRecords
      : publicHeader.numberOfPointRecords;

    fileStream.close();
  }

  /// Load the point data based on the limits provided
  /// If no limits are provided, a maxed out limit object will be defaulted
  ///
  /// If `T` is the same as the LAS file point data, a direct load
  /// will be performed if `limits.isMaxed()`
  template <typename T>
  uint64_t LASFile<T>::loadData(const Limits<uint32_t> & limits) {

    // Check filename sanity
    std::ifstream fileStream(filePath,
                             std::ifstream::in | std::ifstream::binary);
    if (!fileStream.is_open()) {
      throw std::runtime_error(
        fmt::format("Could not open file {}", filePath));
    }

    // Go to the point where the point data starts
    fileStream.seekg(publicHeader.offsetToPointData);
    
    uint64_t size;

    // Load directly if `T` matches what's stored in file
    if (limits.isMaxed() && publicHeader.pointDataRecordFormat == T::FORMAT) {
        pointData.resize(_pointDataCount);
        fileStream.read(reinterpret_cast<char*>(&pointData[0]),
                        _pointDataCount * sizeof(T));
        pointData.shrink_to_fit();
        size = pointData.size();
    } else {

      // Call the actual iterator and loader
      size = _loadData(
        publicHeader.pointDataRecordLength,
        fileStream,
        pointData,
        _pointDataCount,
        limits
      );
    }

    fileStream.close();
    return size;
  }

  /// Returns the number of points in this LAS file as per the public header
  /// loading
  /// If called before loading the header, the behavior is undefined
  template <typename T>
  uint64_t LASFile<T>::pointDataCount() const {
    return _pointDataCount;
  }

  /// Saves the LAS file into the file specified by `file`
  /// The function is overloaded with the default parameter being the
  /// `filePath` const std::string from construction
  ///
  /// If the file already exists, it will append a ".new" before the extension
  /// If the specified file exists and does not have a extension, an extension
  /// will be added before attempting to add ".new"
  template<typename T>
  void LASFile<T>::save(std::string file) const {

    // Check if the file exists
    // Keep appending ".new" before the extension until the filename is unique
    do {

      // Try to read it
      std::ifstream testFile(file, std::ifstream::in);

      // If it's not readable, it doesn't exits; Quit loop
      if (!testFile.is_open()) {
        break;
      }

      testFile.close();

      // Find the extension
      auto index = file.rfind(".las");

      // If it has no extension, first try adding an extension to it
      if (index == std::string::npos) { file += ".las"; }
      // If it does, append ".new" before the extension
      else { file = file.substr(0, index) + ".new.las"; }
    } while (true);

    // Prepare for writing
    std::ofstream fileStream(file, std::ofstream::out | std::ofstream::binary);

    if (!fileStream.is_open()) {
      throw std::runtime_error(
        fmt::format("Could not open file {}", filePath));
    }

    // Write the public header directly, based on `headerSize`
    // It is safe to const_cast because this will not alter the data
    fileStream.write(
      reinterpret_cast<char*>(const_cast<PublicHeader*>(&publicHeader)),
      publicHeader.headerSize);

    // Iterate the veriable length records and write them directly
    for (auto & header : recordHeaders) {
      fileStream.write(
        reinterpret_cast<char*>(const_cast<RecordHeader*>(&header)),
        RecordHeader::RAW_SIZE);
      fileStream.write(header.data.data(), header.recordLengthAfterHeader);
    }

    // No buffering being done. The vector is written directly
    fileStream.write(
      reinterpret_cast<const char*>(&pointData[0]),
      pointData.size() * sizeof(T));

    fileStream.close();
  }

  /// Checks the health of the public header by checking the signature
  /// and the length of the header
  template <typename T>
  bool LASFile<T>::isValid() const {

    // Should start with LASF
    char reference[] = "LASF";
    for (int i = 0; i < publicHeader.fileSignature.size(); i++) {
      if (publicHeader.fileSignature[i] != reference[i]) {
        return false;
      }
    }

    // LAS specification up to 1.4
    if (publicHeader.headerSize < 227 || publicHeader.headerSize > 375) {
      return false;
    }

    return true;
  }

  template class LASFile<PointData<-1>>;
  template class LASFile<PointData<0>>;
  template class LASFile<PointData<1>>;
  template class LASFile<PointData<2>>;
  template class LASFile<PointData<3>>;

}
