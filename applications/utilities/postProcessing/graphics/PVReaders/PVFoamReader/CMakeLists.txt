# create a plugin that adds a reader to the ParaView GUI
# it is added in the file dialog when doing opens/saves.

# The qrc file is processed by Qt's resource compiler (rcc)
# the qrc file must have a resource prefix of "/ParaViewResources"
# and ParaView will read anything contained under that prefix
# the pqReader.xml file contains xml defining readers with their
# file extensions and descriptions.

cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

FIND_PACKAGE(ParaView REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})

LINK_DIRECTORIES(
    $ENV{FOAM_LIBBIN}
)

INCLUDE_DIRECTORIES(
    $ENV{WM_PROJECT_DIR}/src/foam/lnInclude
    $ENV{WM_PROJECT_DIR}/src/OSspecific/$ENV{WM_OSTYPE}/lnInclude
    $ENV{WM_PROJECT_DIR}/src/finiteVolume/lnInclude
    ${PROJECT_SOURCE_DIR}/../vtkPVFoam
)

ADD_DEFINITIONS(
    -std=c++11
    -DWM_$ENV{WM_PRECISION_OPTION}
    -DWM_LABEL_SIZE=$ENV{WM_LABEL_SIZE}
)

# Set output library destination to plugin folder
SET(
    LIBRARY_OUTPUT_PATH $ENV{PV_PLUGIN_PATH}
    CACHE INTERNAL
    "Single output directory for building all libraries."
)

# Build the server-side plugin
ADD_PARAVIEW_PLUGIN(
    PVFoamReader_SM "1.0"
    SERVER_MANAGER_XML PVFoamReader_SM.xml
    SERVER_MANAGER_SOURCES vtkPVFoamReader.cxx
)

TARGET_LINK_LIBRARIES(
    PVFoamReader_SM
    foam
    finiteVolume
    vtkPVFoam
    pqCore
)
#-----------------------------------------------------------------------------
