# Configure
cmake -B build

# Build
cmake --build build

# Run tests
cd build
ctest --output-on-failure
pause