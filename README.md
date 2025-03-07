# Vulkan Compute Shader

## Windows Users

Install Visual Studio and open `vulkan-compute-shader.sln`.

## Prerequisites (Linux)

Install vcpkg:

```
$ sudo apt install build-essential pkg-config cmake curl zip unzip tar ninja-build
$ sudo apt install libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev
$ sudo apt install libxrandr-dev libxi-dev

$ git clone https://github.com/microsoft/vcpkg.git
$ cd vcpkg
$ ./bootstrap-vcpkg.sh
$ echo 'export VCPKG_ROOT=$HOME/vcpkg' >> ~/.bashrc
$ echo 'export PATH=$PATH:$VCPKG_ROOT' >> ~/.bashrc
$ source ~/.bashrc
$ vcpkg integrate install
```

Install vulkan-validationlayers for debugging:

```
$ sudo apt install vulkan-validationlayers
```

## Project 1: hello-vkinfo

```
$ cd hello-vkinfo
$ cmake -B build --preset vcpkg
$ cmake --build build
$ ./build/hello-vkinfo
```

Output:

```
Vulkan API version: 1.3.296

Number of physical devices: 2

Available devices:
Device name: Intel(R) Arc(TM) Graphics
Device type: Integrated GPU
API version: 1.3.277

Device name: NVIDIA RTX 1000 Ada Generation Laptop GPU
Device type: Discrete GPU
API version: 1.3.277
```
