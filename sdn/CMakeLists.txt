set(sdn_sources 
        model/sdn-controller.cc 
        model/sdn-flow-table.cc
        model/sdn-switch.cc)
set(sdn_headers 
        model/sdn-controller.h 
        model/sdn-flow-table.h
        model/sdn-switch.h)

build_lib(
    LIBNAME sdn
    SOURCE_FILES
        ${sdn_sources}
    HEADER_FILES
        ${sdn_headers}
    LIBRARIES_TO_LINK
        ${libcore}
        ${libnetwork}
        ${libinternet}
        ${libopenflow}
)
