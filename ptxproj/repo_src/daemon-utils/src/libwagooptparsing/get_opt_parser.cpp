//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This file is part of project wdx-linux.
//
// Copyright (c) 2023-2025 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
///  \file
///
///  \brief    Implementation for argument parser based on getopt_long.
///
///  \author   PEn: WAGO GmbH & Co. KG
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------------
#include "wago/optparsing/get_opt_parser.hpp"

#include <wc/assertion.h>
#include <wc/std_includes.h>
#include <wc/structuring.h>

#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <getopt.h>

namespace wago {
namespace optparsing {

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
namespace {

bool is_short_option_character(int value);
void check_short_option_value(arg_option const short_option);

//------------------------------------------------------------------------------
// variables' and constants' definitions
//------------------------------------------------------------------------------
#define PARAMETER_USAGE_DESCRIPTION                               "<command option> [<param>]"

constexpr char          const g_default_help_option_text[]      = "Prints this help text.";
constexpr char          const g_required_argument_placeholder[] = "<param>";
constexpr char          const g_optional_argument_placeholder[] = "[param]";
constexpr char          const g_help_option_format[]            = "  %s%-25s %-7s   %c%c   %s\n";
constexpr char          const g_empty[]                         = "";

}


//------------------------------------------------------------------------------
// defines; structure, enumeration and type definitions
//------------------------------------------------------------------------------

/// Enum of available options/arguments
enum class arg_type
{
    flag,           //!< flag:            A boolean option without value/argument, just present/enabled or not
    counted_flag,   //!< counted_flag:    An option without value/argument, counted because allowed multiple times
    unsigned_number,//!< unsigned_number: An option with unsigned number value
    signed_number,  //!< signed_number:   An option with signed number value
    plain,          //!< plain:           An option with text value
    custom          //!< custom:          An option with value, processed by a custom function
};

class argument
{
public:
    arg_type const current_type_m;
    union
    {
        bool        *argument_destination_flag_m;
        unsigned    *argument_destination_counted_flag_m;
        uint64_t    *argument_destination_unsigned_number_m;
        int64_t     *argument_destination_signed_number_m;
        std::string *argument_destination_plain_m;
    };
    arg_custom_func  argument_destination_custom_m;

    arg_option const         short_option_m;
    char       const * const long_option_m;
    char       const * const help_text_m;
    char       const * const custom_options_m;

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    argument(bool                      &argument_destination,
             arg_option  const          short_option,
             char        const * const  long_option,
             char        const * const  help_text) noexcept
    : current_type_m(arg_type::flag)
    , argument_destination_flag_m(&argument_destination)
    , short_option_m(short_option)
    , long_option_m(long_option)
    , help_text_m(help_text)
    , custom_options_m(nullptr)
    { }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    argument(unsigned                  &argument_destination,
             arg_option  const          short_option,
             char        const * const  long_option,
             char        const * const  help_text) noexcept
    : current_type_m(arg_type::counted_flag)
    , argument_destination_counted_flag_m(&argument_destination)
    , short_option_m(short_option)
    , long_option_m(long_option)
    , help_text_m(help_text)
    , custom_options_m(nullptr)
    { }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    argument(uint64_t                  &argument_destination,
             arg_option  const          short_option,
             char        const * const  long_option,
             char        const * const  help_text) noexcept
    : current_type_m(arg_type::unsigned_number)
    , argument_destination_unsigned_number_m(&argument_destination)
    , short_option_m(short_option)
    , long_option_m(long_option)
    , help_text_m(help_text)
    , custom_options_m(nullptr)
    { }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    argument(int64_t                   &argument_destination,
             arg_option  const          short_option,
             char        const * const  long_option,
             char        const * const  help_text) noexcept
    : current_type_m(arg_type::signed_number)
    , argument_destination_signed_number_m(&argument_destination)
    , short_option_m(short_option)
    , long_option_m(long_option)
    , help_text_m(help_text)
    , custom_options_m(nullptr)
    { }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    argument(std::string               &argument_destination,
             arg_option  const          short_option,
             char        const * const  long_option,
             char        const * const  help_text) noexcept
    : current_type_m(arg_type::plain)
    , argument_destination_plain_m(&argument_destination)
    , short_option_m(short_option)
    , long_option_m(long_option)
    , help_text_m(help_text)
    , custom_options_m(nullptr)
    { }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    argument(arg_custom_func               converter_function,
             arg_option      const         short_option,
             char            const * const long_option,
             char            const * const help_text,
             char            const * const custom_options = nullptr)
    : current_type_m(arg_type::custom)
    , argument_destination_custom_m(std::move(converter_function))
    , short_option_m(short_option)
    , long_option_m(long_option)
    , help_text_m(help_text)
    , custom_options_m(custom_options)
    { }

    bool has_required_argument()
    {
        return    (current_type_m != arg_type::flag)
               && (current_type_m != arg_type::counted_flag);
    }

    bool has_optional_argument()
    {
        // This parser currently does not support optional arguments (but getopt_long does if necessary later)
        return false;
    }
};

using arg_object = std::shared_ptr<argument>;

struct help_example
{
    // Rule of zero: Do not declare other types of constructors or a destructor
    help_example(std::string const &example_call_text, std::string const &example_description_text);

    std::string const example_call;
    std::string const example_description;
};

class get_opt_parser_impl
{
public:
    std::string               const name_m;
    std::string               const call_string_m;
    bool                            quiet_m;
    bool                            help_requested_m;
    std::string                     help_title_m;
    std::string                     help_additional_info_m;
    std::vector<help_example>       help_examples_m;

    std::vector<arg_object>         arguments_m;

    get_opt_parser_impl(std::string const &name,
                        std::string const &call_string,
                        bool        const  quiet,
                        bool        const  auto_help)
    : name_m(name)
    , call_string_m(call_string)
    , quiet_m(quiet)
    , help_requested_m(false)
    {
        if(auto_help)
        {
            // Add help option automatically
            arguments_m.push_back(std::make_shared<argument>(help_requested_m, 'h', "help", g_default_help_option_text));
        }
    }

    void check_option(arg_option   const short_option,
                      char const * const long_option);
};

//------------------------------------------------------------------------------
// class implementation
//------------------------------------------------------------------------------
help_example::help_example(std::string const &example_call_text, std::string const &example_description_text)
: example_call(example_call_text)
, example_description(example_description_text)
{ }

get_opt_parser::get_opt_parser(std::string const &name,
                               std::string const &call_string,
                               bool        const  quiet,
                               bool        const  auto_help)
: impl_m(std::make_unique<get_opt_parser_impl>(name, call_string, quiet, auto_help))
{ }

get_opt_parser::get_opt_parser(get_opt_parser&& src) noexcept = default;
get_opt_parser& get_opt_parser::operator=(get_opt_parser&& src) noexcept = default;
get_opt_parser::~get_opt_parser() noexcept = default;

void get_opt_parser::add_flag(arg_option  const  option_id,
                              char        const *long_option,
                              bool              &argument_destination,
                              char        const *help_text)
{
    long_option = long_option != nullptr ? long_option : g_empty;
    help_text   = help_text   != nullptr ? help_text   : g_empty;
    impl_m->check_option(option_id, long_option);
    impl_m->arguments_m.push_back(std::make_shared<argument>(argument_destination, option_id, long_option, help_text));
}

void get_opt_parser::add_flag(arg_option  const  option_id,
                              char        const *long_option,
                              size_t            &argument_destination,
                              char        const *help_text)
{
    long_option = long_option != nullptr ? long_option : g_empty;
    help_text   = help_text   != nullptr ? help_text   : g_empty;
    impl_m->check_option(option_id, long_option);
    impl_m->arguments_m.push_back(std::make_shared<argument>(argument_destination, option_id, long_option, help_text));
}

void get_opt_parser::add_argument(arg_option  const  option_id,
                                  char        const *long_option,
                                  uint64_t          &argument_destination,
                                  char        const *help_text)
{
    long_option = long_option != nullptr ? long_option : g_empty;
    help_text   = help_text   != nullptr ? help_text   : g_empty;
    impl_m->check_option(option_id, long_option);
    impl_m->arguments_m.push_back(std::make_shared<argument>(argument_destination, option_id, long_option, help_text));
}

void get_opt_parser::add_argument(arg_option  const  option_id,
                                  char        const *long_option,
                                  int64_t           &argument_destination,
                                  char        const *help_text)
{
    long_option = long_option != nullptr ? long_option : g_empty;
    help_text   = help_text   != nullptr ? help_text   : g_empty;
    impl_m->check_option(option_id, long_option);
    impl_m->arguments_m.push_back(std::make_shared<argument>(argument_destination, option_id, long_option, help_text));
}

void get_opt_parser::add_argument(arg_option  const  option_id,
                                  char        const *long_option,
                                  std::string       &argument_destination,
                                  char        const *help_text)
{
    long_option = long_option != nullptr ? long_option : g_empty;
    help_text   = help_text   != nullptr ? help_text   : g_empty;
    impl_m->check_option(option_id, long_option);
    impl_m->arguments_m.push_back(std::make_shared<argument>(argument_destination, option_id, long_option, help_text));
}

void get_opt_parser::add_argument(arg_option            const          option_id,
                                  char                  const         *long_option,
                                  arg_custom_func                     &converter_function,
                                  char                  const         *help_text,
                                  char                  const * const  custom_options)
{
    long_option = long_option != nullptr ? long_option : g_empty;
    help_text   = help_text   != nullptr ? help_text   : g_empty;
    impl_m->check_option(option_id, long_option);
    impl_m->arguments_m.push_back(std::make_shared<argument>(converter_function, option_id, long_option, help_text, custom_options));
}

void get_opt_parser::set_help_title(std::string const &help_title)
{
    impl_m->help_title_m = help_title;
}

void get_opt_parser::set_help_additional_info(std::string const &help_additional_info)
{
    impl_m->help_additional_info_m = help_additional_info;
}

void get_opt_parser::add_help_example(std::string const &example_call,
                                      std::string const &example_description)
{
    impl_m->help_examples_m.emplace_back(example_call, example_description);
}

std::string get_opt_parser::generate_help_text()
{
    std::string help_text;
    help_text += "Program call: " + impl_m->call_string_m + "\n";
    if(!impl_m->help_title_m.empty())
    {
        help_text += "\n";
        help_text += impl_m->help_title_m + "\n";
    }
    help_text += "\n";
    help_text += "Usage:\n";
    help_text += "  " + impl_m->name_m + " " PARAMETER_USAGE_DESCRIPTION "\n";
    help_text += "\n";
    help_text += "Application options:\n";
    for(auto const &arg : impl_m->arguments_m)
    {
        char buffer[256];
        char const * param = "";
        if(arg->has_required_argument())
        {
            param = WC_ARRAY_TO_PTR(g_required_argument_placeholder);
        }
        else if(arg->has_optional_argument())
        {
            param = WC_ARRAY_TO_PTR(g_optional_argument_placeholder);
        }
        bool const short_option_available = is_short_option_character(arg->short_option_m);
        bool const long_option_available  = !std::string(arg->long_option_m).empty();
        auto const printed = static_cast<size_t>(snprintf(WC_ARRAY_TO_PTR(buffer),
                                                          sizeof(buffer),
                                                          WC_ARRAY_TO_PTR(g_help_option_format),
                                                          long_option_available ? "--" : "  ",
                                                          long_option_available ? arg->long_option_m : "",
                                                          param,
                                                          short_option_available ? '-' : ' ',
                                                          short_option_available ? arg->short_option_m : ' ',
                                                          arg->help_text_m));
        buffer[sizeof(buffer) - 1] = '\0';
        WC_ASSERT(printed < sizeof(buffer));
        help_text += WC_ARRAY_TO_PTR(buffer);
    }
    for(auto const &arg : impl_m->arguments_m)
    {
        if(arg->custom_options_m != nullptr)
        {
            help_text += "\n";
            help_text += "Available values for " + std::string(arg->long_option_m) + " option:\n";
            help_text += "  " + std::string(arg->custom_options_m) + "\n";
        }
    }
    if(!impl_m->help_additional_info_m.empty())
    {
        help_text += "\n";
        help_text += "Additional information:\n";
        help_text += "  " + impl_m->help_additional_info_m + "\n";
    }
    if(!impl_m->help_examples_m.empty())
    {
        help_text += "\n";
        help_text += "Examples:";
    }
    for(auto const &arg : impl_m->help_examples_m)
    {
        help_text += "\n";
        help_text += "  " + arg.example_call + "\n";
        help_text += "    " + arg.example_description + "\n";
    }

    return help_text;
}

bool get_opt_parser::is_help_requested()
{
    return impl_m->help_requested_m;
}

void get_opt_parser::parse(int   argc,
                           char *argv[])
{
    using arg_option_index = int;

    std::string         commandline_options_short;
    std::vector<option> commandline_options;
    for(auto const &arg : impl_m->arguments_m)
    {
        switch(arg->current_type_m)
        {
            case arg_type::flag:
            case arg_type::counted_flag:
                // TODO: getopt_long supports flags natively, check if use of this improves implementation
                if(is_short_option_character(arg->short_option_m))
                {
                    commandline_options_short += static_cast<char>(arg->short_option_m);
                }
                commandline_options.push_back({ arg->long_option_m, no_argument, nullptr, arg->short_option_m });
                break;
            default:
                if(is_short_option_character(arg->short_option_m))
                {
                    commandline_options_short += static_cast<char>(arg->short_option_m);
                    commandline_options_short += ':';
                }
                commandline_options.push_back({ arg->long_option_m, required_argument, nullptr, arg->short_option_m });
        }
        switch(arg->current_type_m)
        {
            case arg_type::flag:
                *(arg->argument_destination_flag_m)         = false;
                break;
            case arg_type::counted_flag:
                *(arg->argument_destination_counted_flag_m) = 0;
                break;
            default:
                break;
        }
    }
    commandline_options.push_back({ nullptr, no_argument, nullptr, 0 }); // End marker, don't remove

    // Configure argument options
    if(impl_m->quiet_m)
    {
        opterr = 0;
    }
    optind = (optind > 1) ? 0 : 1; // 0 re-inits getopt[_long] in case of an additional parsing

    
    if (!is_help_requested())
    {
        // Get argument options
        arg_option       option;
        arg_option_index option_index;
        
        while((option = getopt_long(argc, argv, commandline_options_short.c_str(), commandline_options.data(), &option_index)) != -1) //lint !e960 !e1960 suggested usage of getopt // parasoft-suppress CERT_C-EXP45-d "This is the suggested usage of getopt. The additional parantheses around the assignment should make it clear, that this is done on purpose."
        {
            if(    (option == 0)
                && (commandline_options.size() >= option_index)
                && (commandline_options.at(static_cast<size_t>(option_index)).flag != nullptr))
            {
                // Option sets a flag, nothing else to do
                continue;
            }
            if(option == '?')
            {
                // Wrong option character or option with missing argument stored in global variable "optopt"
                // getopt_long already printed an error message in case of '?' (if opterr != 0)
                //parse_error_m = "Wrong option character or option with missing argument";
                throw std::runtime_error(std::string("Wrong option character or option with missing argument: ") + argv[optind - 1]);
            }
            auto search_result = std::find_if(impl_m->arguments_m.begin(), impl_m->arguments_m.end(), [option](arg_object const obj){
                return obj->short_option_m == option;
            });
            WC_ASSERT(search_result != impl_m->arguments_m.end());
            arg_object const &arg = *search_result;
            bool invalid = false;
            switch(arg->current_type_m)
            {
                case arg_type::flag:
                    *(arg->argument_destination_flag_m) = true;
                    break;
                case arg_type::counted_flag:
                    *(arg->argument_destination_counted_flag_m) += 1;
                    break;
                case arg_type::unsigned_number:
                    try
                    {
                        *(arg->argument_destination_unsigned_number_m) = std::stoull(optarg);
                    }
                    catch(std::invalid_argument &e)
                    {
                        invalid = true;
                    }
                    catch(std::out_of_range &e)
                    {
                        invalid = true;
                    }
                    break;
                case arg_type::signed_number:
                    try
                    {
                        *(arg->argument_destination_signed_number_m) = std::stoll(optarg);
                    }
                    catch(std::invalid_argument &e)
                    {
                        invalid = true;
                    }
                    catch(std::out_of_range &e)
                    {
                        invalid = true;
                    }
                    break;
                case arg_type::plain:
                    *(arg->argument_destination_plain_m) = optarg;
                    break;
                case arg_type::custom:
                    arg->argument_destination_custom_m(option, optarg);
                    break;
            }
            if(invalid)
            {
                throw std::runtime_error(std::string("Invalid argument for option: ")
                                    + std::string(arg->long_option_m)
                                    + (is_short_option_character(arg->short_option_m) ?
                                        std::string(" (") + static_cast<char>(arg->short_option_m) + ")" : "")
                                    + std::string(" with ") + argv[optind - 1]);
            }
        }
    }
}

void get_opt_parser_impl::check_option(arg_option   const short_option,
                                       char const * const long_option)
{
    check_short_option_value(short_option);
    for(int i = 0; long_option[i] != '\0'; i++) // parasoft-suppress CERT_C-ARR30-a CERT_C-ARR39-a CERT_C-FIO37-a CERT_C-STR31-a "long_option is guaranteed to be a null-terminated string."
    {
        if(isspace(long_option[i]) > 0)
        {
            throw std::runtime_error("Tried to configure long option value with whitespace: '"
                                     + std::string(long_option) + "'");
        }
    }
    for(auto const &arg_element: arguments_m)
    {
        if(arg_element->short_option_m == short_option)
        {
            std::string const option_number = "(" + std::to_string(short_option) +")";
            std::string const option_text   = is_short_option_character(short_option)
                                              ? "'" + std::string(1, static_cast<char>(short_option)) + "' " + option_number
                                              : option_number;
            throw std::runtime_error("Tried to configure short option value "
                                     + option_text + " already used");
        }
        if( (long_option[0] != '\0') && (std::string(arg_element->long_option_m) == long_option) )
        {
            throw std::runtime_error("Tried to configure long option value '"
                                     + std::string(long_option) + "' already used");
        }
    }
    if(!is_short_option_character(short_option) && std::string(long_option).empty())
    {
        throw std::runtime_error("Tried to configure an unusable (dead) option (ID " + std::to_string(short_option) + "): "
                                 "Option ID is not in range [a-zA-Z] to use as short option and long option is empty");
    }
}


//------------------------------------------------------------------------------
// function implementation
//------------------------------------------------------------------------------
namespace {

bool is_short_option_character(int value)
{
    return    ((value >= 'a') && (value <= 'z'))
           || ((value >= 'A') && (value <= 'Z'));
}

void check_short_option_value(arg_option const short_option)
{
    // Invalid short option character
    if(short_option < 0)
    {
        throw std::runtime_error("Tried to configure negative short option value " + std::to_string(short_option));
    }
    // The next short option values are special cases or get_opt[_long] so they are not allowed to configure
    if((short_option == 0) || (short_option == '?'))
    {
        throw std::runtime_error("Tried to configure reserved short option value " + std::to_string(short_option));
    }
    // Function isspace returns also true for at least some values > 255, so check only for basic ASCII (0-127)
    if((short_option < 127) && (isspace(short_option) > 0))
    {
        throw std::runtime_error("Tried to configure short option value with whitespace: '"
                                 + std::to_string(short_option) + "'");
    }
}

}

} // Namespace optparsing
} // Namespace wago


//---- End of source file ------------------------------------------------------
