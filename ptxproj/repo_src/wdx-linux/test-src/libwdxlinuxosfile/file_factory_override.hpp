//------------------------------------------------------------------------------
// Copyright (c) 2022-2025 WAGO GmbH & Co. KG
//
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Allows configuration of the overriden file factory.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef TEST_SRC_LIBWDXLINUXOSFILE_FILE_FACTORY_OVERRIDE_HPP_
#define TEST_SRC_LIBWDXLINUXOSFILE_FILE_FACTORY_OVERRIDE_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "file_i.hpp"
#include "mocks/mock_file.hpp"

#include <wc/structuring.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
class file_factory_override final
{
public:
    file_factory_override();
    ~file_factory_override();

private:
    WC_DISBALE_CLASS_COPY_AND_ASSIGN(file_factory_override)

public:
    /// Returns mock to be used on the very next factory call
    mock_file *get_next_to_be_created_mock();
};

#endif // TEST_SRC_LIBWDXLINUXOSFILE_FILE_FACTORY_OVERRIDE_HPP_
//---- End of source file ------------------------------------------------------

