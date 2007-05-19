/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
 *
 * This file is part of the Paludis package manager. Paludis is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <paludis/qa/homepage_check.hh>
#include <paludis/package_database_entry.hh>
#include <paludis/environment.hh>
#include <paludis/qa/qa_environment.hh>
#include <paludis/util/visitor-impl.hh>
#include <algorithm>

using namespace paludis;
using namespace paludis::qa;

namespace
{
    struct Checker :
        DepSpecVisitorTypes::ConstVisitor,
        DepSpecVisitorTypes::ConstVisitor::VisitChildren<Checker, AllDepSpec>,
        DepSpecVisitorTypes::ConstVisitor::VisitChildren<Checker, UseDepSpec>
    {
        using DepSpecVisitorTypes::ConstVisitor::VisitChildren<Checker, AllDepSpec>::visit;
        using DepSpecVisitorTypes::ConstVisitor::VisitChildren<Checker, UseDepSpec>::visit;

        CheckResult & result;
        bool found_one;

        Checker(CheckResult & rr) :
            result(rr),
            found_one(false)
        {
        }

        void visit(const PackageDepSpec * const)
        {
            result << Message(qal_major, "Got a PackageDepSpec in HOMEPAGE");
        }

        void visit(const PlainTextDepSpec * const t)
        {
            std::string text(t->text());

            if (std::string::npos == text.find("http://") &&
                    std::string::npos == text.find("https://") &&
                    std::string::npos == text.find("ftp://"))
                result << Message(qal_major, "HOMEPAGE part '" + text + "' doesn't look like a URL");

            found_one = true;
        }

        void visit(const BlockDepSpec * const)
        {
            result << Message(qal_major, "Got a PackageDepSpec in HOMEPAGE");
        }

        void visit(const AnyDepSpec * const a)
        {
            result << Message(qal_major, "Got a || ( ) block in HOMEPAGE");
            std::for_each(a->begin(), a->end(), accept_visitor(this));
        }
    };
}

HomepageCheck::HomepageCheck()
{
}

CheckResult
HomepageCheck::operator() (const EbuildCheckData & e) const
{
    CheckResult result(stringify(e.name) + "-" + stringify(e.version),
            identifier());

    try
    {
        PackageDatabaseEntry ee(e.name, e.version,
                e.environment->main_repository()->name());
        std::tr1::shared_ptr<const VersionMetadata> metadata(
                e.environment->package_database()->fetch_repository(ee.repository)->version_metadata(ee.name, ee.version));

        Checker c(result);
        metadata->homepage()->accept(&c);

        if (! c.found_one)
            result << Message(qal_major, "HOMEPAGE empty or unset");
    }
    catch (const InternalError &)
    {
        throw;
    }
    catch (const Exception & err)
    {
        result << Message(qal_fatal, "Caught Exception '" + err.message() + "' ("
                + err.what() + ")");
    }

    return result;
}

const std::string &
HomepageCheck::identifier()
{
    static const std::string id("homepage");
    return id;
}

