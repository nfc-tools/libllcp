# $Id$
set(EXAMPLES-SOURCES
        snep-server
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libllcp)


# Examples
foreach(source ${EXAMPLES-SOURCES})
  add_executable(${source} ${source}.c)
  target_link_libraries(${source} llcp ${LIBS})
  install(TARGETS ${source} RUNTIME DESTINATION bin COMPONENT examples)
endforeach(source)
