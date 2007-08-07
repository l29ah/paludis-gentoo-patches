/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaranm@ciaranm.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_QA_HH
#define PALUDIS_GUARD_PALUDIS_QA_HH 1

#include <paludis/qa-fwd.hh>
#include <paludis/util/fs_entry-fwd.hh>

namespace paludis
{
    class PALUDIS_VISIBLE QAReporter
    {
        public:
            virtual ~QAReporter() = 0;

            virtual void message(
                    const FSEntry & what,
                    const QAMessageLevel,
                    const std::string & name,
                    const std::string & message) = 0;
    };
}

#endif
