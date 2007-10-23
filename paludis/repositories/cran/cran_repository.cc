/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Danny van Dyk <kugelfang@gentoo.org>
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

#include <paludis/hashed_containers.hh>
#include <paludis/util/config_file.hh>
#include <paludis/dep_spec.hh>
#include <paludis/environment.hh>
#include <paludis/action.hh>
#include <paludis/metadata_key.hh>
#include <paludis/repositories/cran/cran_package_id.hh>
#include <paludis/repositories/cran/cran_repository.hh>
#include <paludis/repository_maker.hh>
#include <paludis/repository_info.hh>
#include <paludis/util/dir_iterator.hh>
#include <paludis/util/set.hh>
#include <paludis/util/sequence.hh>
#include <paludis/util/options.hh>
#include <paludis/util/fs_entry.hh>
#include <paludis/util/iterator.hh>
#include <paludis/util/join.hh>
#include <paludis/util/log.hh>
#include <paludis/util/map.hh>
#include <paludis/util/make_shared_ptr.hh>
#include <paludis/util/mutex.hh>
#include <paludis/util/private_implementation_pattern-impl.hh>
#include <paludis/util/stringify.hh>
#include <paludis/util/strip.hh>
#include <paludis/util/system.hh>
#include <paludis/util/tokeniser.hh>
#include <paludis/util/tr1_functional.hh>
#include <paludis/util/is_file_with_extension.hh>
#include <paludis/util/visitor-impl.hh>

#include <libwrapiter/libwrapiter_forward_iterator.hh>
#include <libwrapiter/libwrapiter_output_iterator.hh>

#include <functional>
#include <algorithm>

/** \file
 * Implementation CRANRepository.
 *
 * \ingroup grpcranrepository
 */

using namespace paludis;

#include <paludis/repositories/cran/cran_repository-sr.cc>

typedef MakeHashedMap<QualifiedPackageName, tr1::shared_ptr<const cranrepository::CRANPackageID> >::Type IDMap;

namespace paludis
{
    /**
     * Implementation data for a CRANRepository.
     *
     * \ingroup grperepository
     */
    template <>
    struct Implementation<CRANRepository>
    {
        CRANRepositoryParams params;

        mutable tr1::shared_ptr<Mutex> big_nasty_mutex;
        mutable bool has_ids;
        mutable IDMap ids;

        Implementation(const CRANRepositoryParams &, const tr1::shared_ptr<Mutex> &);
        ~Implementation();
    };
}

Implementation<CRANRepository>::Implementation(const CRANRepositoryParams & p, const tr1::shared_ptr<Mutex> & m) :
    params(p),
    big_nasty_mutex(m),
    has_ids(false)
{
}

Implementation<CRANRepository>::~Implementation()
{
}


CRANRepository::CRANRepository(const CRANRepositoryParams & p) :
    Repository(CRANRepository::fetch_repo_name(stringify(p.location)),
            RepositoryCapabilities::create()
            .installed_interface(0)
            .sets_interface(this)
            .syncable_interface(this)
            .use_interface(0)
            .world_interface(0)
            .environment_variable_interface(0)
            .make_virtuals_interface(0)
            .mirrors_interface(0)
            .provides_interface(0)
            .destination_interface(0)
            .virtuals_interface(0)
            .licenses_interface(0)
            .e_interface(0)
            .qa_interface(0)
            .hook_interface(0)
            .manifest_interface(0),
            "cran"),
    PrivateImplementationPattern<CRANRepository>(new Implementation<CRANRepository>(p, make_shared_ptr(new Mutex)))
{
    tr1::shared_ptr<RepositoryInfoSection> config_info(new RepositoryInfoSection("Configuration information"));

    config_info->add_kv("location", stringify(_imp->params.location));
    config_info->add_kv("distdir", stringify(_imp->params.distdir));
    config_info->add_kv("format", "cran");
    config_info->add_kv("builddir", stringify(_imp->params.builddir));
    config_info->add_kv("library", stringify(_imp->params.library));
    config_info->add_kv("sync", _imp->params.sync);

    _info->add_section(config_info);
}

CRANRepository::~CRANRepository()
{
}

bool
CRANRepository::do_has_category_named(const CategoryNamePart & c) const
{
    return "cran" == stringify(c);
}

bool
CRANRepository::do_has_package_named(const QualifiedPackageName & q) const
{
    Context context("When checking for package '" + stringify(q) + "' in " + stringify(name()) + ":");
    Lock l(*_imp->big_nasty_mutex);

    if (! do_has_category_named(q.category))
        return false;

    need_ids();
    return _imp->ids.end() != _imp->ids.find(q);
}

tr1::shared_ptr<const CategoryNamePartSet>
CRANRepository::do_category_names() const
{
    Context context("When fetching category names in " + stringify(name()) + ":");
    Lock l(*_imp->big_nasty_mutex);

    tr1::shared_ptr<CategoryNamePartSet> result(new CategoryNamePartSet);
    result->insert(CategoryNamePart("cran"));

    return result;
}

tr1::shared_ptr<const QualifiedPackageNameSet>
CRANRepository::do_package_names(const CategoryNamePart & c) const
{
    Context context("When fetching package names in category '" + stringify(c)
            + "' in " + stringify(name()) + ":");
    Lock l(*_imp->big_nasty_mutex);

    tr1::shared_ptr<QualifiedPackageNameSet> result(new QualifiedPackageNameSet);
    if (! do_has_category_named(c))
        return result;

    need_ids();

    std::copy(_imp->ids.begin(), _imp->ids.end(), transform_inserter(result->inserter(),
                tr1::mem_fn(&std::pair<const QualifiedPackageName, tr1::shared_ptr<const cranrepository::CRANPackageID> >::first)));

    return result;
}

tr1::shared_ptr<const PackageIDSequence>
CRANRepository::do_package_ids(const QualifiedPackageName & n) const
{
    Context context("When fetching versions of '" + stringify(n) + "' in "
            + stringify(name()) + ":");
    Lock l(*_imp->big_nasty_mutex);

    tr1::shared_ptr<PackageIDSequence> result(new PackageIDSequence);
    if (! do_has_package_named(n))
        return result;

    need_ids();

    IDMap::const_iterator i(_imp->ids.find(n));
    if (i != _imp->ids.end())
        result->push_back(i->second);
    return result;
}

void
CRANRepository::need_ids() const
{
    Lock l(*_imp->big_nasty_mutex);

    if (_imp->has_ids)
        return;

    Context context("When loading IDs for " + stringify(name()) + ":");

    for (DirIterator d(_imp->params.location), d_end ; d != d_end ; ++d)
        if (is_file_with_extension(*d, ".DESCRIPTION", IsFileWithOptions()))
        {
            tr1::shared_ptr<cranrepository::CRANPackageID> id(new cranrepository::CRANPackageID(_imp->params.environment,
                        shared_from_this(), *d));
            if (! _imp->ids.insert(std::make_pair(id->name(), id)).second)
                Log::get_instance()->message(ll_warning, lc_context) << "Couldn't insert package '" << *id << "' due to name collision";

            if (id->contains_key())
                for (PackageIDSequence::ConstIterator i(id->contains_key()->value()->begin()),
                        i_end(id->contains_key()->value()->end()) ; i != i_end ; ++i)
                    if (! _imp->ids.insert(std::make_pair((*i)->name(),
                                    tr1::static_pointer_cast<const cranrepository::CRANPackageID>(*i))).second)
                        Log::get_instance()->message(ll_warning, lc_context) << "Couldn't insert package '" << **i
                            << "', which is contained in '" << *id << "', due to name collision";
        }

    _imp->has_ids = true;
}

RepositoryName
CRANRepository::fetch_repo_name(const std::string & location)
{
    std::string modified_location(FSEntry(location).basename());
    std::replace(modified_location.begin(), modified_location.end(), '/', '-');
    if (modified_location == "cran")
        return RepositoryName("cran");
    else
        return RepositoryName("cran-" + modified_location);
}

#if 0
void
CRANRepository::do_install(const tr1::shared_ptr<const PackageID> & id_uncasted, const InstallOptions & o) const
{
    if (id_uncasted->repository().get() != this)
        throw PackageInstallActionError("Couldn't install '" + stringify(*id_uncasted) + "' using repository '" +
                stringify(name()) + "'");

    const tr1::shared_ptr<const CRANPackageID> id(tr1::static_pointer_cast<const CRANPackageID>(id_uncasted));
    if (id->bundle_member_key())
        return;

    tr1::shared_ptr<const FSEntrySequence> bashrc_files(_imp->params.environment->bashrc_files());

    Command cmd(Command(LIBEXECDIR "/paludis/cran.bash fetch")
            .with_setenv("CATEGORY", "cran")
            .with_setenv("DISTDIR", stringify(_imp->params.distdir))
            .with_setenv("DISTFILE", id->native_package() + "_" + id->native_version() + ".tar.gz")
            .with_setenv("PN", id->native_package())
            .with_setenv("PV", id->native_version())
            .with_setenv("PALUDIS_CRAN_MIRRORS", _imp->params.mirror)
            .with_setenv("PALUDIS_EBUILD_DIR", std::string(LIBEXECDIR "/paludis/"))
            .with_setenv("PALUDIS_EBUILD_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
            .with_setenv("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " ")));

    if (0 != run_command(cmd))
        throw PackageInstallActionError("Couldn't fetch sources for '" + stringify(*id) + "'");

    if (o.fetch_only)
        return;

    FSEntry image(_imp->params.buildroot / stringify(id->native_package()) / "image");
    FSEntry workdir(_imp->params.buildroot / stringify(id->native_package()) / "work");

    if (! o.destination)
        throw PackageInstallActionError("Can't merge '" + stringify(*id) + "' because no destination was provided.");

    cmd = Command(LIBEXECDIR "/paludis/cran.bash clean install")
        .with_sandbox()
        .with_setenv("CATEGORY", "cran")
        .with_setenv("DISTDIR", stringify(_imp->params.distdir))
        .with_setenv("DISTFILE", id->native_package() + "_" + id->native_version() + ".tar.gz")
        .with_setenv("IMAGE", stringify(image))
        .with_setenv("IS_BUNDLE", (id->bundle_key() ? "yes" : ""))
        .with_setenv("LOCATION", stringify(_imp->params.location))
        .with_setenv("PN", id->native_package())
        .with_setenv("PV", id->native_version())
        .with_setenv("PALUDIS_CRAN_LIBRARY", stringify(_imp->params.library))
        .with_setenv("PALUDIS_EBUILD_DIR", std::string(LIBEXECDIR "/paludis/"))
        .with_setenv("PALUDIS_EBUILD_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
        .with_setenv("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " "))
        .with_setenv("ROOT", stringify(o.destination->installed_interface->root()))
        .with_setenv("WORKDIR", stringify(workdir));

    if (0 != run_command(cmd))
        throw PackageInstallActionError("Couldn't install '" + stringify(*id) + "' to '" +
                stringify(image) + "'");

    MergeOptions m(id, image, FSEntry("/dev/null"));

    if (! o.destination->destination_interface)
        throw PackageInstallActionError("Couldn't install '" + stringify(*id) + "' to '" +
                stringify(o.destination->name()) + "' because it does not provide destination_interface");

    if (! o.destination->installed_interface)
        throw PackageInstallActionError("Couldn't install '" + stringify(*id) + "' to '" +
                stringify(o.destination->name()) + "' because it does not provide installed_interface");

    o.destination->destination_interface->merge(m);

    cmd = Command(LIBEXECDIR "/paludis/cran.bash clean")
        .with_setenv("IMAGE", stringify(image))
        .with_setenv("PN", id->native_package())
        .with_setenv("PV", id->native_version())
        .with_setenv("PALUDIS_CRAN_LIBRARY", stringify(_imp->params.library))
        .with_setenv("PALUDIS_EBUILD_DIR", std::string(LIBEXECDIR "/paludis/"))
        .with_setenv("PALUDIS_EBUILD_LOG_LEVEL", stringify(Log::get_instance()->log_level()))
        .with_setenv("PALUDIS_BASHRC_FILES", join(bashrc_files->begin(), bashrc_files->end(), " "))
        .with_setenv("ROOT", stringify(o.destination->installed_interface->root()))
        .with_setenv("WORKDIR", stringify(workdir))
        .with_setenv("REPOSITORY", stringify(name()));

    if (0 != run_command(cmd))
        throw PackageInstallActionError("Couldn't clean '" + stringify(*id) + "'");

    return;
}
#endif

tr1::shared_ptr<SetSpecTree::ConstItem>
CRANRepository::do_package_set(const SetName & s) const
{
    if ("base" == s.data())
    {
        /**
         * \todo Implement system as all package which are installed
         * by dev-lang/R by default.
         */
        return tr1::shared_ptr<SetSpecTree::ConstItem>(new ConstTreeSequence<SetSpecTree, AllDepSpec>(
                    tr1::shared_ptr<AllDepSpec>(new AllDepSpec)));
    }
    else
        return tr1::shared_ptr<SetSpecTree::ConstItem>();
}

tr1::shared_ptr<const SetNameSet>
CRANRepository::sets_list() const
{
    Context context("While generating the list of sets:");

    tr1::shared_ptr<SetNameSet> result(new SetNameSet);
    result->insert(SetName("base"));
    return result;
}

bool
CRANRepository::do_sync() const
{
    Context context("When syncing repository '" + stringify(name()) + "':");
    Lock l(*_imp->big_nasty_mutex);

    std::string cmd("rsync --delete --recursive --progress --exclude \"*.html\" --exclude \"*.INDEX\" '" +
                    _imp->params.sync + "/src/contrib/Descriptions/' ./");

    if (0 != run_command(Command(cmd).with_chdir(_imp->params.location)))
        return false;

    cmd = "rsync --progress '" + _imp->params.sync + "/src/contrib/PACKAGES' ./";

    if (0 != run_command(Command(cmd).with_chdir(_imp->params.location)))
        return false;

    cmd = "rsync --progress '" + _imp->params.sync + "/CRAN_mirrors.csv' ./";

    return 0 == run_command(Command(cmd).with_chdir(_imp->params.location));
}

tr1::shared_ptr<Repository>
CRANRepository::make_cran_repository(
        Environment * const env,
        tr1::shared_ptr<const Map<std::string, std::string> > m)
{
    Context context("When making CRAN repository from repo_file '" +
            (m->end() == m->find("repo_file") ? std::string("?") : m->find("repo_file")->second) + "':");

    std::string location;
    if (m->end() == m->find("location") || ((location = m->find("location")->second)).empty())
        throw CRANRepositoryConfigurationError("Key 'location' not specified or empty");

    std::string library;
    if (m->end() == m->find("library") || ((library = m->find("library")->second)).empty())
        throw CRANRepositoryConfigurationError("Key 'library' not specified or empty");

    std::string distdir;
    if (m->end() == m->find("distdir") || ((distdir = m->find("distdir")->second)).empty())
        distdir = location + "/distfiles";

    std::string mirror;
    if (m->end() == m->find("mirror") || ((mirror = m->find("mirror")->second)).empty())
        mirror = "http://cran.r-project.org/";

    std::string sync;
    if (m->end() == m->find("sync") || ((sync = m->find("sync")->second)).empty())
        sync = "rsync://cran.r-project.org/CRAN";

    std::string builddir;
    if (m->end() == m->find("builddir") || ((builddir = m->find("builddir")->second)).empty())
    {
        if (m->end() == m->find("buildroot") || ((builddir = m->find("buildroot")->second)).empty())
            builddir = "/var/tmp/paludis";
        else
            Log::get_instance()->message(ll_warning, lc_context) << "Key 'buildroot' is deprecated, use 'builddir' instead";
    }

    return tr1::shared_ptr<Repository>(new CRANRepository(CRANRepositoryParams::create()
                .environment(env)
                .location(location)
                .distdir(distdir)
                .sync(sync)
                .builddir(builddir)
                .library(library)
                .mirror(mirror)));
}

CRANRepositoryConfigurationError::CRANRepositoryConfigurationError(
        const std::string & msg) throw () :
    ConfigurationError("CRAN repository configuration error: " + msg)
{
}


void
CRANRepository::invalidate()
{
    _imp.reset(new Implementation<CRANRepository>(_imp->params, _imp->big_nasty_mutex));
}

void
CRANRepository::invalidate_masks()
{
}

namespace
{
    struct SupportsActionQuery :
        ConstVisitor<SupportsActionTestVisitorTypes>
    {
        bool result;

        SupportsActionQuery() :
            result(false)
        {
        }

        void visit(const SupportsActionTest<InstalledAction> &)
        {
        }

        void visit(const SupportsActionTest<InstallAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<ConfigAction> &)
        {
        }

        void visit(const SupportsActionTest<PretendAction> &)
        {
        }

        void visit(const SupportsActionTest<FetchAction> &)
        {
            result = true;
        }

        void visit(const SupportsActionTest<UninstallAction> &)
        {
        }

        void visit(const SupportsActionTest<InfoAction> &)
        {
        }
    };
}

bool
CRANRepository::do_some_ids_might_support_action(const SupportsActionTestBase & a) const
{
    SupportsActionQuery q;
    a.accept(q);
    return q.result;
}

