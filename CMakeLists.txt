list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

cmake_minimum_required(VERSION 3.4.1)

if (WIN32)
  add_definitions( "-D_GLIBCXX_USE_NANOSLEEP=1" ) # "'sleep_for' is not a member of 'std::this_thread'" in gcc 4.7/4.8
  add_definitions( "-DWIN32_LEAN_AND_MEAN" )
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj -O2 -fPIC -std=c++14 -F/Library/Frameworks -pthread -lcrypto -lcrypt32") 
else()
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++14 -F/Library/Frameworks -pthread")
endif()

#SET(CMAKE_C_COMPILER /path/to/c/compiler)
#SET(CMAKE_CXX_COMPILER /path/to/cpp/compiler)
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++14 -F/Library/Frameworks -pthread -fsanitize=address")    # TODO: way to enable sanitize?  get runtime error with this on

project(WowneroCppLibrary)

set(BUILD_LIBRARY ON)
set(BUILD_SAMPLE OFF)
set(BUILD_SCRATCHPAD OFF)
set(BUILD_TESTS OFF)

###################
# wownero
###################

set(WOWNERO_PROJECT "${CMAKE_SOURCE_DIR}/external/wownero" CACHE STRING "Wownero project source directory")
set(WOWNERO_PROJECT_SRC "${WOWNERO_PROJECT}/src")

# header includes
include_directories(src/)
include_directories("${WOWNERO_PROJECT}/contrib/epee/include")
include_directories("${WOWNERO_PROJECT}/external/")
include_directories("${WOWNERO_PROJECT}/external/easylogging++")
include_directories("${WOWNERO_PROJECT}/external/rapidjson/include")
include_directories("${WOWNERO_PROJECT_SRC}/")
include_directories("${WOWNERO_PROJECT_SRC}/wallet")
include_directories("${WOWNERO_PROJECT_SRC}/wallet/api")
include_directories("${WOWNERO_PROJECT_SRC}/hardforks")
include_directories("${WOWNERO_PROJECT_SRC}/crypto/crypto_ops_builder/include/")

set(EXTERNAL_LIBS_DIR ${CMAKE_SOURCE_DIR}/external-libs)
message(STATUS EXTERNAL_LIBS_DIR : ${EXTERNAL_LIBS_DIR})

####################
# Extra libraries
####################

if (APPLE)
  if (DEPENDS)
    list(APPEND EXTRA_LIBRARIES "-framework Foundation -framework IOKit -framework AppKit")
  else()
    find_library(COREFOUNDATION CoreFoundation)
    find_library(IOKIT IOKit)
    find_library(APPKIT AppKit)
    list(APPEND EXTRA_LIBRARIES ${IOKIT})
    list(APPEND EXTRA_LIBRARIES ${COREFOUNDATION})
    list(APPEND EXTRA_LIBRARIES ${APPKIT})
  endif()
endif()
if (WIN32)
  list(APPEND EXTRA_LIBRARIES setupapi)
endif()

message(STATUS EXTRA_LIBRARIES: ${EXTRA_LIBRARIES})

############
# Protobuf
############

if (NOT APPLE)
  include(FindProtobuf)
  find_package(Protobuf)
  message(STATUS "Protobuf lib: ${Protobuf_LIBRARY}, inc: ${Protobuf_INCLUDE_DIR}, protoc: ${Protobuf_PROTOC_EXECUTABLE}")
  include_directories(${Protobuf_INCLUDE_DIR})
endif()

############
# LibUSB
############

find_library(usb_LIBRARY NAMES usb-1.0 libusb usb)
set(LibUSB_LIBRARIES ${usb_LIBRARY})

############
# Boost
############

set(Boost_NO_BOOST_CMAKE 1)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.58 QUIET REQUIRED COMPONENTS chrono date_time filesystem program_options regex serialization wserialization system thread)
message(STATUS "Using Boost include dir at ${Boost_INCLUDE_DIR}")
include_directories(${Boost_INCLUDE_DIR})

############
# OpenSSL
############

if (APPLE AND NOT IOS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=default -std=c++14")
  if (NOT OPENSSL_ROOT_DIR)
      EXECUTE_PROCESS(COMMAND brew --prefix openssl
        OUTPUT_VARIABLE OPENSSL_ROOT_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    message(STATUS "Using OpenSSL found at ${OPENSSL_ROOT_DIR}")
  endif()
endif()

find_package(OpenSSL REQUIRED)
message(STATUS "Using OpenSSL include dir at ${OPENSSL_INCLUDE_DIR}")
include_directories(${OPENSSL_INCLUDE_DIR})

if(STATIC AND NOT IOS)
  if(UNIX)
    set(OPENSSL_LIBRARIES "${OPENSSL_LIBRARIES};${CMAKE_DL_LIBS};${CMAKE_THREAD_LIBS_INIT}")
  endif()
endif()

if (WIN32)
  list(APPEND OPENSSL_LIBRARIES ws2_32 crypt32)
endif()

############
# libsodium
############

include_directories(external/libsodium/include/sodium)
find_library(SODIUM_LIBRARY sodium REQUIRED)
message(STATUS "Using libsodium library at ${SODIUM_LIBRARY}")

############
# HIDAPI
############

if(APPLE)
  include_directories(SYSTEM /usr/include/malloc)
  if(POLICY CMP0042)
    cmake_policy(SET CMP0042 NEW)
  endif()
endif()

find_package(HIDAPI REQUIRED)
message(STATUS "Using HIDAPI include dir at ${HIDAPI_INCLUDE_DIR}")
add_definitions(-DHAVE_HIDAPI)
include_directories(${HIDAPI_INCLUDE_DIR})

#############
# Wownero
#############

set(WOWNERO_PROJECT_BUILD "${WOWNERO_PROJECT}/build/release" CACHE STRING "Wownero project build directory")
message(STATUS "Using wownero build: " ${WOWNERO_PROJECT_BUILD})

list(APPEND CMAKE_MODULE_PATH "${WOWNERO_PROJECT}/cmake")

add_library(wallet STATIC IMPORTED)
set_target_properties(wallet PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/lib/libwallet.a)

# libwallet-crypto.a provides x86_64 asm for some wallet functions
if (EXISTS ${WOWNERO_PROJECT_BUILD}/src/crypto/wallet/libwallet-crypto.a)
  add_library(wallet_crypto_lib STATIC IMPORTED)
  set_target_properties(wallet_crypto_lib PROPERTIES IMPORTED_LOCATION
      ${WOWNERO_PROJECT_BUILD}/src/crypto/wallet/libwallet-crypto.a)
  set(wallet_crypto wallet_crypto_lib)
endif()

add_library(lmdb STATIC IMPORTED)
set_target_properties(lmdb PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/external/db_drivers/liblmdb/liblmdb.a)

add_library(epee STATIC IMPORTED)
set_target_properties(epee PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/contrib/epee/src/libepee.a)

############
# Unbound
############

find_package(Unbound)
if(NOT UNBOUND_INCLUDE_DIR)
  message(FATAL_ERROR "Could not find libunbound")
else()
  message(STATUS "Found libunbound include (unbound.h) in ${UNBOUND_INCLUDE_DIR}")
  if(UNBOUND_LIBRARIES)
    message(STATUS "Found libunbound library")
    include_directories(${UNBOUND_INCLUDE_DIR})
    if (WIN32)
      add_library(unbound STATIC IMPORTED)
    else()
      add_library(unbound SHARED IMPORTED)
    endif()
    set_target_properties(unbound PROPERTIES IMPORTED_LOCATION ${UNBOUND_LIBRARIES}) 
  else()
    message(FATAL_ERROR "Found libunbound includes, but could not find libunbound library. Please make sure you have installed libunbound or libunbound-dev or the equivalent")
  endif()
endif()

add_library(rpc_base STATIC IMPORTED)
set_target_properties(rpc_base PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/src/rpc/librpc_base.a)

add_library(net STATIC IMPORTED)
set_target_properties(net PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/src/net/libnet.a)
    
add_library(hardforks STATIC IMPORTED)
set_target_properties(hardforks PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/src/hardforks/libhardforks.a)
    
add_library(easylogging STATIC IMPORTED)
set_target_properties(easylogging PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/external/easylogging++/libeasylogging.a)

add_library(cryptonote_core STATIC IMPORTED)
set_target_properties(cryptonote_core PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/src/cryptonote_core/libcryptonote_core.a)

add_library(cryptonote_protocol STATIC IMPORTED)
set_target_properties(cryptonote_protocol PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/src/cryptonote_protocol/libcryptonote_protocol.a)

add_library(cryptonote_basic STATIC IMPORTED)
set_target_properties(cryptonote_basic PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/src/cryptonote_basic/libcryptonote_basic.a)
    
add_library(cryptonote_format_utils_basic STATIC IMPORTED)
set_target_properties(cryptonote_format_utils_basic PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/src/cryptonote_basic/libcryptonote_format_utils_basic.a)

add_library(mnemonics STATIC IMPORTED)
set_target_properties(mnemonics PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/src/mnemonics/libmnemonics.a)

add_library(common STATIC IMPORTED)
set_target_properties(common PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/src/common/libcommon.a)

add_library(cncrypto STATIC IMPORTED)
set_target_properties(cncrypto PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/src/crypto/libcncrypto.a)

add_library(ringct STATIC IMPORTED)
set_target_properties(ringct PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/src/ringct/libringct.a)

add_library(ringct_basic STATIC IMPORTED)
set_target_properties(ringct_basic PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/src/ringct/libringct_basic.a)

add_library(blockchain_db STATIC IMPORTED)
set_target_properties(blockchain_db PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/src/blockchain_db/libblockchain_db.a)

add_library(blocks STATIC IMPORTED)
set_target_properties(blocks PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/src/blocks/libblocks.a)

add_library(checkpoints STATIC IMPORTED)
set_target_properties(checkpoints PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/src/checkpoints/libcheckpoints.a)

add_library(device STATIC IMPORTED)
set_target_properties(device PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/src/device/libdevice.a)
    
add_library(device_trezor STATIC IMPORTED)
set_target_properties(device_trezor PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/src/device_trezor/libdevice_trezor.a)

add_library(multisig STATIC IMPORTED)
set_target_properties(multisig PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/src/multisig/libmultisig.a)

add_library(version STATIC IMPORTED)
set_target_properties(version PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/src/libversion.a)
    
add_library(randomx STATIC IMPORTED)
set_target_properties(randomx PROPERTIES IMPORTED_LOCATION
    ${WOWNERO_PROJECT_BUILD}/external/randomx/librandomx.a)
    
########################
# Build c++ library
########################

set(
    LIBRARY_SRC_FILES
    src/utils/gen_utils.cpp
    src/utils/wownero_utils.cpp
    src/daemon/wownero_daemon_model.cpp
    src/daemon/wownero_daemon.cpp
    src/wallet/wownero_wallet_model.cpp
    src/wallet/wownero_wallet_keys.cpp
    src/wallet/wownero_wallet_full.cpp
)

if (BUILD_LIBRARY)
  add_library(wownero-cpp SHARED ${LIBRARY_SRC_FILES})
  
  target_link_libraries(wownero-cpp
      wallet
      rpc_base
      net
      lmdb
      unbound
      easylogging
      cryptonote_core
      cryptonote_protocol
      cryptonote_basic
      cryptonote_format_utils_basic
      mnemonics
      ringct
      ringct_basic
      common
      blockchain_db
      blocks
      checkpoints
      device
      device_trezor
      cncrypto
      multisig
      version
      randomx
      epee
      hardforks
      ${wallet_crypto}
      
      ${UNBOUND_LIBRARIES}
      ${Boost_LIBRARIES}
      ${Protobuf_LIBRARY}
      ${LibUSB_LIBRARIES}
      ${OPENSSL_LIBRARIES}
      ${SODIUM_LIBRARY}
      ${HIDAPI_LIBRARIES}
      ${EXTRA_LIBRARIES}
  )

if (WIN32)
  target_link_options(wownero-cpp PUBLIC "-Wl,--enable-auto-import,--export-all-symbols")
endif()

  INSTALL(FILES src/daemon/wownero_daemon.h
          src/daemon/wownero_daemon_model.h
          DESTINATION include/daemon)
  INSTALL(FILES src/utils/gen_utils.h
          src/utils/wownero_utils.h
          DESTINATION include/utils)
  INSTALL(FILES src/wallet/wownero_wallet_full.h
          src/wallet/wownero_wallet.h
          src/wallet/wownero_wallet_keys.h
          src/wallet/wownero_wallet_model.h
          DESTINATION include/wallet)
  INSTALL(TARGETS wownero-cpp
          RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime
          LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Runtime
          ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Development)
endif()

########################
# Build C++ sample Code
########################

if (BUILD_SAMPLE)
  set(SAMPLE_CODE_SRC_FILES test/sample_code.cpp)
  
  add_executable(sample_code ${LIBRARY_SRC_FILES} ${SAMPLE_CODE_SRC_FILES})
  
  target_link_libraries(sample_code
      wallet
      rpc_base
      net
      lmdb
      easylogging
      cryptonote_core
      cryptonote_protocol
      cryptonote_basic
      cryptonote_format_utils_basic
      mnemonics
      ringct
      ringct_basic
      common
      blockchain_db
      blocks
      checkpoints
      device
      device_trezor
      cncrypto
      multisig
      version
      randomx
      epee
      hardforks
      ${wallet_crypto}

      ${UNBOUND_LIBRARIES}
      ${Boost_LIBRARIES}
      ${Protobuf_LIBRARY}
      ${LibUSB_LIBRARIES}
      ${OPENSSL_LIBRARIES}
      ${SODIUM_LIBRARY}
      ${HIDAPI_LIBRARIES}
      ${EXTRA_LIBRARIES}
  )
  if (NOT WIN32)
    target_link_libraries(sample_code dl)
  endif()
endif()

########################
# Build C++ scratchpad
########################

if (BUILD_SCRATCHPAD)
  set(SCRATCHPAD_SRC_FILES test/scratchpad.cpp)
  
  add_executable(scratchpad ${LIBRARY_SRC_FILES} ${SCRATCHPAD_SRC_FILES})
  
  target_link_libraries(scratchpad
      wallet
      rpc_base
      net
      lmdb
      easylogging
      cryptonote_core
      cryptonote_protocol
      cryptonote_basic
      cryptonote_format_utils_basic
      mnemonics
      ringct
      ringct_basic
      common
      blockchain_db
      blocks
      checkpoints
      device
      device_trezor
      cncrypto
      multisig
      version
      randomx
      epee
      hardforks
      ${wallet_crypto}
      
      ${UNBOUND_LIBRARIES}
      ${Boost_LIBRARIES}
      ${Protobuf_LIBRARY}
      ${LibUSB_LIBRARIES}
      ${OPENSSL_LIBRARIES}
      ${SODIUM_LIBRARY}
      ${HIDAPI_LIBRARIES}
      ${EXTRA_LIBRARIES}
  )
  if (NOT WIN32)
    target_link_libraries(scratchpad dl)
  endif()
endif()

########################
# Build C++ tests
########################

if (BUILD_TESTS)
  set(TEST_SRC_FILES test/wownero_tests.cpp)
  
  add_executable(wownero_tests ${LIBRARY_SRC_FILES} ${TEST_SRC_FILES})
  
  target_link_libraries(wownero_tests
      wallet
      rpc_base
      net
      lmdb
      easylogging
      cryptonote_core
      cryptonote_protocol
      cryptonote_basic
      cryptonote_format_utils_basic
      mnemonics
      ringct
      ringct_basic
      common
      blockchain_db
      blocks
      checkpoints
      device
      device_trezor
      cncrypto
      multisig
      version
      randomx
      epee
      hardforks
      ${wallet_crypto}
      
      ${UNBOUND_LIBRARIES}
      ${Boost_LIBRARIES}
      ${Protobuf_LIBRARY}
      ${LibUSB_LIBRARIES}
      ${OPENSSL_LIBRARIES}
      ${SODIUM_LIBRARY}
      ${HIDAPI_LIBRARIES}
      ${EXTRA_LIBRARIES}
  )
  if (NOT WIN32)
    target_link_libraries(wownero_tests dl)
  endif()
endif()