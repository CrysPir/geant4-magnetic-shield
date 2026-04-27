cmake_minimum_required(VERSION 3.16)
project(MyShield LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Geant4 REQUIRED ui_all vis_all)

add_executable(MyShield
  src/MyShield.cc
  src/DetectorConstruction.cc
  src/MagneticField.cc
  src/GeometryHelpers.cc
  src/CosmicRaySampler.cc
  src/PrimaryGeneratorAction.cc
  src/ActionInitialization.cc
  src/RunAction.cc
  src/EventAction.cc
  src/SteppingAction.cc
  src/TrackingAction.cc
)

target_include_directories(MyShield PRIVATE
  include
  ${Geant4_INCLUDE_DIRS}
)

target_link_libraries(MyShield PRIVATE ${Geant4_LIBRARIES})

if(MSVC)
  target_compile_options(MyShield PRIVATE /utf-8)
endif()

# Keep macro files next to the executable in the build tree.
file(COPY macros/ DESTINATION ${CMAKE_BINARY_DIR}/macros)
file(COPY simulations/ DESTINATION ${CMAKE_BINARY_DIR}/simulations)
