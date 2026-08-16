cmake_minimum_required(VERSION 3.16)
project(rimlang C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2 -D_GNU_SOURCE")

include_directories(include)

set(SOURCES
    src/parser/lexer.c
    src/parser/parser.c
    src/parser/korean_nlp.c
    src/parser/korean_synonym.c
    src/runtime/runtime.c
    src/runtime/joke_table.c
    src/engine/engine.c
)

add_executable(rimlang src/main.c ${SOURCES})
add_executable(test_runner tests/test_runner.c ${SOURCES})
