set(BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Linux64/")

set(_MINECRAFT_CLIENT_LINUX64_PLATFORM
  "${BASE_DIR}/Linux64_Minecraft.cpp"
  "${BASE_DIR}/Linux64_App.h"
  "${BASE_DIR}/Linux64_App.cpp"
  "${BASE_DIR}/KeyboardMouseInput.h"
  "${BASE_DIR}/KeyboardMouseInput.cpp"
  "${BASE_DIR}/Linux64_Compat.h"
  "${BASE_DIR}/Iggy/include/iggy_stub.h"
)
source_group("Linux64" FILES ${_MINECRAFT_CLIENT_LINUX64_PLATFORM})

set(_MINECRAFT_CLIENT_LINUX64_4JLIBS_INC
  "${BASE_DIR}/4JLibs/inc/4J_Input.h"
  "${BASE_DIR}/4JLibs/inc/4J_Profile.h"
  "${BASE_DIR}/4JLibs/inc/4J_Render.h"
  "${BASE_DIR}/4JLibs/inc/4J_Storage.h"
)
source_group("Linux64/4JLibs/inc" FILES ${_MINECRAFT_CLIENT_LINUX64_4JLIBS_INC})

set(_MINECRAFT_CLIENT_LINUX64_NETWORK
  "${BASE_DIR}/Network/WinsockNetLayer.h"
  "${BASE_DIR}/Network/WinsockNetLayer.cpp"
)
source_group("Linux64/Network" FILES ${_MINECRAFT_CLIENT_LINUX64_NETWORK})

set(_MINECRAFT_CLIENT_LINUX64_COMMON_AUDIO
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/Audio/SoundEngine.cpp"
)
source_group("Common/Audio" FILES ${_MINECRAFT_CLIENT_LINUX64_COMMON_AUDIO})

set(_MINECRAFT_CLIENT_LINUX64_COMMON_NETWORK
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/Network/PlatformNetworkManagerStub.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/Network/PlatformNetworkManagerStub.h"
)
source_group("Common/Network" FILES ${_MINECRAFT_CLIENT_LINUX64_COMMON_NETWORK})

set(_MINECRAFT_CLIENT_LINUX64_COMMON_UI
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UI.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIBitmapFont.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIBitmapFont.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIController.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIController.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIGroup.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIGroup.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UILayer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UILayer.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UIScene.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UITTFFont.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/UI/UITTFFont.h"
)
source_group("Common/UI" FILES ${_MINECRAFT_CLIENT_LINUX64_COMMON_UI})

set(_MINECRAFT_CLIENT_LINUX64_COMMON_ZLIB
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/adler32.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/compress.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/crc32.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/deflate.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/gzclose.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/gzlib.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/gzread.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/gzwrite.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/infback.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/inffast.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/inflate.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/inftrees.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/trees.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/uncompr.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/Common/zlib/zutil.c"
)
source_group("Common/zlib" FILES ${_MINECRAFT_CLIENT_LINUX64_COMMON_ZLIB})

set(_MINECRAFT_CLIENT_LINUX64_SOURCE_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/Extrax64Stubs.cpp"
)
source_group("Source Files" FILES ${_MINECRAFT_CLIENT_LINUX64_SOURCE_FILES})

set(MINECRAFT_CLIENT_LINUX64
  ${_MINECRAFT_CLIENT_LINUX64_PLATFORM}
  ${_MINECRAFT_CLIENT_LINUX64_4JLIBS_INC}
  ${_MINECRAFT_CLIENT_LINUX64_NETWORK}
  ${_MINECRAFT_CLIENT_LINUX64_COMMON_AUDIO}
  ${_MINECRAFT_CLIENT_LINUX64_COMMON_NETWORK}
  ${_MINECRAFT_CLIENT_LINUX64_COMMON_UI}
  ${_MINECRAFT_CLIENT_LINUX64_COMMON_ZLIB}
  ${_MINECRAFT_CLIENT_LINUX64_SOURCE_FILES}
)
