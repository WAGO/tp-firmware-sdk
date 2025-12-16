//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project daemon-utils.
//
// Copyright (c) 2023-2024 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Utility class to handle get opt parsing.
///
///  \author   MaHe: WAGO GmbH & Co. KG
///  \author   PEn:  WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
#ifndef INC_WAGO_OPTPARSING_GET_OPT_PARSER_HPP_
#define INC_WAGO_OPTPARSING_GET_OPT_PARSER_HPP_

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "api.h"

#include <wc/structuring.h>
#include <string>
#include <functional>
#include <vector>
#include <memory>

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
namespace wago {
namespace optparsing {

/// Option identifier, also used as short option when in ranges [a-zA-Z] as char
/// reserved/disallowed values: 0, '?' and any kind of whitespace
using arg_option                = int;

/// Function signature to process custom argument values.
using arg_custom_func_signature = void(arg_option const option, std::string const &argument_text);

/// Function declaration to process custom argument values.
using arg_custom_func           = std::function<arg_custom_func_signature>;

/// Internal options parser implementation
class get_opt_parser_impl;

/// Configurable, lightweight option parser to process command line arguments,
/// with a small footprint.
///
/// \remark This class and its methods are not thread-safe.
class WAGO_OPTPARSING_API get_opt_parser
{
    WC_DISBALE_CLASS_COPY_AND_ASSIGN(get_opt_parser)

public:
    //------------------------------------------------------------------------------
    /// Creates a new parser object.
    ///
    /// \param name
    ///   Program name this parser acts for (e.g. the basename of program call).
    /// \param call_string
    ///   The command this program was called with (mostly argv[0]).
    /// \param quiet
    ///   If true, no failure information will be printed to std::err.
    /// \param auto_help
    ///   If true, common help text option (-h/--help) will be added automatically.
    //------------------------------------------------------------------------------
    WAGO_OPTPARSING_API get_opt_parser(std::string const &name,
                                       std::string const &call_string,
                                       bool        const  quiet       = false,
                                       bool        const  auto_help   = true);

    //------------------------------------------------------------------------------
    /// Parser move.
    //------------------------------------------------------------------------------
    get_opt_parser(get_opt_parser&& src) noexcept;

    //------------------------------------------------------------------------------
    /// Parser move assign.
    //------------------------------------------------------------------------------
    get_opt_parser& operator=(get_opt_parser&& src) noexcept;

    //------------------------------------------------------------------------------
    /// Parser destruction.
    //------------------------------------------------------------------------------
    WAGO_OPTPARSING_API virtual ~get_opt_parser() noexcept;


    // Argument configuration
    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------
    /// Method to add a boolean option without value/argument,
    /// just present/enabled or not.
    ///
    /// \param option_id
    ///   Option identifier, also used as short option when ASCII char [a-zA-Z].
    /// \param long_option
    ///   Long option string, may be empty (not recommended) if option ID is usable as short option.
    /// \param argument_destination
    ///   Variable to store option result after parsing.
    /// \param help_text
    ///   Help text for option, may be empty (not recommended).
    ///
    /// \throw std::runtime_error
    ///   Runtime exceptions are thrown if requested option setup is not acceptable
    ///   (e.g. not unique, contains whitespace, dead).
    /// \throw std::bad_alloc
    ///   Bad alloc exceptions may be thrown if an allocation request does not succeed.
    //------------------------------------------------------------------------------
    WAGO_OPTPARSING_API void add_flag(    arg_option  const  option_id,
                                          char        const *long_option,
                                          bool              &argument_destination,
                                          char        const *help_text);

    //------------------------------------------------------------------------------
    /// Method to add an option without value/argument,
    /// counted because allowed multiple times
    /// (e.g. to change a behavior step by step like -vvv).
    ///
    /// \param option_id
    ///   Option identifier, also used as short option when ASCII char [a-zA-Z].
    /// \param long_option
    ///   Long option string, may be empty (not recommended) if option ID is usable as short option.
    /// \param argument_destination
    ///   Variable to store option result after parsing.
    /// \param help_text
    ///   Help text for option, may be empty (not recommended).
    ///
    /// \throw std::runtime_error
    ///   Runtime exceptions are thrown if requested option setup is not acceptable
    ///   (e.g. not unique, contains whitespace, dead).
    /// \throw std::bad_alloc
    ///   Bad alloc exceptions may be thrown if an allocation request does not succeed.
    //------------------------------------------------------------------------------
    WAGO_OPTPARSING_API void add_flag(    arg_option  const  option_id,
                                          char        const *long_option,
                                          size_t            &argument_destination,
                                          char        const *help_text);

    //------------------------------------------------------------------------------
    /// Method to add an option with unsigned number value.
    ///
    /// \param option_id
    ///   Option identifier, also used as short option when ASCII char [a-zA-Z].
    /// \param long_option
    ///   Long option string, may be empty (not recommended) if option ID is usable as short option.
    /// \param argument_destination
    ///   Variable to store option result after parsing.
    /// \param help_text
    ///   Help text for option, may be empty (not recommended).
    ///
    /// \throw std::runtime_error
    ///   Runtime exceptions are thrown if requested option setup is not acceptable
    ///   (e.g. not unique, contains whitespace, dead).
    /// \throw std::bad_alloc
    ///   Bad alloc exceptions may be thrown if an allocation request does not succeed.
    //------------------------------------------------------------------------------
    WAGO_OPTPARSING_API void add_argument(arg_option  const  option_id,
                                          char        const *long_option,
                                          uint64_t          &argument_destination,
                                          char        const *help_text);

    //------------------------------------------------------------------------------
    /// Method to add an option with signed number value.
    ///
    /// \param option_id
    ///   Option identifier, also used as short option when ASCII char [a-zA-Z].
    /// \param long_option
    ///   Long option string, may be empty (not recommended) if option ID is usable as short option.
    /// \param argument_destination
    ///   Variable to store option result after parsing.
    /// \param help_text
    ///   Help text for option, may be empty (not recommended).
    ///
    /// \throw std::runtime_error
    ///   Runtime exceptions are thrown if requested option setup is not acceptable
    ///   (e.g. not unique, contains whitespace, dead).
    /// \throw std::bad_alloc
    ///   Bad alloc exceptions may be thrown if an allocation request does not succeed.
    //------------------------------------------------------------------------------
    WAGO_OPTPARSING_API void add_argument(arg_option  const  option_id,
                                          char        const *long_option,
                                          int64_t           &argument_destination,
                                          char        const *help_text);

    //------------------------------------------------------------------------------
    /// Method to add an option with text value.
    ///
    /// \param option_id
    ///   Option identifier, also used as short option when ASCII char [a-zA-Z].
    /// \param long_option
    ///   Long option string, may be empty (not recommended) if option ID is usable as short option.
    /// \param argument_destination
    ///   Variable to store option result after parsing.
    /// \param help_text
    ///   Help text for option, may be empty (not recommended).
    ///
    /// \throw std::runtime_error
    ///   Runtime exceptions are thrown if requested option setup is not acceptable
    ///   (e.g. not unique, contains whitespace, dead).
    /// \throw std::bad_alloc
    ///   Bad alloc exceptions may be thrown if an allocation request does not succeed.
    //------------------------------------------------------------------------------
    WAGO_OPTPARSING_API void add_argument(arg_option  const  option_id,
                                          char        const *long_option,
                                          std::string       &argument_destination,
                                          char        const *help_text);

    //------------------------------------------------------------------------------
    /// Method to add an option with value, processed by a custom function.
    ///
    /// \param option_id
    ///   Option identifier, also used as short option when ASCII char [a-zA-Z].
    /// \param long_option
    ///   Long option string, may be empty (not recommended) if option ID is usable as short option.
    /// \param converter_function
    ///   Converter function to process custom option value.
    /// \param help_text
    ///   Help text for option, may be empty (not recommended).
    /// \param custom_options
    ///   Optional, additional help text fragment to list allowed custom options or option pattern
    ///   that will be processed successfully by given converter function.
    ///
    /// \throw std::runtime_error
    ///   Runtime exceptions are thrown if requested option setup is not acceptable
    ///   (e.g. not unique, contains whitespace, dead).
    /// \throw std::bad_alloc
    ///   Bad alloc exceptions may be thrown if an allocation request does not succeed.
    //------------------------------------------------------------------------------
    WAGO_OPTPARSING_API void add_argument(arg_option            const          option_id,
                                          char                  const * const  long_option,
                                          arg_custom_func                     &converter_function,
                                          char                  const * const  help_text,
                                          char                  const * const  custom_options = nullptr);


    // Help text configuration
    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------
    /// Sets the optional title over generated help text.
    ///
    /// \param help_title
    ///   Title to set (should be a single line only).
    //------------------------------------------------------------------------------
    WAGO_OPTPARSING_API void set_help_title(std::string const &help_title);

    //------------------------------------------------------------------------------
    /// Adds an additional text at the end of the generated help.
    ///
    /// \param help_additional_info
    ///   Additional help text to add.
    //------------------------------------------------------------------------------
    WAGO_OPTPARSING_API void set_help_additional_info(std::string const &help_additional_info);

    //------------------------------------------------------------------------------
    /// Adds an example call of this program with explanation to the generated help text.
    ///
    /// \param example_call
    ///   Program call with example parameters.
    /// \param example_description
    ///   Description/explanation of the given example program call.
    ///
    /// \throw std::bad_alloc
    ///   Bad alloc exceptions may be thrown if an allocation request does not succeed.
    //------------------------------------------------------------------------------
    WAGO_OPTPARSING_API void add_help_example(std::string const &example_call,
                                              std::string const &example_description);


    // Parser interaction
    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------
    /// Parses a given options vector with the current set of configured arguments.
    ///
    /// \param argc
    ///   Option/argument count of argv (count of elements - 1).
    /// \param argv
    ///   Argument vector with program call / command as first element.
    ///
    /// \throw std::runtime_error
    ///   Runtime exceptions are thrown if arguments are invalid for an option
    ///   (e.g. a requested conversion could not be performed or is out of range).
    //------------------------------------------------------------------------------
    WAGO_OPTPARSING_API void parse(int   argc,
                                   char *argv[]);

    //------------------------------------------------------------------------------
    /// Determines if the arguments contained a request for help (-h/--help) on last
    /// call of \ref parse() with activated auto help (\see \ref get_opt_parser()).
    ///
    /// \return
    ///   True, if help was requested.
    //------------------------------------------------------------------------------
    WAGO_OPTPARSING_API bool is_help_requested();

    //------------------------------------------------------------------------------
    /// Generates a help text from currently configured arguments/options and
    /// optionally defined help text elements (e.g. title, example calls).
    ///
    /// \throw std::bad_alloc
    ///   Bad alloc exceptions may be thrown if an allocation request does not succeed.
    ///
    /// \see set_help_title
    /// \see set_help_additional_info
    /// \see add_help_example
    ///
    /// \return
    ///   Generated help text.
    //------------------------------------------------------------------------------
    WAGO_OPTPARSING_API std::string generate_help_text();

private:
    std::unique_ptr<get_opt_parser_impl> impl_m;
};

} // Namespace optparsing
} // Namespace wago


#endif // INC_WAGO_OPTPARSING_GET_OPT_PARSER_HPP_
//---- End of source file ------------------------------------------------------
