#############################################################################



SET(FEELPP_MODELS_OPTIONS_NUMBER_OF_PARAMETERS "10" CACHE STRING "number of parameters" )
SET(FEELPP_MODELS_OPTIONS_NUMBER_OF_GEOPARAMETERS "10" CACHE STRING "number of geometric parameters" )
SET(FEELPP_MODELS_OPTIONS_NUMBER_OF_GINACEXPR "10" CACHE STRING "number of ginac expr" )

CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/feelmodelscoreconstconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/feelmodelscoreconstconfig.h )

set(FEELPP_MODELS_LIBBASE_CHECK_DIR "${CMAKE_CURRENT_BINARY_DIR}/checkmodelsbuilt" CACHE STRING "FEELPP_MODELS_LIBBASE_CHECK_DIR")
file(REMOVE_RECURSE ${FEELPP_MODELS_LIBBASE_CHECK_DIR})


#############################################################################
#############################################################################

add_library(feelpp_modelcore
  SHARED
  options.cpp log.cpp timertool.cpp
  modelbase.cpp modelalgebraic.cpp modelnumerical.cpp
  markermanagement.cpp
  )
target_link_libraries(feelpp_modelcore ${FEELPP_LIBRARY} ${FEELPP_LIBRARIES})
INSTALL(TARGETS feelpp_modelcore DESTINATION lib/ COMPONENT LibsFeelppModelsCommon)

include(feelpp.models.macros.cmake)