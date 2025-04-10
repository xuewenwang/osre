find_package(GTest CONFIG REQUIRED)

if( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )
    find_package(Threads)
endif()

INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}
    ../../contrib/cppcore/include
    ../../contrib/assimp/include
    ../../contrib/glew/include
    ../../contrib/glm/
    ../../contrib/soil/src
    .././
    src
)

SET ( unittest_app_src
    src/App/TAbstractCtrlBaseTest.cpp
    src/App/ProjectTest.cpp
    src/App/AssetBundleTest.cpp
    src/App/AssetRegistryTest.cpp
    src/App/AssetWrapperTest.cpp
)

SET ( unittest_common_src
    src/Common/AbstractProcessTest.cpp
    src/Common/ArgumentParserTest.cpp
    src/Common/AbstractServiceTest.cpp
    src/Common/BaseMathTest.cpp
    src/Common/CommonTest.cpp
    src/Common/ObjectTest.cpp
    src/Common/EventTest.cpp
    src/Common/EventBusTest.cpp
    src/Common/IdsTest.cpp
    src/Common/FrustumTest.cpp
    src/Common/LoggerTest.cpp
    src/Common/TRayTest.cpp
)

SET ( unittest_collision_src
)

SET ( unittest_debugging_src
    src/Debugging/osre_debugging_test.cpp
)

SET( unittest_io_src 
    src/IO/UriTest.cpp
)

SET( unittest_platform_src
    src/Platform/AbstractDynamicLoaderTest.cpp
    src/Platform/AbstractThreadTest.cpp
)

SET ( unittest_rb_src
    src/RenderBackend/RenderBackendServiceTest.cpp
    src/RenderBackend/CullStateTest.cpp
    src/RenderBackend/RenderCommonTest.cpp
    src/RenderBackend/PipelineTest.cpp
    src/RenderBackend/MeshTest.cpp
    src/RenderBackend/ShaderTest.cpp
)

SET (unittest_rb_2d_src
    src/RenderBackend/2D/CanvasRendererTest.cpp
)

SET( unittest_rb_oglrenderer_src 
    src/RenderBackend/OGLRenderer/GLEnumTest.cpp
)

SET ( unittest_profiling_src
    src/Profiling/PerformanceCountersTest.cpp
)

SET ( unittest_scene_src
    src/Scene/ComponentTest.cpp
    src/Scene/DbgRendererTest.cpp
    src/Scene/GeometryBuilderTest.cpp
    src/Scene/NodeTest.cpp
    src/Scene/SceneTest.cpp
    src/Scene/TAABBTest.cpp
)

SOURCE_GROUP( src\\App                        FILES ${unittest_app_src} )
SOURCE_GROUP( src\\Common                     FILES ${unittest_common_src} )
SOURCE_GROUP( src\\Collision                  FILES ${unittest_collision_src})
SOURCE_GROUP( src\\Debugging                  FILES ${unittest_debugging_src})
SOURCE_GROUP( src\\IO                         FILES ${unittest_io_src} )
SOURCE_GROUP( src\\Platform                   FILES ${unittest_platform_src})
SOURCE_GROUP( src\\Profiling                  FILES ${unittest_profiling_src})
SOURCE_GROUP( src\\RenderBackend              FILES ${unittest_rb_src} )
SOURCE_GROUP( src\\RenderBackend\\2D          FILES ${unittest_rb_2d_src} )
SOURCE_GROUP( src\\RenderBackend\\OGLRenderer FILES ${unittest_rb_oglrenderer_src} )
SOURCE_GROUP( src\\Scene                      FILES ${unittest_scene_src} )

ADD_EXECUTABLE( osre_unittest
    src/osre_testcommon.h
    ${unittest_app_src}
    ${unittest_common_src}
    ${unittest_collision_src}
    ${unittest_debugging_src}
    ${unittest_io_src}
    ${unittest_math_src}
    ${unittest_platform_src}
    ${unittest_profiling_src}
    ${unittest_rb_src}
    ${unittest_rb_oglrenderer_src}
    ${unittest_rb_2d_src}
    ${unittest_ui_src}
    ${unittest_scene_src}
)

link_directories( 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty/glew/Debug
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty/glew/Release
)

IF( WIN32 )
    SET( platform_libs )
ELSE( WIN32 )
    SET( platform_libs pthread )
ENDIF( WIN32 )

target_link_libraries ( osre_unittest osre ${platform_libs} GTest::gtest GTest::gtest_main) 
set_target_properties(  osre_unittest PROPERTIES FOLDER Tests )
