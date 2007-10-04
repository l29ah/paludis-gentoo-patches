/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/** \file
 *
 * Example \ref example_dep_spec.cc "example_dep_spec.cc" .
 *
 * \ingroup g_dep_spec
 */

/** \example example_dep_spec.cc
 *
 * This example demonstrates how to handle dependency specs.
 *
 * See \ref example_dep_label.cc "example_dep_label.cc" for labels.
 * See \ref example_dep_tree.cc "example_dep_tree.cc" for trees.
 */

#include <paludis/paludis.hh>
#include "example_command_line.hh"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <list>
#include <map>

using namespace paludis;
using namespace examples;

using std::cout;
using std::endl;
using std::setw;
using std::left;

int main(int argc, char * argv[])
{
    try
    {
        CommandLine::get_instance()->run(argc, argv,
                "example_action", "EXAMPLE_ACTION_OPTIONS", "EXAMPLE_ACTION_CMDLINE");

        /* We start with an Environment, respecting the user's '--environment' choice. */
        tr1::shared_ptr<Environment> env(EnvironmentMaker::get_instance()->make_from_spec(
                    CommandLine::get_instance()->a_environment.argument()));

        /* For each command line parameter... */
        for (CommandLine::ParametersConstIterator q(CommandLine::get_instance()->begin_parameters()),
                q_end(CommandLine::get_instance()->end_parameters()) ; q != q_end ; ++q)
        {
            /* Create a PackageDepSpec from the parameter. For user-inputted
             * data, pds_pm_permissive or pds_pm_unspecific should be used (only
             * the latter allows wildcards). */
            PackageDepSpec spec(*q, pds_pm_unspecific);

            /* Display information about the PackageDepSpec. */
            cout << "Information about '" << spec << "':" << endl;

            if (spec.package_ptr())
                cout << "    " << left << setw(24) << "Package:" << " " << *spec.package_ptr() << endl;

            if (spec.category_name_part_ptr())
                cout << "    " << left << setw(24) << "Category part:" << " " << *spec.category_name_part_ptr() << endl;

            if (spec.package_name_part_ptr())
                cout << "    " << left << setw(24) << "Package part:" << " " << *spec.package_name_part_ptr() << endl;

            if (spec.version_requirements_ptr() && ! spec.version_requirements_ptr()->empty())
            {
                cout << "    " << left << setw(24) << "Version requirements:" << " ";
                bool need_join(false);
                for (VersionRequirements::ConstIterator r(spec.version_requirements_ptr()->begin()),
                        r_end(spec.version_requirements_ptr()->end()) ; r != r_end ; ++r)
                {
                    if (need_join)
                    {
                        switch (spec.version_requirements_mode())
                        {
                            case vr_and:
                                cout << " and ";
                                break;

                            case vr_or:
                                cout << " or ";
                                break;

                            case last_vr:
                                throw InternalError(PALUDIS_HERE, "Bad version_requirements_mode");
                        }
                    }

                    cout << r->version_operator << r->version_spec;
                    need_join = true;
                }
                cout << endl;
            }

            if (spec.slot_ptr())
                cout << "    " << left << setw(24) << "Slot:" << " " << *spec.slot_ptr() << endl;

            if (spec.repository_ptr())
                cout << "    " << left << setw(24) << "Repository:" << " " << *spec.repository_ptr() << endl;

            if (spec.use_requirements_ptr() && ! spec.use_requirements_ptr()->empty())
            {
                cout << "    " << left << setw(24) << "Use requirements:" << " ";
                bool need_join(false);
                for (UseRequirements::ConstIterator u(spec.use_requirements_ptr()->begin()),
                        u_end(spec.use_requirements_ptr()->end()) ; u != u_end ; ++u)
                {
                    if (need_join)
                        cout << " and ";

                    switch (u->second)
                    {
                        case use_enabled:
                        case use_unspecified:
                            break;

                        case use_disabled:
                            cout << "-";
                            break;

                        case last_use:
                            throw InternalError(PALUDIS_HERE, "Bad use requirements");
                    }
                    cout << u->first;

                    need_join = true;
                }
                cout << endl;
            }

            /* And display packages matching that spec */
            cout << "    " << left << setw(24) << "Matches:" << " ";
            tr1::shared_ptr<const PackageIDSequence> ids(
                    env->package_database()->query(query::Matches(spec), qo_order_by_version));
            bool need_indent(false);
            for (PackageIDSequence::ConstIterator i(ids->begin()), i_end(ids->end()) ;
                    i != i_end ; ++i)
            {
                if (need_indent)
                    cout << "    " << left << setw(24) << "" << " ";
                cout << **i << endl;
                need_indent = true;
            }

            cout << endl;
        }
    }
    catch (const Exception & e)
    {
        /* Paludis exceptions can provide a handy human-readable backtrace and
         * an explanation message. Where possible, these should be displayed. */
        cout << endl;
        cout << "Unhandled exception:" << endl
            << "  * " << e.backtrace("\n  * ")
            << e.message() << " (" << e.what() << ")" << endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception & e)
    {
        cout << endl;
        cout << "Unhandled exception:" << endl
            << "  * " << e.what() << endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        cout << endl;
        cout << "Unhandled exception:" << endl
            << "  * Unknown exception type. Ouch..." << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

