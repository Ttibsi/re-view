include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)

include(FetchContent)
fetchcontent_declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.2
)

fetchcontent_makeavailable(Catch2)
include(CTest)
include(Catch)
add_executable(test_exe
    # test files here
    test_input.cpp
    test_model.cpp
    test_utils.cpp
)

target_link_libraries(test_exe PRIVATE lib Catch2::Catch2WithMain)
catch_discover_tests(test_exe)
