#pragma once

#include <iostream>
#include <cstdio>
#include <algorithm>
#include <utility>
#include <queue>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <string>
#include <Windows.h>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "stb_image.h"

const float Pi = acos(-1);
const float eps = 1e-6;
const int INT_INF = 1e9;
const float FLOAT_INF = 1e9;

// Screen Size
const float Width = GetSystemMetrics(SM_CXSCREEN);
const float Height = GetSystemMetrics(SM_CYSCREEN);
const int int_Width = (int)Width;
const int int_Height = (int)Height;

// Sphere Size
const int Y_SEGMENTS = 20;
const int X_SEGMENTS = 40;
const int tot_vertices_number = (Y_SEGMENTS + 1) * (X_SEGMENTS + 1);
const int tot_triangles_number = (Y_SEGMENTS - 2) * X_SEGMENTS * 2 + 2 * X_SEGMENTS;

// Fluid Size
const int ROW = 7;
const int NUM = ROW * ROW * ROW;
const float radius = 0.025f;
const float distance = 0.015f;
const float halflen = 1.0f * (ROW - 1) * distance;
const float Bound = halflen + 4.0f * distance;
const float Yoffset = 0.4f;