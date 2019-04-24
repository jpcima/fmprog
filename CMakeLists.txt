cmake_minimum_required(VERSION "3.3")

option(BUILD_TESTS "Build tests" OFF)

if(FALSE)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5Multimedia CONFIG REQUIRED)

add_executable(FMProg WIN32
  "sources/fmprog.cc"
  "sources/mainwindow.cc"
  "sources/mainwindow.ui"
  "sources/operatoreditor.cc"
  "sources/operatoreditor.ui"
  "sources/instrumenteditor.cc"
  "sources/instrumenteditor.ui"
  "sources/instrument/bank.cpp"
  "sources/synth/tinysynth.cpp"
  "sources/ai/evaluation.cc"
  "sources/ai/algorithm.cc"
  "sources/ai/ai.cc"
  "sources/ai/qtmeta.cc"
  "sources/utility/music.cc")
target_include_directories(FMProg PRIVATE "sources")
target_link_libraries(FMProg PRIVATE Qt5::Widgets Qt5::Multimedia)

macro(find_library_required VAR NAME)
  find_library(${VAR} "${NAME}")
  if(NOT ${VAR})
    message(FATAL_ERROR "Cannot find library `${NAME}`.")
  endif()
endmacro()

find_library_required(AUBIO_LIBRARY "aubio")
target_link_libraries(FMProg PRIVATE "${AUBIO_LIBRARY}")

find_package(Threads REQUIRED)
target_link_libraries(FMProg PRIVATE ${CMAKE_THREAD_LIBS_INIT})

add_library(FMProg-chips STATIC
  "sources/chips/gens/Ym2612_Emu.cpp"
  "sources/chips/mamefm/fm.cpp"
  "sources/chips/mamefm/resampler.cpp"
  "sources/chips/mamefm/ymdeltat.cpp"
  "sources/chips/np2/fmgen_file.cpp"
  "sources/chips/np2/fmgen_fmgen.cpp"
  "sources/chips/np2/fmgen_fmtimer.cpp"
  "sources/chips/np2/fmgen_opna.cpp"
  "sources/chips/np2/fmgen_psg.cpp"
  "sources/chips/gens_opn2.cpp"
  "sources/chips/gx_opn2.cpp"
  "sources/chips/mame_opn2.cpp"
  "sources/chips/mame_opna.cpp"
  "sources/chips/np2_opna.cpp"
  "sources/chips/nuked_opn2.cpp"
  "sources/chips/gx/gx_ym2612.c"
  "sources/chips/mame/mame_ym2612fm.c"
  "sources/chips/mamefm/emu2149.c"
  "sources/chips/nuked/ym3438.c"
  "sources/chips/pmdwin_opna.cpp"
  "sources/chips/pmdwin/opna.c"
  "sources/chips/pmdwin/psg.c"
  "sources/chips/pmdwin/rhythmdata.c")
target_include_directories(FMProg-chips PUBLIC "sources")
target_link_libraries(FMProg PRIVATE FMProg-chips)

add_library(FMProg-formats STATIC
  "sources/file-formats/common.cpp"
  "sources/file-formats/ffmt_base.cpp"
  "sources/file-formats/ffmt_enums.cpp"
  "sources/file-formats/format_wohlstand_opn2.cpp")
target_include_directories(FMProg-formats PUBLIC "sources")
target_link_libraries(FMProg-formats PUBLIC Qt5::Core)

target_link_libraries(FMProg PRIVATE FMProg-formats)

if(BUILD_TESTS)
  add_executable(Test-Eval
    "tests/eval.cc"
    "sources/instrument/bank.cpp"
    "sources/synth/tinysynth.cpp"
    "sources/ai/evaluation.cc"
    "sources/ai/ai.cc"
    "sources/utility/music.cc")
  target_link_libraries(Test-Eval PRIVATE FMProg-formats FMProg-chips "${AUBIO_LIBRARY}")
endif()

find_package(OpenMP)
if(OpenMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()
