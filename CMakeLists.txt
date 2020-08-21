cmake_minimum_required(VERSION 2.8)
project(epolltest)
SET(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 14)

add_executable(epolltest main.cpp myepoll.cpp myepoll.h)