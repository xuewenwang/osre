/*-----------------------------------------------------------------------------------------------
The MIT License (MIT)

Copyright (c) 2015 OSRE ( Open Source Render Engine ) by Kim Kulling

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
-----------------------------------------------------------------------------------------------*/
#include "AbstractRenderTest.h"

#include <osre/RenderBackend/RenderCommon.h>
#include <osre/IO/IOService.h>
#include <osre/IO/Stream.h>
#include <osre/Common/Logger.h>

#include <cassert>

namespace OSRE {
namespace RenderTest {

using namespace ::OSRE::RenderBackend;

//-------------------------------------------------------------------------------------------------
AbstractRenderTest::AbstractRenderTest( const String &renderTestName )
: m_RenderTestName( renderTestName )
, m_State( 0 ) {
    // empty
}

//-------------------------------------------------------------------------------------------------
AbstractRenderTest::~AbstractRenderTest() {
    // empty
}

//---------------------------------------------------------------------------------------------
bool AbstractRenderTest::create( RenderBackendService *pRenderBackendSrv ) {
    assert( nullptr != pRenderBackendSrv );
    
    osre_info( m_RenderTestName, "=> Creating test." );
    return onCreate( pRenderBackendSrv );
}

//---------------------------------------------------------------------------------------------
bool AbstractRenderTest::destroy( RenderBackendService *pRenderBackendSrv ) {
    assert( nullptr != pRenderBackendSrv );
    
    osre_info( m_RenderTestName, "<= Destroying test." );

    return onDestroy( pRenderBackendSrv );
}

//---------------------------------------------------------------------------------------------
bool AbstractRenderTest::render( d32 timediff, RenderBackendService *pRenderBackendSrv ) {
    assert( nullptr != pRenderBackendSrv );

    return onRender( timediff, pRenderBackendSrv );
}

//---------------------------------------------------------------------------------------------
bool AbstractRenderTest::onCreate( RenderBackendService *pRenderBackendSrv ) {
    return true;
}

//---------------------------------------------------------------------------------------------
bool AbstractRenderTest::onDestroy( RenderBackendService *pRenderBackendSrv ) {
    return true;
}

//---------------------------------------------------------------------------------------------
void AbstractRenderTest::setup( RenderBackendService *pRenderBackendSrv ) {
    // empty
}

//---------------------------------------------------------------------------------------------
void AbstractRenderTest::teardown( RenderBackendService *pRenderBackendSrv ) {
    // empty
}

//---------------------------------------------------------------------------------------------
bool AbstractRenderTest::updateSnaptShot() {
    if( !AbstractRenderTest::hasState( AbstractRenderTest::ScreenShotGeneated ) ) {
        String name = getTestName();
        IO::Uri screenshot( "file://./" + name + ".jpg" );
        IO::AbstractFileSystem *pFS = IO::IOService::getInstance()->getFileSystem( "file" );

        if( !pFS ) {
            AbstractRenderTest::setState( AbstractRenderTest::ScreenShotGeneated );
            return false;
        }

        IO::Stream *pFileStream = pFS->open( screenshot, IO::Stream::WriteAccess );
        if( pFileStream ) {
            //pRenderDevice->makeScreenshot( pFileStream );
            pFS->close( &pFileStream );
        }
        pFS->release();

        AbstractRenderTest::setState( AbstractRenderTest::ScreenShotGeneated );
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
const String &AbstractRenderTest::getTestName() const {
    return m_RenderTestName;
}

//-------------------------------------------------------------------------------------------------
void AbstractRenderTest::setState( TestState state ) {
    m_State |= state;
}

//-------------------------------------------------------------------------------------------------
bool AbstractRenderTest::hasState( TestState state ) const {
    if( m_State & state ) {
        return true;
    } else {
        return false;
    }
}

//-------------------------------------------------------------------------------------------------
RenderBackend::Material *AbstractRenderTest::createMaterial( const String &VsSrc, 
                                                             const String &FsSrc ) {
    Material *mat      = new Material;
    mat->m_numTextures = 0;
    mat->m_type        = ShaderMaterial;
    mat->m_pShader     = new Shader;
    mat->m_pShader->m_src[ SH_VertexShaderType ] = VsSrc;
    mat->m_pShader->m_src[ SH_FragmentShaderType ] = FsSrc;

    return mat;
}

//-------------------------------------------------------------------------------------------------

} // Namespace RenderTest
} // Namespace OSRE
