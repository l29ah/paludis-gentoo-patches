#!/usr/bin/env bash
# vim: set sw=4 sts=4 et tw=0 :

echo -n "/* vim"
echo ": set ro : */"
echo
echo "/* ******************************************************** */"
echo "/* THIS IS A GENERATED FILE. DO NOT EDIT THIS FILE DIRECTLY */"
echo "/* ******************************************************** */"
echo

what_to_make=${1}
shift

for src in ${@} ; do
    if ! source ${src} ; then
        echo "source ${src} failed" 1>&2
        exit 7
    fi
done

set | grep '^make_enum_.*() $' | sort -r | \
while read a ; do
    a=${a##make_enum_}
    a=${a%% *}

    want_keys=( )
    want_key_descriptions=( )
    want_prefix_key=
    want_namespace=paludis
    want_destringify=

    key()
    {
        want_keys=( "${want_keys[@]}" "$1" )
        want_key_descriptions=( "${want_key_descriptions[@]}" "$2" )
    }

    prefix()
    {
        want_prefix_key=$1
    }

    namespace()
    {
        want_namespace=$1
    }

    doxygen_comment()
    {
        :
    }

    want_destringify()
    {
        want_destringify=yes
    }

    make_enum_${a}
    if [[ -z "${want_prefix_key}" ]] ; then
        echo "no prefix key set for ${a}" 1>&2
        exit 1
    fi

    key()
    {
        :
    }

    prefix()
    {
        :
    }

    namespace()
    {
        :
    }

    want_destringify()
    {
        :
    }

    if [[ "${what_to_make}" == "--header" ]] ; then

        doxygen_comment()
        {
            cat
        }

        make_enum_${a}

        echo "enum ${a}"
        echo "{"

        for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
            echo "    ${want_keys[${k}]},  ///< ${want_key_descriptions[${k}]}"
        done

        echo "    last_${want_prefix_key} ///< Number of keys in ${want_namespace}::${a}"

        echo "};"

        echo
        echo "/**"
        echo " * Write a ${a} to a stream."
        echo " */"
        echo "std::ostream &"
        echo "operator<< (std::ostream &, const $a &) PALUDIS_VISIBLE;"
        echo

        if [[ -n "${want_destringify}" ]] ; then
            echo "/**"
            echo " * Read a ${a} from a stream."
            echo " */"
            echo "std::istream &"
            echo "operator>> (std::istream &, $a &) PALUDIS_VISIBLE;"
            echo
        fi

    elif [[ "${what_to_make}" == "--source" ]] ; then

        echo "std::ostream &"
        echo "${want_namespace}::operator<< (std::ostream & o, const ${a} & s)"
        echo "{"
        echo "    do"
        echo "    {"
        echo "        switch (s)"
        echo "        {"

        for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
            echo "            case ${want_keys[${k}]}:"
            echo "                o << \"${want_keys[${k}]#${want_prefix_key}_}\";";
            echo "                continue;"
            echo
        done

        echo "            case last_${want_prefix_key}:"
        echo "                ;"
        echo "        }"
        echo "        throw InternalError(PALUDIS_HERE, \"Bad ${a} value '\" + paludis::stringify("
        echo "            static_cast<int>(s)));"
        echo "    } while (false);"
        echo
        echo "    return o;"
        echo "}"
        echo

        if [[ -n "${want_destringify}" ]] ; then
            echo "std::istream &"
            echo "${want_namespace}::operator>> (std::istream & s, $a & a)"
            echo "{"
            echo "    std::string value;"
            echo "    s >> value;"
            echo
            echo "    do"
            echo "    {"
            for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
                echo "        if (value == \"${want_keys[${k}]#${want_prefix_key}_}\")"
                echo "        {"
                echo "            a = ${want_keys[${k}]};"
                echo "            break;"
                echo "        }"
                echo
            done
            echo "        s.setstate(std::ios::badbit);"
            echo "    } while (false);"
            echo
            echo "    return s;"
            echo "}"
            echo
        fi

    else
        echo "bad argument (expected --header or --source)" 1>&2
        exit 1
    fi
done

