/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2006, 2007 Ciaran McCreesh <ciaran.mccreesh@blueyonder.co.uk>
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

#include <paludis/repositories/e/vdb_repository.hh>
#include <paludis/environments/test/test_environment.hh>
#include <paludis/package_database.hh>
#include <paludis/util/sequence.hh>
#include <paludis/query.hh>
#include <test/test_framework.hh>
#include <test/test_runner.hh>
#include <fstream>
#include <iterator>

#include <libwrapiter/libwrapiter_forward_iterator.hh>

using namespace test;
using namespace paludis;

/** \file
 * Test cases for VDBRepository.
 *
 */

namespace test_cases
{
    /**
     * \test Test VDBRepository repo names
     *
     */
    struct VDBRepositoryRepoNameTest : TestCase
    {
        VDBRepositoryRepoNameTest() : TestCase("repo name") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            tr1::shared_ptr<Repository> repo(VDBRepository::make_vdb_repository(&env, keys));
            TEST_CHECK_STRINGIFY_EQUAL(repo->name(), "installed");
        }
    } test_vdb_repository_repo_name;

    /**
     * \test Test VDBRepository has_category_named
     *
     */
    struct VDBRepositoryHasCategoryNamedTest : TestCase
    {
        VDBRepositoryHasCategoryNamedTest() : TestCase("has category named") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            tr1::shared_ptr<Repository> repo(VDBRepository::make_vdb_repository(
                        &env, keys));

            TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-one")));
            TEST_CHECK(repo->has_category_named(CategoryNamePart("cat-two")));
            TEST_CHECK(! repo->has_category_named(CategoryNamePart("cat-three")));
        }
    } test_vdb_repository_has_category_named;

    /**
     * \test Test VDBRepository query_use
     *
     */
    struct VDBRepositoryQueryUseTest : TestCase
    {
        VDBRepositoryQueryUseTest() : TestCase("query USE") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            tr1::shared_ptr<Repository> repo(VDBRepository::make_vdb_repository(&env, keys));
            env.package_database()->add_repository(1, repo);

            tr1::shared_ptr<const PackageID> e1(*env.package_database()->query(query::Matches(
                            PackageDepSpec("=cat-one/pkg-one-1", pds_pm_permissive)), qo_require_exactly_one)->begin());

            TEST_CHECK(repo->use_interface->query_use(UseFlagName("flag1"), *e1) == use_enabled);
            TEST_CHECK(repo->use_interface->query_use(UseFlagName("flag2"), *e1) == use_enabled);
            TEST_CHECK(repo->use_interface->query_use(UseFlagName("flag3"), *e1) == use_disabled);
        }
    } test_vdb_repository_query_use;

    /**
     * \test Test VDBRepository add_to_world.
     */
    struct VDBRepositoryAddToWorldNewFileTest : TestCase
    {
        VDBRepositoryAddToWorldNewFileTest() : TestCase("add to world (new file)") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            keys->insert("world", "vdb_repository_TEST_dir/world-new-file");
            tr1::shared_ptr<Repository> repo(VDBRepository::make_vdb_repository(
                        &env, keys));
            repo->world_interface->add_to_world(QualifiedPackageName("cat-one/foofoo"));
            std::ifstream world("vdb_repository_TEST_dir/world-new-file");
            std::string world_content((std::istreambuf_iterator<char>(world)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(world_content, "cat-one/foofoo\n");
        }
    } test_vdb_repository_add_to_world_new_file;

    /**
     * \test Test VDBRepository add_to_world.
     */
    struct VDBRepositoryAddToWorldEmptyFileTest : TestCase
    {
        VDBRepositoryAddToWorldEmptyFileTest() : TestCase("add to world (empty file)") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            keys->insert("world", "vdb_repository_TEST_dir/world-empty");
            tr1::shared_ptr<Repository> repo(VDBRepository::make_vdb_repository(
                        &env, keys));
            repo->world_interface->add_to_world(QualifiedPackageName("cat-one/foofoo"));
            std::ifstream world("vdb_repository_TEST_dir/world-empty");
            std::string world_content((std::istreambuf_iterator<char>(world)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(world_content, "cat-one/foofoo\n");
        }
    } test_vdb_repository_add_to_world_empty_file;

    /**
     * \test Test VDBRepository add_to_world.
     */
    struct VDBRepositoryAddToWorldNoMatchTest : TestCase
    {
        VDBRepositoryAddToWorldNoMatchTest() : TestCase("add to world (no match)") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            keys->insert("world", "vdb_repository_TEST_dir/world-no-match");
            tr1::shared_ptr<Repository> repo(VDBRepository::make_vdb_repository(
                        &env, keys));
            repo->world_interface->add_to_world(QualifiedPackageName("cat-one/foofoo"));
            std::ifstream world("vdb_repository_TEST_dir/world-no-match");
            std::string world_content((std::istreambuf_iterator<char>(world)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(world_content, "cat-one/foo\ncat-one/bar\ncat-one/oink\ncat-one/foofoo\n");
        }
    } test_vdb_repository_add_to_world_no_match;

    /**
     * \test Test VDBRepository add_to_world.
     */
    struct VDBRepositoryAddToWorldMatchTest : TestCase
    {
        VDBRepositoryAddToWorldMatchTest() : TestCase("add to world (match)") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(
                    new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            keys->insert("world", "vdb_repository_TEST_dir/world-match");
            tr1::shared_ptr<Repository> repo(VDBRepository::make_vdb_repository(
                        &env, keys));
            repo->world_interface->add_to_world(QualifiedPackageName("cat-one/foofoo"));
            std::ifstream world("vdb_repository_TEST_dir/world-match");
            std::string world_content((std::istreambuf_iterator<char>(world)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(world_content, "cat-one/foo\ncat-one/foofoo\ncat-one/bar\n");
        }
    } test_vdb_repository_add_to_world_match;

    /**
     * \test Test VDBRepository add_to_world.
     */
    struct VDBRepositoryAddToWorldNoMatchNoEOLTest : TestCase
    {
        VDBRepositoryAddToWorldNoMatchNoEOLTest() : TestCase("add to world (no match, no trailing eol)") { }

        void run()
        {
            TestEnvironment env;
            tr1::shared_ptr<Map<std::string, std::string> > keys(new Map<std::string, std::string>);
            keys->insert("format", "vdb");
            keys->insert("names_cache", "/var/empty");
            keys->insert("provides_cache", "/var/empty");
            keys->insert("location", "vdb_repository_TEST_dir/repo1");
            keys->insert("world", "vdb_repository_TEST_dir/world-no-match-no-eol");
            tr1::shared_ptr<Repository> repo(VDBRepository::make_vdb_repository(
                        &env, keys));
            repo->world_interface->add_to_world(QualifiedPackageName("cat-one/foofoo"));
            std::ifstream world("vdb_repository_TEST_dir/world-no-match-no-eol");
            std::string world_content((std::istreambuf_iterator<char>(world)), std::istreambuf_iterator<char>());
            TEST_CHECK_EQUAL(world_content, "cat-one/foo\ncat-one/bar\ncat-one/oink\ncat-one/foofoo\n");
        }
    } test_vdb_repository_add_to_world_no_match_no_eol;
}

