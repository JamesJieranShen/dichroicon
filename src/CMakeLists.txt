add_library( dichroiconobj OBJECT
  # Append each .cc file
  Dichroicon.cc
)

target_include_directories( dichroiconobj PUBLIC
  $<BUILD_INTERFACE:${DICHROICON_INCLUDE_DIR}>
  $<INSTALL_INTERFACE:include>)

file(COPY include/ DESTINATION ${DICHROICON_INCLUDE_DIR})

add_executable(dichroicon 
  dichroicon.cpp
  GeoHexDichroiconFactory.cc
)
target_link_libraries(dichroicon PUBLIC 
  dichroiconobj ${Ratpac_LIBRARIES} ${ROOT_LIBRARIES} ${Geant4_LIBRARIES})

set_target_properties(dichroiconobj dichroicon
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${DICHROICON_LIBRARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${DICHROICON_LIBRARY_DIR})

install(TARGETS dichroiconobj dichroicon
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)

install(DIRECTORY ${DICHROICON_INCLUDE_DIR}
    DESTINATION ${CMAKE_INSTALL_PREFIX})
