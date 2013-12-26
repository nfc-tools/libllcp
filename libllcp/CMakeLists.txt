# $Id$

IF(WIN32)
  SET(WINDOWS_SOURCES ../contrib/win32/stdlib)
  SET(WINDOWS_SOURCES ../contrib/win32/libllcp/socketpair)
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../contrib/win32)
ENDIF(WIN32)

set(LIBRARY_SOURCES
		llc_connection
		llc_link
		llc_service
		llc_service_llc
		llc_service_sdp
		llcp
		llcp_log
		llcp_parameters
		llcp_pdu
		mac_iso18092
		${WINDOWS_SOURCES}
		)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(llcp SHARED ${LIBRARY_SOURCES})
target_link_libraries(llcp ${LIBS})

set(installheaders 
		llc_link.h 
		llc_connection.h 
		llc_service.h 
		llc_service_sdp.h 
		llc_service_llc.h 
		llcp.h 
		llcp_pdu.h 
		llcp_parameters.h
		mac.h
		)
install(FILES ${installheaders} DESTINATION ${INCLUDE_INSTALL_DIR}/nfc COMPONENT headers)

if(WIN32)
  # On Windows the shared (runtime) library should be either in the same
  # directory as the excutables or in the path, we add it to same directory
  install(TARGETS llcp RUNTIME DESTINATION bin COMPONENT libraries)

  # At compile time we need the .LIB file, we place it in the lib directory
  install(TARGETS llcp ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT headers)
else(WIN32)
  install(TARGETS llcp LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries)
endif(WIN32)
