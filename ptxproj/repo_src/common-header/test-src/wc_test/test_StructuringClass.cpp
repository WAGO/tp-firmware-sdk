//------------------------------------------------------------------------------
// Copyright (c) 2021-2025 WAGO GmbH & Co. KG
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
///  \brief    Test for structuring helper macros targeting classes.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wc/structuring.h"

#include <type_traits>
#include <gtest/gtest.h>

//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------
class interface
{
  WC_INTERFACE_CLASS(interface);
};

class derived_from_interface : public interface
{

};

class impl_of_interface : public interface
{
  WC_INTERFACE_IMPL_CLASS(impl_of_interface);
  impl_of_interface() = default;
};

class without_limitations
{

};

class with_disabled_move
{
public:
  with_disabled_move() = default;
  with_disabled_move(with_disabled_move const&) = default;
  with_disabled_move& operator=(with_disabled_move const&) = default;
  WC_DISBALE_CLASS_MOVE(with_disabled_move);
};

class with_disabled_move_assign
{
public:
  with_disabled_move_assign() = default;
  with_disabled_move_assign(with_disabled_move_assign const&) = default;
  with_disabled_move_assign& operator=(with_disabled_move_assign const&) = default;
  WC_DISBALE_CLASS_MOVE_ASSIGN(with_disabled_move_assign);
};

class with_disabled_copy_assign
{
public:
  with_disabled_copy_assign() = default;
  WC_DISBALE_CLASS_COPY_AND_ASSIGN(with_disabled_copy_assign);
  with_disabled_copy_assign(with_disabled_copy_assign&&) = default;
  with_disabled_copy_assign& operator=(with_disabled_copy_assign&&) = default;
};

class with_disabled_copy_assign_move
{
public:
  with_disabled_copy_assign_move() = default;
  WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE(with_disabled_copy_assign_move);
  with_disabled_copy_assign_move& operator=(with_disabled_copy_assign_move&&) = default;
};

class with_disabled_copy_assign_move_assign
{
public:
  with_disabled_copy_assign_move_assign() = default;
  WC_DISBALE_CLASS_COPY_ASSIGN_AND_MOVE_ASSIGN(with_disabled_copy_assign_move_assign);
};

//------------------------------------------------------------------------------
// test implementation
//------------------------------------------------------------------------------
TEST(Structuring, class_interface)
{
  // For expected behavior of C++ interfaces see also:
  // - https://stackoverflow.com/a/30885842
  // - https://softwareengineering.stackexchange.com/questions/235674/what-is-the-pattern-for-a-safe-interface-in-c

  // Interface
  EXPECT_TRUE(std::is_polymorphic<interface>::value);

  EXPECT_FALSE(std::is_default_constructible<interface>::value);

  EXPECT_FALSE(std::is_copy_constructible<interface>::value);
  EXPECT_FALSE(std::is_copy_assignable<interface>::value);

  EXPECT_FALSE(std::is_move_constructible<interface>::value);
  EXPECT_FALSE(std::is_move_assignable<interface>::value);

  // Derived class from interface
  EXPECT_TRUE(std::is_polymorphic<derived_from_interface>::value);

  EXPECT_TRUE(std::is_copy_constructible<derived_from_interface>::value);
  EXPECT_TRUE(std::is_copy_assignable<derived_from_interface>::value);

  EXPECT_TRUE(std::is_move_constructible<derived_from_interface>::value);
  EXPECT_TRUE(std::is_move_assignable<derived_from_interface>::value);

  derived_from_interface test_instance;
}

TEST(Structuring, class_impl_of_interface)
{
  // Interface implementation
  EXPECT_TRUE(std::is_polymorphic<impl_of_interface>::value);

  EXPECT_FALSE(std::is_default_constructible<impl_of_interface>::value);

  EXPECT_FALSE(std::is_copy_constructible<impl_of_interface>::value);
  EXPECT_FALSE(std::is_copy_assignable<impl_of_interface>::value);

  EXPECT_FALSE(std::is_move_constructible<impl_of_interface>::value);
  EXPECT_FALSE(std::is_move_assignable<impl_of_interface>::value);
}

TEST(Structuring, class_without_limitations)
{
  EXPECT_FALSE(std::is_polymorphic<without_limitations>::value);

  EXPECT_TRUE(std::is_copy_constructible<without_limitations>::value);
  EXPECT_TRUE(std::is_copy_assignable<without_limitations>::value);

  EXPECT_TRUE(std::is_move_constructible<without_limitations>::value);
  EXPECT_TRUE(std::is_move_assignable<without_limitations>::value);
}

TEST(Structuring, class_with_disabled_move)
{
  EXPECT_FALSE(std::is_polymorphic<without_limitations>::value);

  EXPECT_TRUE(std::is_copy_constructible<with_disabled_move>::value);
  EXPECT_TRUE(std::is_copy_assignable<with_disabled_move>::value);

  EXPECT_FALSE(std::is_move_constructible<with_disabled_move>::value);
  EXPECT_TRUE(std::is_move_assignable<with_disabled_move>::value);
}

TEST(Structuring, class_with_disabled_move_assign)
{
  EXPECT_FALSE(std::is_polymorphic<without_limitations>::value);

  EXPECT_TRUE(std::is_copy_constructible<with_disabled_move_assign>::value);
  EXPECT_TRUE(std::is_copy_assignable<with_disabled_move_assign>::value);

  EXPECT_FALSE(std::is_move_constructible<with_disabled_move_assign>::value);
  EXPECT_FALSE(std::is_move_assignable<with_disabled_move_assign>::value);
}

TEST(Structuring, class_with_disabled_copy_assign)
{
  EXPECT_FALSE(std::is_polymorphic<without_limitations>::value);

  EXPECT_FALSE(std::is_copy_constructible<with_disabled_copy_assign>::value);
  EXPECT_FALSE(std::is_copy_assignable<with_disabled_copy_assign>::value);

  EXPECT_TRUE(std::is_move_constructible<with_disabled_copy_assign>::value);
  EXPECT_TRUE(std::is_move_assignable<with_disabled_copy_assign>::value);
}

TEST(Structuring, class_with_disabled_copy_assign_move)
{
  EXPECT_FALSE(std::is_polymorphic<without_limitations>::value);

  EXPECT_FALSE(std::is_copy_constructible<with_disabled_copy_assign_move>::value);
  EXPECT_FALSE(std::is_copy_assignable<with_disabled_copy_assign_move>::value);

  EXPECT_FALSE(std::is_move_constructible<with_disabled_copy_assign_move>::value);
  EXPECT_TRUE(std::is_move_assignable<with_disabled_copy_assign_move>::value);
}

TEST(Structuring, class_with_disabled_copy_assign_move_assign)
{
  EXPECT_FALSE(std::is_polymorphic<without_limitations>::value);

  EXPECT_FALSE(std::is_copy_constructible<with_disabled_copy_assign_move_assign>::value);
  EXPECT_FALSE(std::is_copy_assignable<with_disabled_copy_assign_move_assign>::value);

  EXPECT_FALSE(std::is_move_constructible<with_disabled_copy_assign_move_assign>::value);
  EXPECT_FALSE(std::is_move_assignable<with_disabled_copy_assign_move_assign>::value);
}


//---- End of source file ------------------------------------------------------

