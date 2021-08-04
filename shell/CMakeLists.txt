cmake_minimum_required(VERSION 3.19)
project(shell C)




set(CMAKE_C_STANDARD 99)


file(GLOB shell
        "*.h"
        "*.c"
        )

add_executable(shell  ${shell} )