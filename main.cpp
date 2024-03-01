#include "raylib.h"
#include <array>
#include <cmath>
#include <iostream>
#include <ostream>
#include <random>
#include <string>
#include <vector>

const int WIDTH = 1600;
const int HEIGHT = 900;

const int NUM_WAVES = 3;
const int NUM_PEAKS = NUM_WAVES / 2;
const float PEAK_TO_PEAK = 0.15 * HEIGHT;
const float OFFSET = 0.05;
const float WAVE_SMOOTHNESS = 4.0;

Color operator-(Color first, Color second) {
  return Color{
      (unsigned char)(first.r - second.r),
      (unsigned char)(first.g - second.g),
      (unsigned char)(first.b - second.b),
      (unsigned char)(first.a - second.a),
  };
}

Color operator*(Color color, float multiplier) {
  return Color{
      (unsigned char)(color.r * multiplier),
      (unsigned char)(color.g * multiplier),
      (unsigned char)(color.b * multiplier),
      (unsigned char)(color.a * multiplier),
  };
}

Color operator+(Color first, Color second) {
  return Color{
      (unsigned char)(first.r + second.r),
      (unsigned char)(first.g + second.g),
      (unsigned char)(first.b + second.b),
      (unsigned char)(first.a + second.a),
  };
}

std::ostream &operator<<(std::ostream &os, const Color color) {
  return os << "( " << (int)color.r << ", " << (int)color.g << ", "
            << (int)color.b << ", " << (int)color.a << " )";
}

static float getRandomNumber(float min, float max) {
  static std::random_device rd;
  static std::mt19937 e2(rd());
  std::uniform_real_distribution<> dist(min, max);

  return dist(e2);
}

float getWavelength() { return getRandomNumber(WIDTH / 2.0, WIDTH); }

float getAmplitude() { return getRandomNumber(PEAK_TO_PEAK / 2, PEAK_TO_PEAK); }

std::array<Vector2, NUM_WAVES> getStartingPoints() {
  std::array<Vector2, NUM_WAVES> startingPoints;

  for (int i = 0; i < NUM_WAVES; ++i) {
    float offset = getRandomNumber(-OFFSET, OFFSET);
    float pointY = ((i + 1.0) / (NUM_WAVES + 1) + offset) * HEIGHT;

    startingPoints[i] = Vector2{0, pointY};
  }

  return startingPoints;
}

float sinsoidPoint(float x, float wavelength, float amplitude,
                   float phase = 0) {
  float theta = 2 * PI * x / wavelength + phase;
  float y = amplitude * std::sin(theta);
  return y;
}

std::array<std::vector<Vector2>, NUM_WAVES> generateWaves() {
  std::array<std::vector<Vector2>, NUM_WAVES> waves;

  std::array<Vector2, NUM_WAVES> startingPoints = getStartingPoints();
  for (int i = 0; i < NUM_WAVES; ++i) {
    float wavelength = getWavelength();
    float amplitude = getAmplitude() / 2;

    std::vector<Vector2> wave(WAVE_SMOOTHNESS * WIDTH);
    for (int j = 0; j < WAVE_SMOOTHNESS * WIDTH; ++j) {
      float x = j / WAVE_SMOOTHNESS;
      float phase = getRandomNumber(0, 0);
      float y =
          startingPoints[i].y + sinsoidPoint(x, wavelength, amplitude, phase);
      wave[j] = Vector2{x, y};
    }

    waves[i] = wave;
  }

  return waves;
}

std::array<Color, NUM_WAVES + 1> generateGradient(Color from, Color to) {
  Color difference = from - to;

  std::array<Color, NUM_WAVES + 1> gradient;
  for (int i = 0; i < NUM_WAVES + 1; ++i) {
    float multiplier = i * 1.0 / (NUM_WAVES + 1);
    gradient[i] = from - difference * multiplier;
  }

  return gradient;
}

Color calculatePixelColor(Vector2 position,
                          std::array<std::vector<Vector2>, NUM_WAVES> &waves,
                          std::array<Color, NUM_WAVES + 1> &gradient) {
  float x = position.x;
  float y = position.y;
  for (int i = 0; i < NUM_WAVES; ++i) {
    int j = x * WAVE_SMOOTHNESS;
    float wavePointY = waves[i][j].y;
    if (y <= wavePointY) {
      return gradient[i];
    }
  }

  return gradient[NUM_WAVES];
}

int main(void) {
  InitWindow(WIDTH, HEIGHT, "Chromatica");

  auto waves = generateWaves();
  auto gradient = generateGradient(DARKPURPLE, BLACK);
  for (auto color : gradient) {
    std::cout << color << '\n';
  }
  int savedCounter = 0;
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(DARKGRAY);

    for (auto wave : waves) {
      DrawSplineLinear(&wave[0], wave.size(), 1, RAYWHITE);
    }

    for (int y = 0; y < HEIGHT; ++y) {
      for (int x = 0; x < WIDTH; ++x) {
        Vector2 position = Vector2{(float)x, (float)y};
        Color color = calculatePixelColor(position, waves, gradient);
        DrawPixelV(position, color);
      }
    }

    DrawFPS(0, 0);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      waves = generateWaves();
      gradient = generateGradient(DARKPURPLE, BLACK);
    }

    if (IsKeyPressed(KEY_SPACE)) {
      Image image = LoadImageFromScreen();
      std::string filename = "exports/image-" + std::to_string(savedCounter++) + ".png";
      ExportImage(image, filename.c_str());
    }

    EndDrawing();
  }

  return 0;
}
