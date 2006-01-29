/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006 Ciaran McCreesh <ciaranm@gentoo.org>
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

#ifndef PALUDIS_GUARD_PALUDIS_ITERATOR_UTILITIES_HH
#define PALUDIS_GUARD_PALUDIS_ITERATOR_UTILITIES_HH 1

namespace paludis
{
    /**
     * Return a new iterator pointing to the item after i.
     */
    template <typename T_>
    T_ next(const T_ & i)
    {
        T_ result(i);
        return ++result;
    }

    /**
     * Return a new iterator pointing to the item before i.
     */
    template <typename T_>
    T_ previous(const T_ & i)
    {
        T_ result(i);
        return --result;
    }
}

#endif
