# Package sources
set(SOURCES
  phCook.cc
  ph.cc
  phAdjacent.cc
  phOutput.cc
  phLinks.cc
  phGeomBC.cc
  phBlock.cc
  phAdapt.cc
  phRestart.cc
  phInput.cc
  phPartition.cc
  phIO.c
  phBC.cc
  phBubble.cc
  phConstraint.cc
  phModelGeometry.cc
  phFilterMatching.cc
  phAxisymmetry.cc
  phInterfaceCutter.cc
  phstream.cc
  phiotimer.cc
)

if(PUMI_FORTRAN_INTERFACE)
  enable_language(Fortran)
  set(CMAKE_Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/mod_files)
  set(SOURCES ${SOURCES} phiotimer.f phastaChef.f)
  install(DIRECTORY ${CMAKE_Fortran_MODULE_DIRECTORY}/ DESTINATION include)
endif()

if(ENABLE_SIMMETRIX)
  set(SOURCES ${SOURCES}
      phAttrib.cc
      phSnap.cc
      phMeshQuality.cc
     )
endif()

if(SIMMODSUITE_SimAdvMeshing_FOUND) # simmetrix enabled
  set(SOURCES ${SOURCES} phGrowthCurves.cc)
endif()
if(NOT ENABLE_SIMMETRIX # no simmetrix
   OR
   NOT SIMMODSUITE_SimAdvMeshing_FOUND) # have simmetrix, but not advMeshing
  set(SOURCES ${SOURCES} phGrowthCurves_empty.cc)
endif()


#determine which timer to use based on the target system and compiler
if(CMAKE_SYSTEM_NAME MATCHES BlueGeneQ*)
  add_definitions(-DUSE_PCU_TIME)
elseif(CMAKE_CXX_COMPILER_ID MATCHES Intel)
  add_definitions(-DHAVE_INTEL_RDTSC)
else()
  #from https://github.com/abduld/libwb/issues/5
  include(CheckFunctionExists)
  CHECK_FUNCTION_EXISTS(clock_gettime HAVE_CLOCK_GETTIME)
  if(NOT HAVE_CLOCK_GETTIME)
    find_library(LIBRT_LIBRARIES rt)
    message(STATUS "librt: ${LIBRT_LIBRARIES}")
  endif(NOT HAVE_CLOCK_GETTIME)
  if( HAVE_CLOCK_GETTIME OR LIBRT_LIBRARIES )
    add_definitions(-DHAVE_CLOCK_GETTIME)
  else()
    add_definitions(-DUSE_PCU_TIME)
  endif()
endif()

# Package headers
set(HEADERS
  chef.h
  phInput.h
  phstream.h
  phiotimer.h
  phastaChef.h
)

# Add the ph library
add_library(ph ${SOURCES})

# Include directories
target_include_directories(ph INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
    )
target_include_directories(ph PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )
target_link_libraries(ph
    PUBLIC
      ma
      mds
      sam
      parma
      pcu
      apf_zoltan
    )
if(ENABLE_SIMMETRIX)
  target_link_libraries(ph PUBLIC
      gmi_sim
      apf_sim
    )
endif()
if(LIBRT_LIBRARIES)
  target_link_libraries(ph PUBLIC ${LIBRT_LIBRARIES})
endif()

scorec_export_library(ph)

bob_end_subdir()
