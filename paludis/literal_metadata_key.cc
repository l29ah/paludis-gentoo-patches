/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007, 2008 Ciaran McCreesh
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

#include <paludis/literal_metadata_key.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/wrapped_forward_iterator.hh>
#include <paludis/util/join.hh>
#include <paludis/formatter.hh>
#include <paludis/package_id.hh>
#include <paludis/action.hh>
#include <tr1/functional>

using namespace paludis;

namespace paludis
{
    template <>
    struct Implementation<LiteralMetadataFSEntrySequenceKey>
    {
        const std::tr1::shared_ptr<const FSEntrySequence> value;

        Implementation(const std::tr1::shared_ptr<const FSEntrySequence> & v) :
            value(v)
        {
        }
    };

    template <>
    struct Implementation<LiteralMetadataStringSetKey>
    {
        const std::tr1::shared_ptr<const Set<std::string> > value;

        Implementation(const std::tr1::shared_ptr<const Set<std::string> > & v) :
            value(v)
        {
        }
    };

    template <>
    struct Implementation<LiteralMetadataStringSequenceKey>
    {
        const std::tr1::shared_ptr<const Sequence<std::string> > value;

        Implementation(const std::tr1::shared_ptr<const Sequence<std::string> > & v) :
            value(v)
        {
        }
    };

#ifndef PALUDIS_NO_DOUBLE_TEMPLATE
    template <>
#endif
    template <typename T_>
    struct Implementation<LiteralMetadataValueKey<T_> >
    {
        const T_ value;

        Implementation(const T_ & v) :
            value(v)
        {
        }
    };
}

LiteralMetadataFSEntrySequenceKey::LiteralMetadataFSEntrySequenceKey(const std::string & h, const std::string & r,
        const MetadataKeyType t, const std::tr1::shared_ptr<const FSEntrySequence> & v) :
    MetadataCollectionKey<FSEntrySequence>(h, r, t),
    PrivateImplementationPattern<LiteralMetadataFSEntrySequenceKey>(new Implementation<LiteralMetadataFSEntrySequenceKey>(v)),
    _imp(PrivateImplementationPattern<LiteralMetadataFSEntrySequenceKey>::_imp)
{
}

LiteralMetadataFSEntrySequenceKey::~LiteralMetadataFSEntrySequenceKey()
{
}

const std::tr1::shared_ptr<const FSEntrySequence>
LiteralMetadataFSEntrySequenceKey::value() const
{
    return _imp->value;
}

namespace
{
    std::string format_fsentry(const FSEntry & i, const Formatter<FSEntry> & f)
    {
        return f.format(i, format::Plain());
    }
}

std::string
LiteralMetadataFSEntrySequenceKey::pretty_print_flat(const Formatter<FSEntry> & f) const
{
    using namespace std::tr1::placeholders;
    return join(value()->begin(), value()->end(), " ", std::tr1::bind(&format_fsentry, _1, f));
}

LiteralMetadataStringSetKey::LiteralMetadataStringSetKey(const std::string & h, const std::string & r,
        const MetadataKeyType t, const std::tr1::shared_ptr<const Set<std::string> > & v) :
    MetadataCollectionKey<Set<std::string> >(h, r, t),
    PrivateImplementationPattern<LiteralMetadataStringSetKey>(new Implementation<LiteralMetadataStringSetKey>(v)),
    _imp(PrivateImplementationPattern<LiteralMetadataStringSetKey>::_imp)
{
}

LiteralMetadataStringSetKey::~LiteralMetadataStringSetKey()
{
}

const std::tr1::shared_ptr<const Set<std::string> >
LiteralMetadataStringSetKey::value() const
{
    return _imp->value;
}

LiteralMetadataStringSequenceKey::LiteralMetadataStringSequenceKey(const std::string & h, const std::string & r,
        const MetadataKeyType t, const std::tr1::shared_ptr<const Sequence<std::string> > & v) :
    MetadataCollectionKey<Sequence<std::string> >(h, r, t),
    PrivateImplementationPattern<LiteralMetadataStringSequenceKey>(new Implementation<LiteralMetadataStringSequenceKey>(v)),
    _imp(PrivateImplementationPattern<LiteralMetadataStringSequenceKey>::_imp)
{
}

LiteralMetadataStringSequenceKey::~LiteralMetadataStringSequenceKey()
{
}

const std::tr1::shared_ptr<const Sequence<std::string> >
LiteralMetadataStringSequenceKey::value() const
{
    return _imp->value;
}

namespace
{
    std::string format_string(const std::string & i, const Formatter<std::string> & f)
    {
        return f.format(i, format::Plain());
    }
}

std::string
LiteralMetadataStringSetKey::pretty_print_flat(const Formatter<std::string> & f) const
{
    using namespace std::tr1::placeholders;
    return join(value()->begin(), value()->end(), " ", std::tr1::bind(&format_string, _1, f));
}

std::string
LiteralMetadataStringSequenceKey::pretty_print_flat(const Formatter<std::string> & f) const
{
    using namespace std::tr1::placeholders;
    return join(value()->begin(), value()->end(), " ", std::tr1::bind(&format_string, _1, f));
}

ExtraLiteralMetadataValueKeyMethods<long>::~ExtraLiteralMetadataValueKeyMethods()
{
}

std::string
ExtraLiteralMetadataValueKeyMethods<long>::pretty_print() const
{
    long v(static_cast<const LiteralMetadataValueKey<long> *>(this)->value());
    return stringify(v);
}

ExtraLiteralMetadataValueKeyMethods<std::tr1::shared_ptr<const PackageID> >::~ExtraLiteralMetadataValueKeyMethods()
{
}

std::string
ExtraLiteralMetadataValueKeyMethods<std::tr1::shared_ptr<const PackageID> >::pretty_print(const Formatter<PackageID> & f) const
{
    std::tr1::shared_ptr<const PackageID> v(static_cast<const LiteralMetadataValueKey<std::tr1::shared_ptr<const PackageID> > *>(this)->value());
    if (v->supports_action(SupportsActionTest<InstalledAction>()))
        return f.format(*v, format::Installed());
    else if (v->supports_action(SupportsActionTest<InstallAction>()))
    {
        if (v->masked())
            return f.format(*v, format::Plain());
        else
            return f.format(*v, format::Installable());
    }
    else
        return f.format(*v, format::Plain());
}

template <typename T_>
LiteralMetadataValueKey<T_>::LiteralMetadataValueKey(const std::string & r, const std::string & h,
        const MetadataKeyType t, const T_ & v) :
    MetadataValueKey<T_>(r, h, t),
    PrivateImplementationPattern<LiteralMetadataValueKey<T_> >(new Implementation<LiteralMetadataValueKey<T_> >(v)),
    _imp(PrivateImplementationPattern<LiteralMetadataValueKey<T_ > >::_imp)
{
}

template <typename T_>
LiteralMetadataValueKey<T_>::~LiteralMetadataValueKey()
{
}

template <typename T_>
const T_
LiteralMetadataValueKey<T_>::value() const
{
    return _imp->value;
}

template class LiteralMetadataValueKey<FSEntry>;
template class LiteralMetadataValueKey<std::string>;
template class LiteralMetadataValueKey<bool>;
template class LiteralMetadataValueKey<long>;
template class LiteralMetadataValueKey<std::tr1::shared_ptr<const PackageID> >;

