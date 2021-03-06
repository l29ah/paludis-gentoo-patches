/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2009, 2010, 2011 Ciaran McCreesh
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

#ifndef PALUDIS_GUARD_PALUDIS_RESOLVER_REASON_FWD_HH
#define PALUDIS_GUARD_PALUDIS_RESOLVER_REASON_FWD_HH 1

#include <paludis/util/attributes.hh>
#include <paludis/util/sequence-fwd.hh>
#include <iosfwd>
#include <memory>

namespace paludis
{
    namespace resolver
    {
        class Reason;
        class DependencyReason;
        class TargetReason;
        class PresetReason;
        class SetReason;
        class DependentReason;
        class WasUsedByReason;
        class LikeOtherDestinationTypeReason;
        class ViaBinaryReason;

        typedef Sequence<std::shared_ptr<const Reason> > Reasons;
    }

    std::ostream & operator<< (std::ostream & s, const resolver::Reason & d) PALUDIS_VISIBLE;
}

#endif
