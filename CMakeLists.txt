cmake_minimum_required(VERSION 3.20)

project(alt_shortest_path LANGUAGES C)

# Require standard C17 without compiler-specific language extensions.
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Generate compilation information for clangd and other development tools.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable CTest integration for automated tests.
include(CTest)

# Build the ALT library.
add_library(alt_shortest_path STATIC
    src/dijkstra.c
    src/graph.c
    src/landmark_selection.c
    src/landmark_preprocessing.c
    src/priority_queue.c
)

# Make the public headers available to the library and its consumers.
target_include_directories(alt_shortest_path
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Build the tests.
if (BUILD_TESTING)
  # Graph tests
  add_executable(graph_tests
      tests/graph_tests.c
  )

  target_compile_options(graph_tests PRIVATE 
      -UNDEBUG
  )

  target_link_libraries(graph_tests
      PRIVATE
          alt_shortest_path
  )

  add_test(NAME
      graph_tests
          COMMAND
              graph_tests
  )

  # Priority Queue tests
  add_executable(priority_queue_tests
      tests/priority_queue_tests.c
  )

  target_compile_options(priority_queue_tests PRIVATE
      -UNDEBUG
  )

  target_link_libraries(priority_queue_tests
      PRIVATE
          alt_shortest_path
  )

  add_test(NAME
      priority_queue_tests
          COMMAND
              priority_queue_tests
  )

  # Dijkstra tests
  add_executable(dijkstra_tests
      tests/dijkstra_tests.c
  )

  target_compile_options(dijkstra_tests PRIVATE
      -UNDEBUG
  )

  target_link_libraries(dijkstra_tests
      PRIVATE
          alt_shortest_path
  )

  add_test(NAME
      dijkstra_tests
          COMMAND
              dijkstra_tests
  )

  # Landmark selection tests
  add_executable(landmark_selection_tests
      tests/landmark_selection_tests.c
  )

  target_compile_options(landmark_selection_tests PRIVATE
      -UNDEBUG
  )

  target_link_libraries(landmark_selection_tests
      PRIVATE
          alt_shortest_path
  )

  add_test(NAME
      landmark_selection_tests
          COMMAND
              landmark_selection_tests
  )

  # Landmark preprocessing tests
  add_executable(landmark_preprocessing_tests
      tests/landmark_preprocessing_tests.c
  )

  target_compile_options(landmark_preprocessing_tests PRIVATE
      -UNDEBUG
  )

  target_link_libraries(landmark_preprocessing_tests
      PRIVATE
          alt_shortest_path
  )

  add_test(NAME
      landmark_preprocessing_tests
          COMMAND
              landmark_preprocessing_tests
  )

endif()
