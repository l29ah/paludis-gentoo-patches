#!/usr/bin/env ruby
# vim: set sw=4 sts=4 et tw=80 :
#
require 'Paludis'
require 'getoptlong'

include Paludis
include Paludis::QA

def need_entry_heading
    if @last_heading != @current_heading
        print "\n#{@current_heading}\n"
        @last_heading = @current_heading
    end
end

def set_entry_heading(heading, local_quiet = false)
    @current_heading = heading
    need_entry_heading unless @quiet || local_quiet
end

def display_header(result)
    need_entry_heading
    puts "#{result.item}: #{result.rule} :"
end

def display_header_once(once, result)
    unless once
        display_header(result)
        return true
    end
    false
end

def display_no_errors(result)
    display_header(result) if @verbose
end

def display_errors(result)
    done_out = false
    result.messages.each do |message|
        next if message.level < @min_level
        case message.level
            when QALevel::Info
                done_out = display_header_once(done_out, result)
                print "  info:        "
            when QALevel::Skip
                next unless @verbose
                done_out = display_header_once(done_out, result)
                print "  exclude:        "
            when QALevel::Minor
                done_out = display_header_once(done_out, result)
                print "  minor:       "
            when QALevel::Major
                done_out = display_header_once(done_out, result)
                print "  major:       "
            when QALevel::Fatal
                done_out = display_header_once(done_out, result)
                print "  fatal:       "
            when QALevel::Maybe
                done_out = display_header_once(done_out, result)
                print "  maybe:       "
            else
        end
        puts message.msg
    end
end

def do_check_kind(maker, ok, fatal, value)
    checks = maker.check_names
    checks.sort!
    checks.sort_by {|x| maker.find_check(x).is_important? ? 1 : 0 }
    checks.each do |check_name|
        begin
            unless @checks.empty?
                next unless @checks.include? check_name
            end

            unless @exclude_checks.empty?
                next if @exclude_checks.include? check_name
            end

            r = maker.find_check(check_name).check(value)

            if r.empty?
                display_no_errors(r)
                next
            end
            display_errors(r)
            case r.most_severe_level
                when QALevel::Info, QALevel::Skip, QALevel::Maybe
                    next
                when QALevel::Minor, QALevel::Major
                    ok = false
                    next
                when QALevel::Fatal
                    return [false, false]
            end
        rescue RuntimeError
            ok = false
            $stdout.puts "Eek! caught exception '#{$!}' when doing check '#{check_name}'"
        end
    end
    return [ok, false]
end

def do_check_package_dir(dir, env)
    ok = true
    fatal = false

    set_entry_heading "QA check for package directory #{dir}:"

    ok, fatal = do_check_kind(PackageDirCheckMaker.instance, ok, fatal, dir)

    unless fatal
        Dir.foreach(dir) do |d|
            full_dir = "#{dir}/#{d}"
            next if d == 'CVS' || d[0,1] == '.'
            next unless File.file? full_dir
            ok, fatal = do_check_kind(FileCheckMaker.instance, ok, fatal, full_dir)
        end
    end

    unless fatal
        Dir["#{dir}/*.ebuild"].each do |d|
            qpn = QualifiedPackageName.new(File.basename(File.dirname(dir)), File.basename(dir))
            ver = File.basename(d, '.ebuild').gsub(File.basename(dir) + '-','')
            ecd = EbuildCheckData.new(qpn, ver, env)

            ok, fatal = do_check_kind(EbuildCheckMaker.instance,ok,fatal, ecd)

            break if fatal
        end
    end

    unless fatal
        Dir["#{dir}/*.ebuild"].each do |d|
            env.portage_repository.profiles.each do |profile|
                unless @arches.empty?
                    next unless @arches.include? profile.arch
                end

                unless @exclude_arches.empty?
                    next if @exclude_arches.include? profile.arch
                end

                set_entry_heading("QA checks for package directory #{d} with profile #{profile.path}", true)
                qpn = QualifiedPackageName.new(File.basename(File.dirname(dir)), File.basename(dir))
                ver = File.basename(d, '.ebuild').gsub(File.basename(dir) + '-','')
                ppecd = PerProfileEbuildCheckData.new(qpn, ver, env, profile.path)

                ok, fatal = do_check_kind(PerProfileEbuildCheckMaker.instance, ok, fatal, ppecd)

                break if fatal
            end

            break if fatal
        end
    end

    if !ok && File.file?("#{dir}/metadata.xml")
        puts "metadata.xml"
        metadata = MetadataFile.new("#{dir}/metadata.xml")
        puts "  herds:       " + metadata.herds.join(', ') unless metadata.herds.empty?
        unless metadata.maintainers.empty?
            metadata.maintainers.each do |m|
                next if m[:email].nil? && m[:name].nil?
                print "  maintainer:  "

                if m[:email].nil?
                    print m[:name]
                elsif m[:name].nil?
                    print "<#{m[:email]}>"
                else
                    print "#{m[:name]} <#{m[:email]}>"
                end
                print "\n"
            end
        end
    end
    return ok
end

def do_check_category_dir(dir,env)
    set_entry_heading "QA checks for category directory #{dir}:"
    ok = true
    Dir.foreach(dir) do |d|
        next if File.basename(d) == 'CVS' || File.basename(d)[0,1] == '.'
        full_dir = "#{dir}/#{d}"
        if File.directory? full_dir
            ok &= do_check_package_dir(full_dir,env)
        elsif File.basename(d) == 'metadata.xml'
            fatal = false

            set_entry_heading "QA checks for category file #{full_dir}:"

            ok, fatal = do_check_kind(FileCheckMaker.instance, ok, fatal, full_dir)

            break if fatal
        end
    end
    return ok
end

def do_check_eclass_dir(dir, env)
    set_entry_heading "QA checks for eclass directory #{dir}:"
    ok = true
    Dir.foreach(dir) do |d|
        next if File.basename(d) == 'CVS' || File.basename(d)[0,1] == '.'
        if File.extname(d) == '.eclass'
            fatal = false
            full_dir = "#{dir}/#{d}"
            set_entry_heading "QA checks for eclass file #{full_dir}:"

            ok,fatal = do_check_kind(FileCheckMaker.instance, ok, fatal, full_dir)

            break if fatal
        end
    end
    return ok
end

def do_check_profiles_dir(dir, env)
    set_entry_heading "QA checks for profiles directory #{dir}:"

    ok, fatal = do_check_kind(ProfilesCheckMaker.instance, ok, fatal, dir)

    env.portage_repository.profiles.each do |p|
        break if fatal;
        set_entry_heading "QA checks for profile.desc entry #{p.path} #{p.arch} #{p.status}:"

        ok, fatal = do_check_kind(ProfileCheckMaker.instance, ok, fatal, ProfileCheckData.new(dir, p));

    end
end

def do_check_top_level(dir)
    set_entry_heading "QA checks for top level directory #{dir}"

    env = QAEnvironment.new(dir)
    ok = true

    Dir.foreach(dir) do |d|
        next if d == 'CVS' || d[0,1] == '.'
        full_dir = "#{dir}/#{d}"
        next unless File.directory? full_dir
        if d == 'eclass'
            ok &= do_check_eclass_dir("#{full_dir}", env)
        elsif d == 'profiles'
            ok &= do_check_profiles_dir("#{full_dir}", env)
        elsif env.package_database.fetch_repository(env.package_database.favourite_repository).has_category_named?(d)
            ok &= do_check_category_dir("#{full_dir}", env)
        end
    end
    return ok
end

def do_check(dir)
    if File.basename(dir) == 'eclass'
        env = QAEnvironment.new(File.dirname(dir), @write_cache_dir)
        return do_check_eclass_dir(dir, env)
    elsif File.basename(dir) == 'profiles'
        env = QAEnvironment.new(File.dirname(dir), @write_cache_dir)
        return do_check_profiles_dir(dir, env)
    elsif Dir["#{dir}/*\-*.ebuild"].length>0
        env = QAEnvironment.new(File.dirname(File.dirname(dir)))
        return do_check_package_dir(dir,env)
    elsif  File.directory? "#{dir}/profiles"
        return do_check_top_level(dir)
    elsif File.directory?(File.dirname(dir) + '/profiles')
        env = QAEnvironment.new(File.dirname(dir))
        return do_check_category_dir(dir, env)
    else
        $stderr.puts "qualudia.rb should be run inside a repository #{dir}"
        exit 1
    end
end

def describe_check(title, maker)
    puts "#{title}:"
    maker.check_names.each do |check_name|
        $stderr.puts "  #{check_name}:"
        $stderr.puts "    #{maker.find_check(check_name).describe}"
    end
    $stderr.puts
end

@verbose = false
@quiet = false
@min_level = QALevel::Info
@write_cache_dir = '/var/empty'
@checks = []
@exclude_checks = []
@arches = []
@exclude_arches = []
describe = false

Log.instance.log_level = LogLevel::Qa
Log.instance.program_name = $0

opts = GetoptLong.new(
    [ '--help',            '-h',  GetoptLong::NO_ARGUMENT ],
    [ '--version',         '-V',  GetoptLong::NO_ARGUMENT ],
    [ '--describe',        '-d',  GetoptLong::NO_ARGUMENT ],
    [ '--qa-check',        '-c',  GetoptLong::REQUIRED_ARGUMENT ],
    [ '--exclude-qa-check','-C',  GetoptLong::REQUIRED_ARGUMENT ],
    [ '--arch',            '-a',  GetoptLong::REQUIRED_ARGUMENT ],
    [ '--exclude-arch',    '-A',  GetoptLong::REQUIRED_ARGUMENT ],
    [ '--log-level',       '-L',  GetoptLong::REQUIRED_ARGUMENT ],
    [ '--message-level',   '-M',  GetoptLong::REQUIRED_ARGUMENT ],
    [ '--verbose',         '-v',  GetoptLong::NO_ARGUMENT ],
    [ '--quiet',           '-q',  GetoptLong::NO_ARGUMENT ],
    [ '--write-cache-dir',       GetoptLong::REQUIRED_ARGUMENT ])

opts.each do | opt, arg |
    case opt
    when '--help'
        puts "Usage: " + $0 + " [options]"
        puts "Usage: " + $0 + " [package/category ..]"
        puts
        puts "Actions:"
        puts "  --describe, -d          Display program version"
        puts "  --help, -h              Display a help message"
        puts "  --version, -V           Display program version"
        puts
        puts "Options for general checks:"
        puts "  --qa-check, -c          Only perform given check"
        puts "  --exclude-qa-check, -C   exclude given check"
        puts "  --arch, -a              Only perform checks for the specified arch"
        puts "  --exclude-arch, -A       Do not perform checks for the specified arch"
        puts "  --verbose, -v           Be verbose"
        puts "  --quiet, -q             Be quiet"
        puts "  --log-level, -L         Set log level"
        puts "      debug                 Show debug output (noisy)"
        puts "      qa                    Show QA messages and warnings only (default)"
        puts "      warning               Show warnings only"
        puts "      silent                Suppress all log messages (UNSAFE)"
        puts "  --message-level, -M     Specify the message level"
        puts "      info                  Show info and upwards (default)"
        puts "      minor                 Show minor and upwards"
        puts "      major                 Show major and upwards"
        puts "      fatal                 Show only fatals"
        puts
        puts "Configuration Options:"
        puts "  --write-cache-dir       Only perform given check"
        exit 0

    when '--version'
        puts $0.to_s.split(/\//).last + " " + Paludis::Version
        exit 0

    when '--describe'
        describe_check("Package Directory Checks", PackageDirCheckMaker.instance)
        describe_check("File Checks", FileCheckMaker.instance)
        describe_check("Ebuild Checks", EbuildCheckMaker.instance)
        describe_check("Per Profile Ebuild Checks", PerProfileEbuildCheckMaker.instance)
        describe_check("Top level profiles/ checks", ProfilesCheckMaker.instance)
        describe_check("Per profiles.desc entry checks", ProfileCheckMaker.instance)
        exit 0

    when '--qa-check'
        unless @exclude_checks.empty?
            $stderr.puts "Don't specify --exclude-qa-check and --qa-check in the same command"
            exit 1
        end
        @checks << arg

    when '--exclude-qa-check'
        unless @checks.empty?
            $stderr.puts "Don't specify --exclude-qa-check and --qa-check in the same command"
            exit 1
        end
        @exclude_checks << arg

    when '--arch'
        unless @exclude_arches.empty?
            $stderr.puts "Don't specify --exclude-arch and --arch in the same command"
            exit 1
        end
        @arches << arg

    when '--exclude-arch'
        unless @arches.empty?
            $stderr.puts "Don't specify --exclude-arch and --arch in the same command"
            exit 1
        end
        @exclude_arches << arg

    when '--verbose'
        @verbose = true

    when '--quiet'
        @quiet = true

    when '--log-level'
        case arg
        when 'debug'
            Log.instance.log_level = LogLevel::Debug
        when 'qa'
            Log.instance.log_level = LogLevel::Qa
        when 'warning'
            Log.instance.log_level = LogLevel::Warning
        when 'silent'
            Log.instance.log_level = LogLevel::Silent
        else
            $stderr.puts "Bad --log-level value " + arg
            exit 1
        end

    when '--message-level'
        case arg
        when 'info'
            @min_level = QALevel::Info
        when 'minor'
            @min_level = QALevel::Minor
        when 'major'
            @min_level = QALevel::Major
        when 'fatal'
            @min_level = QALevel::Fatal
        else
            $stderr.puts "Bad --message-level value " + arg
            exit 1
        end

    when '--write-cache-dir'
        @write_cache_dir = arg

    end
end
unless ARGV.empty?
    ARGV.each do |dir|
        if dir.include? '/'
            full_dir = dir
        else
            full_dir = "#{Dir.getwd}/#{dir}"
        end
        if File.directory? full_dir
            do_check(full_dir)
        else
            $stderr.puts "#{full_dir} is not a directory"
        end
    end
else
    do_check(Dir.getwd)
end
