/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#ifndef PALUDIS_GUARD_PALUDIS_METADATA_KEY_HH
#define PALUDIS_GUARD_PALUDIS_METADATA_KEY_HH 1

#include <paludis/metadata_key-fwd.hh>
#include <paludis/mask-fwd.hh>
#include <paludis/package_id-fwd.hh>
#include <paludis/name-fwd.hh>
#include <paludis/dep_spec-fwd.hh>
#include <paludis/dep_tree.hh>
#include <paludis/contents-fwd.hh>
#include <paludis/repository-fwd.hh>
#include <paludis/formatter-fwd.hh>
#include <paludis/util/fs_entry-fwd.hh>
#include <paludis/util/attributes.hh>
#include <paludis/util/instantiation_policy.hh>
#include <paludis/util/private_implementation_pattern.hh>
#include <paludis/util/visitor.hh>
#include <string>

/** \file
 * Declarations for metadata key classes.
 *
 * \ingroup g_metadata_key
 *
 * \section Examples
 *
 * - \ref example_metadata_key.cc "example_metadata_key.cc" (for metadata keys)
 */

namespace paludis
{
    /**
     * Types for a visitor that can visit a MetadataKey subclass.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    struct MetadataKeyVisitorTypes :
        VisitorTypes<
            MetadataKeyVisitorTypes,
            MetadataKey,
            MetadataPackageIDKey,
            MetadataSetKey<UseFlagNameSet>,
            MetadataSetKey<IUseFlagSet>,
            MetadataSetKey<KeywordNameSet>,
            MetadataSetKey<Set<std::string> >,
            MetadataSetKey<PackageIDSequence>,
            MetadataSpecTreeKey<DependencySpecTree>,
            MetadataSpecTreeKey<LicenseSpecTree>,
            MetadataSpecTreeKey<FetchableURISpecTree>,
            MetadataSpecTreeKey<SimpleURISpecTree>,
            MetadataSpecTreeKey<ProvideSpecTree>,
            MetadataSpecTreeKey<RestrictSpecTree>,
            MetadataStringKey,
            MetadataContentsKey,
            MetadataTimeKey,
            MetadataRepositoryMaskInfoKey,
            MetadataFSEntryKey
            >
    {
    };

    /**
     * A MetadataKey is a generic key that contains a particular piece of
     * information about a PackageID instance.
     *
     * A basic MetadataKey has:
     *
     * - A raw name. This is in a repository-defined format designed to closely
     *   represent the internal name. For example, ebuilds and VDB IDs use
     *   raw names like 'DESCRIPTION' and 'KEYWORDS', whereas CRAN uses names
     *   like 'Title' and 'BundleDescription'. The raw name is unique in a
     *   PackageID.
     *
     * - A human name. This is the name that should be used when outputting
     *   normally for a human to read.
     *
     * - A MetadataKeyType. This is a hint to clients as to whether the key
     *   should be displayed when outputting information about a package ID.
     *
     * Subclasses provide additional information, including the 'value' of the
     * key. A ConstVisitor using MetadataKeyVisitorTypes can be used to get more
     * detail.
     */
    class PALUDIS_VISIBLE MetadataKey :
        private InstantiationPolicy<MetadataKey, instantiation_method::NonCopyableTag>,
        private PrivateImplementationPattern<MetadataKey>,
        public virtual ConstAcceptInterface<MetadataKeyVisitorTypes>
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataKey(const std::string & raw_name, const std::string & human_name, const MetadataKeyType);

        public:
            virtual ~MetadataKey() = 0;

            ///\}

            /**
             * Fetch our raw name.
             */
            virtual const std::string raw_name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch our human name.
             */
            virtual const std::string human_name() const PALUDIS_ATTRIBUTE((warn_unused_result));

            /**
             * Fetch our key type.
             */
            virtual const MetadataKeyType type() const PALUDIS_ATTRIBUTE((warn_unused_result));
    };

    /**
     * A MetadataPackageIDKey is a MetadataKey that has a PackageID as its
     * value.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE MetadataPackageIDKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataPackageIDKey>
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataPackageIDKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const tr1::shared_ptr<const PackageID> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataStringKey is a MetadataKey that has a std::string as its
     * value.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE MetadataStringKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataStringKey>
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataStringKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const std::string value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataFSEntryKey is a MetadataKey that has an FSEntry as its value.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE MetadataFSEntryKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataFSEntryKey>
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataFSEntryKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const FSEntry value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataTimeKey is a MetadataKey that has a time_t as its value.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE MetadataTimeKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataTimeKey>
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataTimeKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const time_t value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataContentsKey is a MetadataKey that holds a Contents heirarchy.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE MetadataContentsKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataContentsKey>
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataContentsKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const tr1::shared_ptr<const Contents> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataRepositoryMaskInfoKey is a MetadataKey that holds
     * RepositoryMaskInfo as its value.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    class PALUDIS_VISIBLE MetadataRepositoryMaskInfoKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataRepositoryMaskInfoKey>
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataRepositoryMaskInfoKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const tr1::shared_ptr<const RepositoryMaskInfo> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataSetKey is a MetadataKey that holds a Set of some kind of item
     * as its value.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    template <typename C_>
    class PALUDIS_VISIBLE MetadataSetKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataSetKey<C_> >
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataSetKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const tr1::shared_ptr<const C_> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a single-line formatted version of our value, using the
             * supplied Formatter to format individual items.
             */
            virtual std::string pretty_print_flat(const Formatter<typename C_::value_type> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataSetKey<IUseFlagSet> is a MetadataKey that holds an IUseFlagSet
     * as its value.
     *
     * This specialisation of MetadataSetKey provides an additional
     * pretty_print_flat_with_comparison method.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    template <>
    class PALUDIS_VISIBLE MetadataSetKey<IUseFlagSet> :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataSetKey<IUseFlagSet> >
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataSetKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const tr1::shared_ptr<const IUseFlagSet> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a single-line formatted version of our value, using the
             * supplied Formatter to format individual items.
             */
            virtual std::string pretty_print_flat(const Formatter<IUseFlag> &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a single-line formatted version of our value, using the
             * supplied Formatter to format individual items, and the supplied
             * PackageID to decorate using format::Added and format::Changed as
             * appropriate.
             */
            virtual std::string pretty_print_flat_with_comparison(
                    const Environment * const,
                    const tr1::shared_ptr<const PackageID> &,
                    const Formatter<IUseFlag> &
                    ) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataSpecTreeKey<> is a MetadataKey that holds a spec tree of some
     * kind as its value.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    template <typename C_>
    class PALUDIS_VISIBLE MetadataSpecTreeKey :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataSpecTreeKey<C_> >
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataSpecTreeKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const tr1::shared_ptr<const typename C_::ConstItem> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a multiline-line indented and formatted version of our
             * value, using the supplied Formatter to format individual items.
             */
            virtual std::string pretty_print(const typename C_::ItemFormatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a single-line formatted version of our value, using the
             * supplied Formatter to format individual items.
             */
            virtual std::string pretty_print_flat(const typename C_::ItemFormatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };

    /**
     * A MetadataSpecTreeKey<FetchableURISpecTree> is a MetadataKey that holds
     * a FetchableURISpecTree as its value.
     *
     * This specialisation of MetadataSpecTreeKey provides an additional
     * initial_label method.
     *
     * \ingroup g_metadata_key
     * \since 0.26
     * \nosubgrouping
     */
    template <>
    class PALUDIS_VISIBLE MetadataSpecTreeKey<FetchableURISpecTree> :
        public MetadataKey,
        public ConstAcceptInterfaceVisitsThis<MetadataKeyVisitorTypes, MetadataSpecTreeKey<FetchableURISpecTree> >
    {
        protected:
            ///\name Basic operations
            ///\{

            MetadataSpecTreeKey(const std::string &, const std::string &, const MetadataKeyType);

            ///\}

        public:
            /**
             * Fetch our value.
             */
            virtual const tr1::shared_ptr<const FetchableURISpecTree::ConstItem> value() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a multiline-line indented and formatted version of our
             * value, using the supplied Formatter to format individual items.
             */
            virtual std::string pretty_print(const FetchableURISpecTree::ItemFormatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a single-line formatted version of our value, using the
             * supplied Formatter to format individual items.
             */
            virtual std::string pretty_print_flat(const FetchableURISpecTree::ItemFormatter &) const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;

            /**
             * Return a URILabel that represents the initial label to use when
             * deciding the behaviour of individual items in the heirarchy.
             */
            virtual const tr1::shared_ptr<const URILabel> initial_label() const
                PALUDIS_ATTRIBUTE((warn_unused_result)) = 0;
    };
}

#endif
