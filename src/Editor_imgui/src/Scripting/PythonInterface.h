/*-----------------------------------------------------------------------------------------------
The MIT License (MIT)

Copyright (c) 2015-2024 OSRE ( Open Source Render Engine ) by Kim Kulling

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
#pragma once

#include <osre/Common/osre_common.h>

#include <cppcore/Container/TArray.h>

namespace OSRE {

namespace App {
    class AppBase;
}

namespace Editor {

//-------------------------------------------------------------------------------------------------
///	@ingroup    Editor
///
/// @brief This class implements the global python entry points.
/// 
/// This class implements the main facade to the python scripting.
//-------------------------------------------------------------------------------------------------
class PythonInterface {
public:
    /// @brief  The class constructor.
    PythonInterface();

    /// @brief  The class default destructor.
    ~PythonInterface() = default;

    /// @brief Will create the python interface.
    /// @param app  The application instance.
    /// @return true, if succeded, false if not
    bool create(App::AppBase *app);

    /// @brief Will destroy the python interface.
    /// @return true, if succeded, false if not
    bool destroy();
    
    /// @brief Will add a python script path to the interpreter.
    /// @param path  The new path
    void addPath(const String &path);
    
    /// @brief Will execute a script.
    /// @param src  The script to execute.
    /// @return true, if succeded, false if not
    bool runScript(const String &src);

private:
    bool mCreated;
    StringArray mPaths;
    App::AppBase *mApp;
};

} // Namespace Editor
} // namespace OSRE
