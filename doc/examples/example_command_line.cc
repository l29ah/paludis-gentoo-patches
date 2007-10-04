/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Basic command line handling for most examples.
 */

#include "example_command_line.hh"
#include <paludis/paludis.hh>
#include <cstdlib>
#include <iostream>

using namespace paludis;
using namespace examples;

using std::cout;
using std::endl;

template class InstantiationPolicy<CommandLine, instantiation_method::SingletonTag>;

CommandLine::CommandLine() :
    ArgsHandler(),

    action_args(this, "Actions",
            "Selects which basic action to perform. At most one action should "
            "be specified."),
    a_version(&action_args,   "version",      'V',  "Display program version"),
    a_help(&action_args,      "help",         'h',  "Display program help"),

    general_args(this, "General options",
            "Options which are relevant for most or all actions."),
    a_log_level(&general_args, "log-level",  '\0'),
    a_environment(&general_args, "environment", 'E', "Environment specification (class:suffix, both parts optional)")
{
}

std::string
CommandLine::app_name() const
{
    return "example";
}

std::string
CommandLine::app_synopsis() const
{
    return "An example app";
}

std::string
CommandLine::app_description() const
{
    return "This is an example program.";
}

CommandLine::~CommandLine()
{
}

void
examples::show_help_and_exit(const char * const argv[])
{
    cout << "Usage: " << argv[0] << " [options]" << endl;
    cout << endl;
    cout << *CommandLine::get_instance();

    exit(EXIT_SUCCESS);
}

void
examples::show_version_and_exit()
{
    cout << PALUDIS_PACKAGE << " " << PALUDIS_VERSION_MAJOR << "."
        << PALUDIS_VERSION_MINOR << "." << PALUDIS_VERSION_MICRO;
    if (! std::string(PALUDIS_SUBVERSION_REVISION).empty())
        cout << " svn " << PALUDIS_SUBVERSION_REVISION;
    cout << endl;

    exit(EXIT_SUCCESS);
}

void
CommandLine::run(const int argc, const char * const * const argv, const std::string & client,
        const std::string & env_var, const std::string & env_prefix)
{
    args::ArgsHandler::run(argc, argv, client, env_var, env_prefix);

    if (CommandLine::get_instance()->a_log_level.specified())
        Log::get_instance()->set_log_level(CommandLine::get_instance()->a_log_level.option());

    if (CommandLine::get_instance()->a_help.specified())
        show_help_and_exit(argv);

    if (CommandLine::get_instance()->a_version.specified())
        show_version_and_exit();
}

