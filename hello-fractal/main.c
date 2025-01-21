#include <GLFW/glfw3.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <stdio.h>
#include <time.h>
#include <string.h>

#include "compute.h"
#include "instance.h"
#include "device.h"
#include "pipeline.h"
#include "memory.h"

#define width 1000
#define height 1000

uint32_t InputData[width];
uint32_t OutputData[height][width];

void generate()
{
    for (uint32_t row = 0; row < height; row++)
    {
        for (uint32_t col = 0; col < width; col++)
        {
            float r = (float)row / 500.0 - 1.0;
            float i = (float)col / 500.0 - 1.0;

            uint32_t cnt = 0;
            while (((r * r + i * i) < 4.0) && (cnt < 63))
            {
                float temp = r * r - i * i + 0.17;
                i = 2 * r * i + 0.57;
                r = temp;
                cnt++;
            }
            OutputData[row][col] = (cnt << 10) | 0xff000000;
        }
    }
}

int main(int ac, char** av)
{
    if (!glfwInit()) {
        return -1;
    }

    CreateInstance();
    GetPhysicalDevice();
    CreateDeviceAndComputeQueue();
    CreatePipeline();
    CreateDescriptorSet();
    CreateBuffers(sizeof(InputData), sizeof(OutputData));
    CreateCommandPool();
    PrepareCommandBuffer();
    CopyToInputBuffer(InputData, sizeof(InputData));

    double time = glfwGetTime();
    generate();
    time = glfwGetTime() - time;
    printf("CPU fractal: %f ms.\n", time * 1000.0f);

    // Allocate memory for the image data in RGBA format (4 channels per pixel)
    unsigned char* image_data = (unsigned char*)malloc(width * height * 4);

    // Convert the uint32_t color data into RGBA format
    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            uint32_t color = OutputData[row][col];

            // Extract each channel from the 32-bit packed color
            image_data[(row * width + col) * 4 + 0] = (color >> 16) & 0xFF;  // Red
            image_data[(row * width + col) * 4 + 1] = (color >> 8) & 0xFF;   // Green
            image_data[(row * width + col) * 4 + 2] = color & 0xFF;          // Blue
            image_data[(row * width + col) * 4 + 3] = (color >> 24) & 0xFF;  // Alpha
        }
    }
    stbi_write_png("fractal_cpu.png", width, height, 4, image_data, width * 4);

    memset(OutputData, 0, sizeof(OutputData));
    time = glfwGetTime();
    Compute();
    time = glfwGetTime() - time;
    printf("GPU fractal: %f ms.\n", time * 1000.0f);

    CopyFromOutputBuffer(OutputData, sizeof(OutputData));

    memset(image_data, 0, sizeof(image_data));

    // Convert the uint32_t color data into RGBA format
    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            uint32_t color = OutputData[row][col];

            // Extract each channel from the 32-bit packed color
            image_data[(row * width + col) * 4 + 0] = (color >> 16) & 0xFF;  // Red
            image_data[(row * width + col) * 4 + 1] = (color >> 8) & 0xFF;   // Green
            image_data[(row * width + col) * 4 + 2] = color & 0xFF;          // Blue
            image_data[(row * width + col) * 4 + 3] = (color >> 24) & 0xFF;  // Alpha
        }
    }
    stbi_write_png("fractal_gpu.png", width, height, 4, image_data, width * 4);

    free(image_data);

    DestroyPipeline();
    DestroyCommandPoolAndLogicalDevice();

    return 0;
}
