# Linux / Windows どちらでもビルドできる。本番の Linux が gcc 7.3 なので C++17 まで。
#
# Visual Studio の .vcxproj と二重管理になっているが、どちらも「ソースを並べて C++17 で
# 繋ぐ」以上のことをしていない。実害はソースを増やしたときに2箇所直すことだけ。

cmake_minimum_required(VERSION 3.10)
project(HLAGateway CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(HLAGateway
    main.cpp
    gateway/CGateway.cpp
    gateway/udp/CPoller.cpp
    gateway/udp/CUdpSocket.cpp
    platform/Platform.cpp
    ICD/icd_types.cpp
    ICD/Object/MinefieldData.cpp
    ICD/Object/RadarBeam.cpp
    ICD/Object/RadioReceiver.cpp
    ICD/Interaction/WeaponFire.cpp
)

target_compile_options(HLAGateway PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -pedantic>
    # MSVC にソースと実行文字セットの両方を UTF-8 と教える。これが無いと日本語の
    # 文字列リテラルが CP932 になり、gcc 側と実行時の出力が食い違う。
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /utf-8>
)

if(WIN32)
    # ws2_32 = ソケット、winmm = timeBeginPeriod（周期ループの起床精度）
    target_link_libraries(HLAGateway PRIVATE ws2_32 winmm)
else()
    # std::this_thread::sleep_until が pthread を要る構成がある
    find_package(Threads REQUIRED)
    target_link_libraries(HLAGateway PRIVATE Threads::Threads)
endif()
