#!/bin/bash
echo "=== Building Math Clash Game with MSYS2 SFML ==="
echo "Compiler: $(g++ --version | head -1)"
echo "Building..."

# Use MSYS2 SFML paths (automatically in PATH)
g++ -std=c++17 -O2 -I/ucrt64/include \
    src/main.cpp \
    -o MathClashGame.exe \
    -L/ucrt64/lib \
    -lsfml-graphics -lsfml-window -lsfml-system

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "📦 Copying required DLL files from MSYS2..."
    cp /ucrt64/bin/sfml-graphics-2.dll .
    cp /ucrt64/bin/sfml-window-2.dll .
    cp /ucrt64/bin/sfml-system-2.dll .
    echo "🎮 Ready to run: ./MathClashGame.exe"
else
    echo "❌ Build failed!"
fi