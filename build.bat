cmake -S. -B./build -DSFPG_RIGHTCLICK=ON
cd build
cmake --build . --config Release
cpack -C Release
cd ..
pause