
include_directories(${PROJECT_INCLUDE_DIR})
enable_testing()

file(GLOB ARMO_HEADERS ${PROJECT_SOURCE_DIR}/algos/*.hpp, ${PROJECT_SOURCE_DIR}/algos/*.h)
file(GLOB ARMO_SOURCES ${PROJECT_SOURCE_DIR}/algos/*.cpp)
add_executable(armo ${ARMO_SOURCES} ${ARMO_HEADERS})
target_link_libraries(armo ${LIBS})
if(Matplot)
 target_include_directories(armo PRIVATE ${Python3_INCLUDE_DIRS} ${Python3_NumPy_INCLUDE_DIRS})
 target_link_libraries(armo Python3::Python Python3::NumPy ${PCL_LIBRARIES})
endif()
ADD_DEPENDENCIES(armo gravity LASTOOLS)
add_test(NAME armo_test COMMAND armo)
if(Cov)
  set_target_properties(armo
  PROPERTIES
    COMPILE_FLAGS "-ftest-coverage -fprofile-arcs"
    LINK_FLAGS    "-lgcov --coverage -fprofile-arcs"
  )
endif()
