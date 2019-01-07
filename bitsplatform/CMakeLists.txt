add_contract(bitsplatform bitsplatform ${CMAKE_CURRENT_SOURCE_DIR}/src/bitsplatform.cpp)
target_include_directories(bitsplatform.wasm
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(bitsplatform.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
