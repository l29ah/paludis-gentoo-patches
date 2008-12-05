/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2008 Ciaran McCreesh
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

#include <paludis/repositories/e/can_skip_phase.hh>
#include <paludis/util/set.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/metadata_key.hh>
#include <set>

using namespace paludis;
using namespace paludis::erepository;

bool
paludis::erepository::can_skip_phase(const std::tr1::shared_ptr<const ERepositoryID> & id,
        const EAPIPhase & phase)
{
    if (! id->defined_phases_key())
        return false;

    std::string skipifno(phase.equal_option("skipifno"));
    if (skipifno.empty())
        return false;

    std::set<std::string> skip_if_no_values;
    tokenise<delim_kind::AnyOfTag, delim_mode::DelimiterTag>(skipifno, ",", "", inserter(skip_if_no_values, skip_if_no_values.begin()));

    for (std::set<std::string>::const_iterator i(skip_if_no_values.begin()), i_end(skip_if_no_values.end()) ;
            i != i_end ; ++i)
        if (id->defined_phases_key()->value()->end() != id->defined_phases_key()->value()->find(*i))
            return false;

    return true;
}

