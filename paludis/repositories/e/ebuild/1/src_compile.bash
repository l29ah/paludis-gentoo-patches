#!/usr/bin/env bash
# vim: set sw=4 sts=4 et :

# Copyright (c) 2006, 2007 Ciaran McCreesh
#
# Based in part upon ebuild.sh from Portage, which is Copyright 1995-2005
# Gentoo Foundation and distributed under the terms of the GNU General
# Public License v2.
#
# This file is part of the Paludis package manager. Paludis is free software;
# you can redistribute it and/or modify it under the terms of the GNU General
# Public License, version 2, as published by the Free Software Foundation.
#
# Paludis is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA  02111-1307  USA

src_compile()
{
    [[ -x ${ECONF_SOURCE:-.}/configure ]] && econf
    if [[ -f Makefile ]] || [[ -f makefile ]] || [[ -f GNUmakefile ]] ; then
        emake || die "emake failed"
    fi
}

ebuild_f_compile()
{
    if [[ -d "${S}" ]] ; then
        cd "${S}" || die "cd to \${S} (\"${S}\") failed"
    elif [[ -d "${WORKDIR}" ]] ; then
        cd "${WORKDIR}" || die "cd to \${WORKDIR} (\"${WORKDIR}\") failed"
    fi

    if hasq "compile" ${SKIP_FUNCTIONS} ; then
        ebuild_section "Skipping src_compile (SKIP_FUNCTIONS)"
    else
        if [[ $(type -t pre_src_compile ) == "function" ]] ; then
            ebuild_section "Starting pre_src_compile"
            pre_src_compile
            ebuild_section "Done pre_src_compile"
        fi

        ebuild_section "Starting src_compile"
        src_compile
        ebuild_section "Done src_compile"

        if [[ $(type -t post_src_compile ) == "function" ]] ; then
            ebuild_section "Starting post_src_compile"
            post_src_compile
            ebuild_section "Done post_src_compile"
        fi
    fi
}


