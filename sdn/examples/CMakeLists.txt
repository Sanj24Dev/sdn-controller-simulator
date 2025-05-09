if(${ENABLE_EXAMPLES})
build_lib_example(
    NAME sdn-one
    SOURCE_FILES sdn-one-switch.cc
    LIBRARIES_TO_LINK
    ${libcore}
    ${libapplications}
    ${libinternet}
    ${libcsma}
    ${libopenflow}
    ${libnetwork}
    ${libsdn}
)
build_lib_example(
    NAME sdn-two
    SOURCE_FILES sdn-two-switch.cc
    LIBRARIES_TO_LINK
    ${libcore}
    ${libapplications}
    ${libinternet}
    ${libcsma}
    ${libopenflow}
    ${libnetwork}
    ${libsdn}
)
endif()