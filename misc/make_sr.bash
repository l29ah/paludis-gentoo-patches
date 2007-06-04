#!/bin/bash
# vim: set sw=4 sts=4 et tw=0 :

what_to_make=${1}
shift

echo -n "/* vim"
echo ": set ro : */"
echo
echo "/* ******************************************************** */"
echo "/* THIS IS A GENERATED FILE. DO NOT EDIT THIS FILE DIRECTLY */"
echo "/* ******************************************************** */"
echo

if [[ "${what_to_make}" == "--header" ]] ; then
    echo "#ifdef DOXYGEN"
    echo "// doxygen needs this to get namespaces right"
    echo "namespace paludis"
    echo "{"
    echo "#endif"
fi

for src in ${@} ; do
    if ! source ${src} ; then
        echo "source ${src} failed" 1>&2
        exit 7
    fi
done

make_const_ref()
{
    local x=${@}
    if [[ "${x%\*}" != "${x}" ]] ; then
        echo "${@}" const
    elif [[ "${1#const}" != "${1}" ]] ; then
        echo "${@}" const "&"
    else
        echo const "${@}" "&"
    fi
}

set | grep '^make_class_.*() $' | sort -r | \
while read a ; do
    a=${a##make_class_}
    a=${a%% *}

    want_named_args=
    want_visible=
    want_keys=( )
    want_key_types=( )
    want_cache_func=( )
    want_inherit=( )
    want_comparison_operators=
    want_comparison_fields=( )

    extra_constructors()
    {
        :
    }

    extra_methods()
    {
        :
    }

    allow_named_args()
    {
        want_named_args=${1:-hh}
    }

    visible()
    {
        want_visible=yes
    }

    doxygen_comment()
    {
        :
    }

    inherit()
    {
        want_inherit=( "${want_inherit[@]}" "$1" )
    }

    key()
    {
        want_keys=( "${want_keys[@]}" "$1" )
        want_key_types=( "${want_key_types[@]}" "$2" )
        want_cache_func=( "${want_cache_func[@]}" "__unset_function__" )
    }

    cache_key()
    {
        want_keys=( "${want_keys[@]}" "$1" )
        want_key_types=( "${want_key_types[@]}" "$2 -> $3" )
        want_cache_func=( "${want_cache_func[@]}" "$4" )
    }

    comparison_operators()
    {
        want_comparison_operators=$1
        shift
        want_comparison_fields=( $@ )
    }

    make_class_${a}

    if [[ "${want_comparison_fields[0]}" == "all" ]] ; then
        want_comparison_fields=( ${want_keys[@]} )
    fi

    doxygen_comment()
    {
        cat
    }

    allow_named_args()
    {
        :
    }

    key()
    {
        :
    }

    cache_key()
    {
        :
    }

    inherit()
    {
        :
    }

    comparison_operators()
    {
        :
    }


    make_class_${a}

    doxygen_comment()
    {
        :
    }


    extra_constructors()
    {
        echo
        cat
    }

    extra_methods()
    {
        echo
        cat
    }

    if [[ "${what_to_make}" == "--header" ]] ; then
        current_class=${a}

        if [[ -z ${want_visible} ]] ; then
            echo -n "class ${a}"
        else
            echo -n "class PALUDIS_VISIBLE ${a}"
        fi

        if [[ 0 != "${#want_inherit[@]}" ]] || [[ -n "${want_comparison_operators}" ]] ; then
            echo " :"
            if [[ -n "${want_comparison_operators}" ]] ; then
                if [[ "${want_comparison_operators}" == "all" ]] ; then
                    echo -n "        public paludis::relational_operators::HasRelationalOperators"
                elif [[ "${want_comparison_operators}" == "equality" ]] ; then
                    echo -n "        public paludis::equality_operators::HasEqualityOperators"
                else
                    echo "bad first parameter for comparison_operators" 1>&2
                    exit 1
                fi

                if [[ 0 == ${#want_inherit[@]} ]] ; then
                    echo
                else
                    echo ","
                fi
            fi

            for (( k = 0 ; k < ${#want_inherit[@]} ; k++ )) ; do
                echo -n "        ${want_inherit[${k}]}"
                if [[ ${k} == $(( ${#want_inherit[@]} - 1 )) ]] ; then
                    echo
                else
                    echo ","
                fi
            done
        else
            echo
        fi

        echo "{"
        echo "        ///\\name Data members"
        echo "        ///\\{"
        echo

        for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
            if [[ ${want_key_types[${k}]/->} == ${want_key_types[${k}]} ]] ; then
                echo "    public:"
                echo "        ${want_key_types[${k}]} ${want_keys[${k}]};"
                echo
            else
                echo "    protected:"
                echo "        ${want_key_types[${k}]% ->*} raw_${want_keys[${k}]};"
                echo "        mutable paludis::tr1::shared_ptr<const ${want_key_types[${k}]#*-> }> cached_${want_keys[${k}]};"
                echo
            fi
        done

        echo
        echo "        ///\\}"
        echo

        echo "    public:"
        echo

        echo "        ///\\name Cache key methods"
        echo "        ///\\{"
        echo
        for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
            if [[ ${want_key_types[${k}]/->} != ${want_key_types[${k}]} ]] ; then
                echo "        ${current_class} & set_${want_keys[${k}]}($(make_const_ref ${want_key_types[${k}]% ->*}));"
                echo
                echo "        const ${want_key_types[${k}]%-> *} get_raw_${want_keys[${k}]}() const"
                echo "                PALUDIS_ATTRIBUTE((warn_unused_result));"
                echo
                echo "        paludis::tr1::shared_ptr<const ${want_key_types[${k}]#*-> }> ${want_keys[${k}]}() const"
                echo "                PALUDIS_ATTRIBUTE((warn_unused_result));"
                echo
            fi
        done
        echo
        echo "        ///\\}"


        if [[ -n "${want_comparison_operators}" ]] ; then

            echo "        ///\\name Comparison operators"
            echo "        ///\\{"
            echo

            if [[ "${want_comparison_operators}" == "all" ]] ; then
                echo "        bool operator== (const ${a} & other) const;"
                echo "        bool operator< (const ${a} & other) const;"
            elif [[ "${want_comparison_operators}" == "equality" ]] ; then
                echo "        bool operator== (const ${a} & other) const;"
            else
                echo "bad first parameter for comparison_operators" 1>&2
                exit 1
            fi

            echo
            echo "        ///\\}"
            echo
        fi

        echo "        ///\\name Basic operations"
        echo "        ///\\{"
        echo

        echo "        explicit ${a}("
        for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
            echo -n "                $(make_const_ref "${want_key_types[${k}]% ->*}") value_for_${want_keys[${k}]}"
            if [[ $(( ${k} + 1 )) -lt ${#want_keys[@]} ]] ; then
                echo ","
            else
                echo
            fi
        done
        echo "            );"
        echo

        make_class_${a}

        echo
        echo "        ///\\}"
        echo

        if [[ -n "${want_named_args}" ]] ; then

            echo "    public:"
            echo "        ///\\name Named argument constructor"
            echo "        ///\\{"
            echo

            echo "/**"
            echo " * Named arguments for the constructor for ${current_class}."
            echo " *"
            echo " * \\see ${current_class}"
            echo " * \\ingroup grpnamedarguments"
            echo " * \\nosubgrouping"
            echo " */"

            echo "template <"
            for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
                echo -n "        bool has_${k}_ = true"
                if [[ $(( ${k} + 1 )) -lt ${#want_keys[@]} ]] ; then
                    echo ","
                else
                    echo ">"
                fi
            done
            echo -n "class NamedArguments"

            if [[ "${want_named_args}" == "cc" ]] ; then
                echo ";"
            else
                echo
                echo "{"
                echo "    friend class ${current_class};"
                echo "    template <"
                for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
                    echo -n "            bool"
                    if [[ $(( ${k} + 1 )) -lt ${#want_keys[@]} ]] ; then
                        echo ","
                    else
                        echo ">"
                    fi
                done
                echo "        friend class ${current_class}::NamedArguments;"
                echo
                echo "    private:"

                for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
                    echo "        const typename paludis::Select<has_${k}_, ${want_key_types[${k}]% ->*}, paludis::Empty>::Type _v${k};"
                done

                echo "        NamedArguments("
                for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
                    echo -n "            const typename paludis::Select<has_${k}_, ${want_key_types[${k}]% ->*}, paludis::Empty>::Type & v${k}"
                    if [[ $(( ${k} + 1 )) -lt ${#want_keys[@]} ]] ; then
                        echo ","
                    else
                        echo ") :"
                    fi
                done
                for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
                    echo -n "            _v${k}(v${k})"
                    if [[ $(( ${k} + 1 )) -lt ${#want_keys[@]} ]] ; then
                        echo ","
                    else
                        echo
                    fi
                done
                echo "        {"
                echo "        }"

                echo
                echo "    public:"
                echo

                echo "        ///\\name Value specification functions"
                echo "        ///\\{"

                for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
                    echo "        /**"
                    echo "         * Set the value of the ${current_class}.${want_keys[${k}]} member."
                    echo "         */"
                    echo "        NamedArguments<"
                    for (( kk = 0 ; kk < ${#want_keys[@]} ; kk++ )) ; do
                        if [[ ${k} == ${kk} ]] ; then
                            echo -n "            true"
                        else
                            echo -n "            has_${kk}_"
                        fi
                        if [[ $(( ${kk} + 1 )) -lt ${#want_keys[@]} ]] ; then
                            echo ","
                        else
                            echo ">"
                        fi
                    done
                    echo "        ${want_keys[${k}]}($(make_const_ref "${want_key_types[${k}]% ->*}" ) v)"
                    echo "        {"
                    echo "            const typename paludis::Select<has_${k}_, void, paludis::Empty>::Type"
                    echo "                check_not_already_specified = paludis::Empty::instance;"
                    echo
                    echo "            return NamedArguments<"
                    for (( kk = 0 ; kk < ${#want_keys[@]} ; kk++ )) ; do
                        if [[ ${k} == ${kk} ]] ; then
                            echo -n "            true"
                        else
                            echo -n "            has_${kk}_"
                        fi
                        if [[ $(( ${kk} + 1 )) -lt ${#want_keys[@]} ]] ; then
                            echo ","
                        else
                            echo ">("
                        fi
                    done
                    for (( kk = 0 ; kk < ${#want_keys[@]} ; kk++ )) ; do
                        if [[ ${k} == ${kk} ]] ; then
                            echo -n "            v"
                        else
                            echo -n "            _v${kk}"
                        fi
                        if [[ $(( ${kk} + 1 )) -lt ${#want_keys[@]} ]] ; then
                            echo ","
                        else
                            echo ");"
                        fi
                    done

                    echo "        }"
                    echo
                done
                echo "        ///\\}"

                echo
                echo "};"
            fi

            echo "        /**"
            echo "         * Create using named arguments."
            echo "         */"
            echo "        ${current_class}(const NamedArguments<"
            for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
                echo -n "            true"
                if [[ $(( ${k} + 1 )) -lt ${#want_keys[@]} ]] ; then
                    echo ","
                else
                    echo "> & va);"
                fi
            done

            echo
            echo "        /**"
            echo "         * Convenience function to create an empty named arguments class."
            echo "         */"
            echo "        static NamedArguments<"
            for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
                echo -n "            false"
                if [[ $(( ${k} + 1 )) -lt ${#want_keys[@]} ]] ; then
                    echo ","
                else
                    echo ">"
                fi
            done
            echo "        create() PALUDIS_ATTRIBUTE((warn_unused_result));"

            echo
            echo "        ///\\}"

        fi
        echo "};"

    elif [[ "${what_to_make}" == "--source" ]] ; then

        if [[ "${want_named_args}" == "cc" ]] ; then
            echo "template <"
            for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
                echo -n "        bool has_${k}_"
                if [[ $(( ${k} + 1 )) -lt ${#want_keys[@]} ]] ; then
                    echo ","
                else
                    echo ">"
                fi
            done
            echo "class ${a}::NamedArguments"
            echo "{"
            echo "    friend class ${a};"
            echo "    public:"

            for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
                echo "        const typename paludis::Select<has_${k}_, ${want_key_types[${k}]% ->*}, paludis::Empty>::Type _v${k};"
            done

            echo "        NamedArguments("
            for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
                echo -n "            const typename paludis::Select<has_${k}_, ${want_key_types[${k}]% ->*}, paludis::Empty>::Type & v${k}"
                if [[ $(( ${k} + 1 )) -lt ${#want_keys[@]} ]] ; then
                    echo ","
                else
                    echo ") :"
                fi
            done
            for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
                echo -n "            _v${k}(v${k})"
                if [[ $(( ${k} + 1 )) -lt ${#want_keys[@]} ]] ; then
                    echo ","
                else
                    echo
                fi
            done
            echo "        {"
            echo "        }"

            echo
            echo "    public:"
            echo

            echo "        ///\\name Value specification functions"
            echo "        ///\\{"

            for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
                echo "        /**"
                echo "         * Set the value of the ${current_class}.${want_keys[${k}]} member."
                echo "         */"
                echo "        NamedArguments<"
                for (( kk = 0 ; kk < ${#want_keys[@]} ; kk++ )) ; do
                    if [[ ${k} == ${kk} ]] ; then
                        echo -n "            true"
                    else
                        echo -n "            has_${kk}_"
                    fi
                    if [[ $(( ${kk} + 1 )) -lt ${#want_keys[@]} ]] ; then
                        echo ","
                    else
                        echo ">"
                    fi
                done
                echo "        ${want_keys[${k}]}($(make_const_ref "${want_key_types[${k}]% ->*}" ) v)"
                echo "        {"
                echo "            const typename paludis::Select<has_${k}_, void, paludis::Empty>::Type"
                echo "                check_not_already_specified = paludis::Empty::instance;"
                echo
                echo "            return NamedArguments<"
                for (( kk = 0 ; kk < ${#want_keys[@]} ; kk++ )) ; do
                    if [[ ${k} == ${kk} ]] ; then
                        echo -n "            true"
                    else
                        echo -n "            has_${kk}_"
                    fi
                    if [[ $(( ${kk} + 1 )) -lt ${#want_keys[@]} ]] ; then
                        echo ","
                    else
                        echo ">("
                    fi
                done
                for (( kk = 0 ; kk < ${#want_keys[@]} ; kk++ )) ; do
                    if [[ ${k} == ${kk} ]] ; then
                        echo -n "            v"
                    else
                        echo -n "            _v${kk}"
                    fi
                    if [[ $(( ${kk} + 1 )) -lt ${#want_keys[@]} ]] ; then
                        echo ","
                    else
                        echo ");"
                    fi
                done

                echo "        }"
                echo
            done
            echo "        ///\\}"

            echo
            echo "};"
        fi

        echo "${a}::${a}("
        for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
            echo -n "        $(make_const_ref "${want_key_types[${k}]% ->*}") value_for_${want_keys[${k}]}"
            if [[ $(( ${k} + 1 )) -lt ${#want_keys[@]} ]] ; then
                echo ","
            else
                echo ") :"
            fi
        done
        for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
            if [[ ${want_key_types[${k}]/->} == ${want_key_types[${k}]} ]] ; then
                echo -n "    ${want_keys[${k}]}(value_for_${want_keys[${k}]})"
            else
                echo "    raw_${want_keys[${k}]}(value_for_${want_keys[${k}]}),"
                echo -n "    cached_${want_keys[${k}]}()"
            fi
            if [[ $(( ${k} + 1 )) -lt ${#want_keys[@]} ]] ; then
                echo ","
            else
                echo
            fi
        done
        echo "{"
        echo "}"
        echo

        for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
            if [[ ${want_key_types[${k}]/->} != ${want_key_types[${k}]} ]] ; then
                echo "${a} &"
                echo "${a}::set_${want_keys[${k}]}($(make_const_ref ${want_key_types[${k}]% ->*}) va)"
                echo "{"
                echo "    raw_${want_keys[${k}]} = va;"
                echo "    cached_${want_keys[${k}]}.reset();"
                echo "    return *this;"
                echo "}"
                echo
                echo "const ${want_key_types[${k}]%-> *}"
                echo "${a}::get_raw_${want_keys[${k}]}() const"
                echo "{"
                echo "    return raw_${want_keys[${k}]};"
                echo "}"
                echo
                echo "paludis::tr1::shared_ptr<const ${want_key_types[${k}]#*-> }>"
                echo "${a}::${want_keys[${k}]}() const"
                echo "{"
                echo "    if (! cached_${want_keys[${k}]})"
                echo "        cached_${want_keys[${k}]} = ${want_cache_func[${k}]}(raw_${want_keys[${k}]});"
                echo "    return cached_${want_keys[${k}]};"
                echo "}"
                echo
            fi
        done

        if [[ -n "${want_named_args}" ]] ; then
            echo "${a}::${a}(const NamedArguments<"
            for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
                echo -n "            true"
                if [[ $(( ${k} + 1 )) -lt ${#want_keys[@]} ]] ; then
                    echo ","
                else
                    echo "> & va) :"
                fi
            done
            for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
                if [[ ${want_key_types[${k}]/->} == ${want_key_types[${k}]} ]] ; then
                    echo -n "            ${want_keys[${k}]}(va._v${k})"
                else
                    echo "            raw_${want_keys[${k}]}(va._v${k}),"
                    echo -n "            cached_${want_keys[${k}]}()"
                fi
                if [[ $(( ${k} + 1 )) -lt ${#want_keys[@]} ]] ; then
                    echo ","
                else
                    echo
                fi
            done
            echo "        {"
            echo "        }"
        fi

        if [[ -n "${want_comparison_operators}" ]] ; then

            if [[ "${want_comparison_operators}" == "all" ]] ; then
                echo "bool"
                echo "${a}::operator== (const ${a} & other) const"
                echo "{"
                for (( k = 0 ; k < ${#want_comparison_fields[@]} ; k++ )) ; do
                    w="${want_comparison_fields[${k}]}"
                    s=${w//[^*]}
                    w=${w#\*}
                    echo "    if (${want_comparison_fields[${k}]} != ${s}other.${w})"
                    echo "        return false;"
                done
                echo "    return true;"
                echo "}"
                echo
                echo "bool"
                echo "${a}::operator< (const ${a} & other) const"
                echo "{"
                echo "    using namespace std::rel_ops;"
                echo
                for (( k = 0 ; k < ${#want_comparison_fields[@]} ; k++ )) ; do
                    w="${want_comparison_fields[${k}]}"
                    s=${w//[^*]}
                    w=${w#\*}
                    echo "    if (${want_comparison_fields[${k}]} < ${s}other.${w})"
                    echo "        return true;"
                    echo "    if (${want_comparison_fields[${k}]} > ${s}other.${w})"
                    echo "        return false;"
                done
                echo "    return false;"
                echo "}"
                echo
            elif [[ "${want_comparison_operators}" == "equality" ]] ; then
                echo "bool"
                echo "${a}::operator== (const ${a} & other) const"
                echo "{"
                for (( k = 0 ; k < ${#want_comparison_fields[@]} ; k++ )) ; do
                    w="${want_comparison_fields[${k}]}"
                    s=${w//[^*]}
                    w=${w#\*}
                    echo "    if (${want_comparison_fields[${k}]} != ${s}other.${w})"
                    echo "        return false;"
                done
                echo "    return true;"
                echo "}"
                echo
            else
                echo "bad first parameter for comparison_operators" 1>&2
                exit 1
            fi
        fi

        if [[ -n "${want_named_args}" ]] ; then
            echo
            echo "${a}::NamedArguments<"
            for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
                echo -n "            false"
                if [[ $(( ${k} + 1 )) -lt ${#want_keys[@]} ]] ; then
                    echo ","
                else
                    echo ">"
                fi
            done
            echo "${a}::create()"
            echo "{"
            echo "    return ${a}::NamedArguments<"
            for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
                echo -n "            false"
                if [[ $(( ${k} + 1 )) -lt ${#want_keys[@]} ]] ; then
                    echo ","
                else
                    echo ">("
                fi
            done
            for (( k = 0 ; k < ${#want_keys[@]} ; k++ )) ; do
                echo -n "            paludis::Empty::instance"
                if [[ $(( ${k} + 1 )) -lt ${#want_keys[@]} ]] ; then
                    echo ","
                else
                    echo ");"
                fi
            done
            echo "}"

        fi

    else
        echo "bad argument (expected --header or --source)" 1>&2
        exit 1
    fi
done

if [[ "${what_to_make}" == "--header" ]] ; then
    echo "#ifdef DOXYGEN"
    echo "// end paludis namespace for doxygen"
    echo "}"
    echo "#endif"
fi

