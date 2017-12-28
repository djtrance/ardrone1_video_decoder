gcc testDrone_video.c `pkg-config --cflags --libs opencv` -lm -o ardroneCV

echo "Test ardroneCV"
