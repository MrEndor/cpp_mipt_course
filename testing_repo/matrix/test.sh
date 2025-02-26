#! /usr/bin/env bash

echo "begin testing $1"

echo "Check for banned words"
python3 banned_words_checker.py --solution=$1.hpp --banned-words=banned_words.json
if [[ ! $? -eq 0 ]]
then
  echo "В вашем коде есть забаненые слова!"
  exit 1
fi

echo "clang-tidy check"
clang-tidy -extra-arg=-std=c++23 -quiet $1.hpp
if [[ ! $? -eq 0 ]]
then
  echo "Проверка clang-tidy не пройдена"
  exit 1
fi


echo "clang-format check"
clang-format -i -style='{BasedOnStyle: Google, DerivePointerAlignment: false, PointerAlignment: Left, AlignOperands: true}' $1.hpp &&
git diff --ignore-submodules --color $1.hpp > diff &&
cat diff
if [[ -s diff ]]
then
  echo "Проверка clang-format не пройдена"
  exit 1
fi
echo "clang-format achieved"


echo "Начинаем билдить"

echo "Сначала проверим тесты на CE"
clang++-18 -std=c++23 ce_test_mult.cpp 2> /dev/null
if [[ $? -eq 0 ]]
then
  echo "Компилируется перемножение неподходящих матриц"
  exit 1
fi

clang++-18 -std=c++23 ce_test_trace.cpp 2> /dev/null
if [[ $? -eq 0 ]]
then
  echo "Компилируется трейс от не квадратной матрицы"
  exit 1
fi
echo "Тесты на CE проверили"

echo "Build with clang"
mkdir build
cd build
cmake -DCMAKE_CXX_COMPILER=clang++-18 ..
if [[ ! $? -eq 0 ]]
then
  echo "Сборка cmake с clang не сработала"
  exit 1
fi
make
if [[ ! $? -eq 0 ]]
then
  echo "Make с clang не сработал"
  exit 1
fi
echo "Build with clang achieved"

echo "Running tests with clang build"
./$1
if [[ ! $? -eq 0 ]]
then
  echo "Тесты не пройдены"
  exit 1
fi
echo "Google tests achieved with clang achieved"

cd ..

echo "Build with g++"
rm -r build
mkdir build
cd build
cmake -DCMAKE_CXX_COMPILER=g++ ..
if [[ ! $? -eq 0 ]]
then
  echo "Сборка cmake с g++ не сработала"
  exit 1
fi
make
if [[ ! $? -eq 0 ]]
then
  echo "Make с g++ не сработал"
  exit 1
fi
echo "Build with g++ achieved"


echo "Running tests with g++ build"
./$1
if [[ ! $? -eq 0 ]]
then
  echo "Тесты не пройдены"
  exit 1
fi
echo "Google tests achieved with g++ achieved"


echo "Попробуем valgrind!"
valgrind --leak-check=yes --log-file=log.txt ./$1
echo "Valgrind log:"
cat log.txt
python3 ../valgrind_parser.py
if [[ ! $? -eq 0 ]]
then
  echo "А кто украл free?"
  exit 1
fi
echo "Valgrind achieved"


echo "Вы потрясающие!"
