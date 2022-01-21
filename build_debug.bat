cmake -S. -B./build -DSFPG_RIGHTCLICK=ON
cd build
cmake --build . --config Debug
cpack -C Debug
cd ..
pause