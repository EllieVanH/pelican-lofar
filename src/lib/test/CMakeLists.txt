#
# pelican-lofar/lib/test/CMakeLists.txt
#

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CPPUNIT_INCLUDE_DIR})

# ==== Create test utility library.
set(lofarTestLib_src
    src/LofarUdpEmulator.cpp
    src/LofarEmulatorDataSim.cpp
    src/GPU_TestCard.cpp
    src/LofarPipelineTester.cpp
    src/LofarDataBlobGenerator.cpp
    src/PipelineWrapper.cpp
    src/DedispersionDataGenerator.cpp
    src/TestDir.cpp
    src/JTestEmulator.cpp
    src/ABEmulator.cpp
)

# CUDA routines
set(cuda_src
   CudaVectorAdd.cu
   )
if( CUDA_FOUND )
    CUDA_COMPILE( cuda_obj ${cuda_src} )
    list(APPEND lofarTestLib_src 
         src/TestCudaVectorAdd.cpp
        )
endif( CUDA_FOUND )

add_library(lofarTestLib STATIC ${lofarTestLib_src} ${cuda_obj} )
target_link_libraries(lofarTestLib
    pelican-lofar
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
)

set(TEST_EXE_LIBS
    pelican-lofar_static
    ${PELICAN_TESTUTILS_LIBRARY}
    ${PELICAN_LIBRARY}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${FFTW3_FFTW_LIBRARY}
    ${FFTW3_FFTWF_LIBRARY}
    ${CPPUNIT_LIBRARIES}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTXML_LIBRARY})


# ==== Create test binary and add it to the cmake test framework.
set(lofarTest_src
    src/CppUnitMain.cpp
    src/GPU_ManagerTest.cpp
    src/GPU_MemoryMapTest.cpp
    src/AdapterTimeSeriesDataSetTest.cpp
    src/BandPassTest.cpp
    src/BinMapTest.cpp
    src/DataStreamingTest.cpp
    src/DedispersionDataAnalysisOutputTest.cpp
    src/DedispersionSpectraTest.cpp
    #src/FilterBankAdapterTest.cpp
    #src/LofarChunkerTest.cpp
    src/LockingContainerTest.cpp
    src/LofarDataSplittingChunkerTest.cpp
    #src/PelicanBlobClientTest.cpp
    src/PPF_ChanneliserTest.cpp
    src/PPF_CoefficientsTest.cpp
    src/PumaOutputTest.cpp
    #src/RFI_ClipperTest.cpp
    # test - commented by Jayanth
    #src/SpectrumDataSetTest.cpp
)
if(CUDA_FOUND)
    list(APPEND lofarTest_src
            src/GPU_NVidiaTest.cpp
            src/GPU_ParamTest.cpp
            # test - commented by Jayanth
            #src/DedispersionBufferTest.cpp
            src/DedispersionModuleTest.cpp
        #    src/DedispersionAnalyserTest.cpp
        )
endif(CUDA_FOUND)
if(HDF5_FOUND)
if(LOFAR_DAL_FOUND)
    list(APPEND lofarTest_src
         src/H5_LofarBFDataWriterTest.cpp
         src/H5_LofarBFStokesWriterTest.cpp
         src/H5_LofarBFVoltageWriterTest.cpp
        )
endif(LOFAR_DAL_FOUND)
endif(HDF5_FOUND)
add_executable(lofarTest ${lofarTest_src})
set_target_properties(lofarTest PROPERTIES 
    COMPILE_FLAGS "${OpenMP_CXX_FLAGS}"
    LINK_FLAGS "${OpenMP_CXX_FLAGS}")
target_link_libraries(lofarTest
    lofarTestLib
    ${TEST_EXE_LIBS})
add_test(lofarTest lofarTest)


# ==== Create the lofar udp emulator binary.
add_executable(lofarEmulator src/lofarEmulatorMain.cpp)
target_link_libraries(lofarEmulator
    ${TEST_EXE_LIBS}
    lofarTestLib
)

# ==== Create the lofar binary which sends simulated data.
add_executable(lofarEmulatorDataSim src/lofarEmulatorDataSimMain.cpp)
target_link_libraries(lofarEmulatorDataSim
    ${TEST_EXE_LIBS}
    lofarTestLib
)
install(TARGETS "lofarEmulatorDataSim" DESTINATION ${BINARY_INSTALL_DIR})

# ==== Create the JTest binary which sends simulated data.
add_executable(JTestEmulator src/JTestEmulatorMain.cpp)
target_link_libraries(JTestEmulator
    ${TEST_EXE_LIBS}
    lofarTestLib
)
install(TARGETS "JTestEmulator" DESTINATION ${BINARY_INSTALL_DIR})

# ==== Create the ALFABURST binary which sends simulated data.
add_executable(ABEmulator src/ABEmulatorMain.cpp)
target_link_libraries(ABEmulator
    ${TEST_EXE_LIBS}
    lofarTestLib
)
install(TARGETS "ABEmulator" DESTINATION ${BINARY_INSTALL_DIR})

# ==== Create a non cppunit PPF test binary
add_executable(ppf_performance_test src/PPF_PerformanceTest.cpp)
set_target_properties(ppf_performance_test PROPERTIES
    COMPILE_FLAGS "${OpenMP_CXX_FLAGS}"
    LINK_FLAGS "${OpenMP_CXX_FLAGS}"
)

target_link_libraries(ppf_performance_test
    ${TEST_EXE_LIBS}
)

# ==== Test the channel profile of the PPF channeliser.
#add_executable(ppf_profile_test src/PPF_ProfileTest.cpp)
#set_target_properties(ppf_profile_test PROPERTIES
#    COMPILE_FLAGS "${OpenMP_CXX_FLAGS}"
#    LINK_FLAGS "${OpenMP_CXX_FLAGS}"
#)
#target_link_libraries(ppf_profile_test
#   ${TEST_EXE_LIBS}
#)

# ==== Copy files required for testing to the build directory.
include(${CMAKE_SOURCE_DIR}/cmake/CopyFiles.cmake)
copy_files(${CMAKE_CURRENT_SOURCE_DIR}/data/*.* . testLibFiles)
