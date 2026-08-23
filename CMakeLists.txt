cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake"
        CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(hocoslamfy C)

include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "Hocoslamfy")
set(VITA_TITLEID  "HOCO00001")
set(VITA_VERSION  "01.00")

set(SDL_INCLUDE "${VITASDK}/arm-vita-eabi/include/SDL")
set(SDL_LIBDIR  "${VITASDK}/arm-vita-eabi/lib")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -fomit-frame-pointer -DPSVITA -Wno-unused-variable")

add_executable(hocoslamfy
  main.c
  init.c
  title.c
  game.c
  score.c
  audio.c
  bg.c
  text.c
  unifont.c
  platform/vita.c
)

target_include_directories(hocoslamfy PRIVATE
  .
  ${SDL_INCLUDE}
)

target_link_libraries(hocoslamfy
  -Wl,-q
  -L${SDL_LIBDIR}
  -Wl,--start-group
  SDL_mixer
  SDL_image
  SDL
  SceGxm_stub
  SceDisplay_stub
  SceCtrl_stub
  SceAppMgr_stub
  SceAudio_stub
  SceIofilemgr_stub
  SceCommonDialog_stub
  SceTouch_stub
  SceHid_stub
  SceMotion_stub
  ScePower_stub
  SceProcessmgr_stub
  SceLibKernel_stub
  png
  jpeg
  webp
  z
  vorbisfile
  vorbis
  ogg
  FLAC
  mikmod
  mpg123
  pthread
  m
  -Wl,--end-group
)

vita_create_self(eboot.bin hocoslamfy UNSAFE)

vita_create_vpk(hocoslamfy.vpk ${VITA_TITLEID} eboot.bin
  VERSION ${VITA_VERSION}
  NAME    ${VITA_APP_NAME}
  FILE ${CMAKE_SOURCE_DIR}/sce_sys/icon0.png              sce_sys/icon0.png
  FILE ${CMAKE_SOURCE_DIR}/sce_sys/pic0.png               sce_sys/pic0.png
  FILE ${CMAKE_SOURCE_DIR}/sce_sys/livearea/contents/bg.png       sce_sys/livearea/contents/bg.png
  FILE ${CMAKE_SOURCE_DIR}/sce_sys/livearea/contents/startup.png  sce_sys/livearea/contents/startup.png
  FILE ${CMAKE_SOURCE_DIR}/sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
  FILE ${CMAKE_SOURCE_DIR}/data/Sky.png            data/Sky.png
  FILE ${CMAKE_SOURCE_DIR}/data/Mountains.png      data/Mountains.png
  FILE ${CMAKE_SOURCE_DIR}/data/Clouds3.png        data/Clouds3.png
  FILE ${CMAKE_SOURCE_DIR}/data/Clouds2.png        data/Clouds2.png
  FILE ${CMAKE_SOURCE_DIR}/data/Clouds1.png        data/Clouds1.png
  FILE ${CMAKE_SOURCE_DIR}/data/Grass3.png         data/Grass3.png
  FILE ${CMAKE_SOURCE_DIR}/data/Grass2.png         data/Grass2.png
  FILE ${CMAKE_SOURCE_DIR}/data/Grass1.png         data/Grass1.png
  FILE ${CMAKE_SOURCE_DIR}/data/TitleHeader1.png   data/TitleHeader1.png
  FILE ${CMAKE_SOURCE_DIR}/data/TitleHeader2.png   data/TitleHeader2.png
  FILE ${CMAKE_SOURCE_DIR}/data/TitleHeader3.png   data/TitleHeader3.png
  FILE ${CMAKE_SOURCE_DIR}/data/TitleHeader4.png   data/TitleHeader4.png
  FILE ${CMAKE_SOURCE_DIR}/data/TitleHeader5.png   data/TitleHeader5.png
  FILE ${CMAKE_SOURCE_DIR}/data/TitleHeader6.png   data/TitleHeader6.png
  FILE ${CMAKE_SOURCE_DIR}/data/TitleHeader7.png   data/TitleHeader7.png
  FILE ${CMAKE_SOURCE_DIR}/data/TitleHeader8.png   data/TitleHeader8.png
  FILE ${CMAKE_SOURCE_DIR}/data/Bee.png            data/Bee.png
  FILE ${CMAKE_SOURCE_DIR}/data/Bamboo.png         data/Bamboo.png
  FILE ${CMAKE_SOURCE_DIR}/data/Crash.png          data/Crash.png
  FILE ${CMAKE_SOURCE_DIR}/data/GameOverHeader.png data/GameOverHeader.png
  FILE ${CMAKE_SOURCE_DIR}/data/bgm.wav            data/bgm.wav
  FILE ${CMAKE_SOURCE_DIR}/data/fly.wav            data/fly.wav
  FILE ${CMAKE_SOURCE_DIR}/data/pass.wav           data/pass.wav
  FILE ${CMAKE_SOURCE_DIR}/data/collision.wav      data/collision.wav
  FILE ${CMAKE_SOURCE_DIR}/data/highscore.wav      data/highscore.wav
)
