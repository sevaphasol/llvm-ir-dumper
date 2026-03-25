#include <array>
#include <cerrno>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <unistd.h>
#include <vector>

#include <sys/wait.h>

#include <argparse/argparse.hpp>

namespace fs = std::filesystem;

namespace {

struct DefaultPaths
{
    fs::path repo_root;
    fs::path workdir;
    fs::path plugin_path;
    fs::path converter_path;
    fs::path profile_tool_path;
    fs::path binary_out;
};

struct Options
{
    fs::path              workdir;
    fs::path              source;
    fs::path              plugin_path;
    fs::path              converter_path;
    fs::path              profile_tool_path;
    fs::path              before_ll;
    fs::path              after_ll;
    fs::path              before_json;
    fs::path              after_json;
    fs::path              before_dot;
    fs::path              after_dot;
    fs::path              profile_json;
    fs::path              profile_dot;
    fs::path              before_svg;
    fs::path              after_svg;
    fs::path              profile_svg;
    fs::path              binary_out;
    std::string           opt_level;
    bool                  no_svg            = false;
    bool                  inject_logging    = false;
    bool                  profile_after_run = false;
    std::vector<std::string> run_args;
    std::vector<std::string> extra_clang_args;
};

bool
isRootPath( const fs::path& path )
{
    return path == path.root_path();
}

fs::path
expandUser( fs::path path )
{
    const auto text = path.string();
    if ( text.empty() || text[0] != '~' )
    {
        return path;
    }

    const char* home = std::getenv( "HOME" );
    if ( home == nullptr || *home == '\0' )
    {
        return path;
    }

    if ( text == "~" )
    {
        return fs::path( home );
    }

    if ( text.size() >= 2 && text[1] == '/' )
    {
        return fs::path( home ) / text.substr( 2 );
    }

    return path;
}

fs::path
makeAbsolute( const fs::path& path )
{
    return path.is_absolute() ? path : fs::absolute( path );
}

fs::path
findRepoRoot( const fs::path& executable_path )
{
    auto candidate = makeAbsolute( executable_path ).parent_path();

    while ( !candidate.empty() )
    {
        if ( fs::exists( candidate / "scripts" / "compile_with_plugin.py" ) &&
             fs::exists( candidate / "CMakeLists.txt" ) )
        {
            return candidate;
        }

        if ( isRootPath( candidate ) )
        {
            break;
        }

        candidate = candidate.parent_path();
    }

    return fs::current_path();
}

DefaultPaths
buildDefaultPaths( const char* argv0 )
{
    const auto repo_root = findRepoRoot( argv0 );

    return DefaultPaths{
        .repo_root         = repo_root,
        .workdir           = repo_root,
        .plugin_path       = repo_root / "install" / "lib" / "libLLVMIRDumper.so",
        .converter_path    = repo_root / "install" / "bin" / "ir_graph_to_dot",
        .profile_tool_path = repo_root / "install" / "bin" / "ir_graph_profile_merge",
        .binary_out        = repo_root / "build" / "a.out",
    };
}

void
configureProgram( argparse::ArgumentParser& program, const DefaultPaths& defaults )
{
    program.add_description(
        "Compile a source file with the LLVM IR dumper plugin, dump the IR graph to JSON, and convert it to dot/SVG."
    );

    program.add_argument( "--workdir" )
        .default_value( defaults.workdir.string() )
        .help( "Parent directory for source, llvm_ir, json, dot and svg outputs." );
    program.add_argument( "--source" ).required().help( "Path to the source file to compile." );
    program.add_argument( "--plugin-path" )
        .default_value( defaults.plugin_path.string() )
        .help( "Path to libLLVMIRDumper.so." );
    program.add_argument( "--converter-path" )
        .default_value( defaults.converter_path.string() )
        .help( "Path to ir_graph_to_dot executable." );
    program.add_argument( "--profile-tool-path" )
        .default_value( defaults.profile_tool_path.string() )
        .help( "Path to ir_graph_profile_merge executable." );

    program.add_argument( "--before-ll" )
        .default_value( std::string( "llvm_ir/before_opt.ll" ) )
        .help( "Output path for the before-optimization LLVM IR." );
    program.add_argument( "--after-ll" )
        .default_value( std::string( "llvm_ir/after_opt.ll" ) )
        .help( "Output path for the after-optimization LLVM IR." );
    program.add_argument( "--before-json" )
        .default_value( std::string( "json/before_opt.json" ) )
        .help( "Output path for serialized graph before optimization." );
    program.add_argument( "--after-json" )
        .default_value( std::string( "json/after_opt.json" ) )
        .help( "Output path for serialized graph after optimization." );
    program.add_argument( "--before-dot" )
        .default_value( std::string( "dot/before_opt.dot" ) )
        .help( "Output path for the before-optimization dot dump." );
    program.add_argument( "--after-dot" )
        .default_value( std::string( "dot/after_opt.dot" ) )
        .help( "Output path for the after-optimization dot dump." );
    program.add_argument( "--profile-json" )
        .default_value( std::string( "json/after_opt_profiled.json" ) )
        .help( "Output path for merged static+dynamic graph JSON." );
    program.add_argument( "--profile-dot" )
        .default_value( std::string( "dot/after_opt_profiled.dot" ) )
        .help( "Output path for profiled dot dump." );
    program.add_argument( "--before-svg" )
        .default_value( std::string( "svg/before_opt.svg" ) )
        .help( "Output path for the before-optimization SVG." );
    program.add_argument( "--after-svg" )
        .default_value( std::string( "svg/after_opt.svg" ) )
        .help( "Output path for the after-optimization SVG." );
    program.add_argument( "--profile-svg" )
        .default_value( std::string( "svg/after_opt_profiled.svg" ) )
        .help( "Output path for the profiled SVG." );
    program.add_argument( "--binary-out" )
        .default_value( defaults.binary_out.string() )
        .help( "Output path for the compiled binary." );
    program.add_argument( "--opt-level" )
        .default_value( std::string( "O2" ) )
        .help( "Optimization level passed to clang." );

    program.add_argument( "--no-svg" ).default_value( false ).implicit_value( true ).help(
        "Do not render dot dumps to SVG via Graphviz dot." );
    program.add_argument( "--inject-logging" )
        .default_value( false )
        .implicit_value( true )
        .help( "Inject runtime logging for function/basic-block/edge events." );
    program.add_argument( "--profile-after-run" )
        .default_value( false )
        .implicit_value( true )
        .help( "Run the built binary, merge runtime logs, and render profiled outputs." );

    program.add_argument( "--run-arg" )
        .default_value( std::vector<std::string>{} )
        .append()
        .help( "Argument passed to the profiled binary. Can be specified multiple times." );
    program.add_argument( "--extra-clang-arg" )
        .default_value( std::vector<std::string>{} )
        .append()
        .help( "Extra argument forwarded to clang. Can be specified multiple times." );
}

std::string
quoteCommandPart( std::string_view part )
{
    if ( part.find_first_of( " \t\n\"'\\$" ) == std::string_view::npos )
    {
        return std::string( part );
    }

    std::string quoted = "'";
    for ( const char ch : part )
    {
        if ( ch == '\'' )
        {
            quoted += "'\\''";
        } else
        {
            quoted += ch;
        }
    }
    quoted += "'";
    return quoted;
}

std::string
formatCommand( const std::vector<std::string>& command )
{
    std::ostringstream stream;
    bool               first = true;

    for ( const auto& part : command )
    {
        if ( !first )
        {
            stream << ' ';
        }
        first = false;
        stream << quoteCommandPart( part );
    }

    return stream.str();
}

void
throwSystemError( const std::string& action )
{
    throw std::system_error( errno, std::generic_category(), action );
}

int
runCommand( const std::vector<std::string>& command )
{
    if ( command.empty() )
    {
        throw std::runtime_error( "attempted to execute an empty command" );
    }

    std::cerr << formatCommand( command ) << '\n';

    const pid_t child_pid = ::fork();
    if ( child_pid < 0 )
    {
        throwSystemError( "fork" );
    }

    if ( child_pid == 0 )
    {
        std::vector<char*> argv;
        argv.reserve( command.size() + 1 );
        for ( const auto& part : command )
        {
            argv.push_back( const_cast<char*>( part.c_str() ) );
        }
        argv.push_back( nullptr );

        ::execvp( command.front().c_str(), argv.data() );
        std::perror( "execvp" );
        std::_Exit( 127 );
    }

    int wait_status = 0;
    while ( ::waitpid( child_pid, &wait_status, 0 ) < 0 )
    {
        if ( errno == EINTR )
        {
            continue;
        }

        throwSystemError( "waitpid" );
    }

    if ( WIFEXITED( wait_status ) )
    {
        return WEXITSTATUS( wait_status );
    }

    if ( WIFSIGNALED( wait_status ) )
    {
        throw std::runtime_error( "command terminated by signal " +
                                  std::to_string( WTERMSIG( wait_status ) ) + ": " +
                                  formatCommand( command ) );
    }

    throw std::runtime_error( "command terminated unexpectedly: " + formatCommand( command ) );
}

void
execCommand( const std::vector<std::string>& command )
{
    const int exit_code = runCommand( command );
    if ( exit_code != 0 )
    {
        throw std::runtime_error( "command failed with exit code " +
                                  std::to_string( exit_code ) + ": " +
                                  formatCommand( command ) );
    }
}

fs::path
resolveIndependentPath( const std::string& raw_path )
{
    return makeAbsolute( expandUser( fs::path( raw_path ) ) );
}

fs::path
resolveWorkdirPath( const fs::path& workdir, const std::string& raw_path )
{
    const auto path = expandUser( fs::path( raw_path ) );
    return path.is_absolute() ? path : workdir / path;
}

bool
isExistingFile( const fs::path& path )
{
    std::error_code ec;
    return fs::exists( path, ec ) && !fs::is_directory( path, ec );
}

void
requireExistingFile( const fs::path& path, std::string_view description )
{
    if ( !isExistingFile( path ) )
    {
        throw std::runtime_error( std::string( description ) + " not found: " + path.string() );
    }
}

void
ensureParentDir( const fs::path& path )
{
    const auto parent = path.parent_path();
    if ( !parent.empty() )
    {
        std::error_code ec;
        fs::create_directories( parent, ec );
        if ( ec )
        {
            throw std::runtime_error( "unable to create directory " + parent.string() + ": " +
                                      ec.message() );
        }
    }
}

bool
hasExecutableInPath( std::string_view program_name )
{
    const char* path_env = std::getenv( "PATH" );
    if ( path_env == nullptr )
    {
        return false;
    }

    std::string_view path_entries( path_env );
    std::size_t      cursor = 0;

    while ( cursor <= path_entries.size() )
    {
        const auto separator = path_entries.find( ':', cursor );
        const auto entry = path_entries.substr(
            cursor,
            separator == std::string_view::npos ? path_entries.size() - cursor
                                                : separator - cursor );
        const fs::path candidate =
            ( entry.empty() ? fs::current_path() : fs::path( entry ) ) / std::string( program_name );

        if ( ::access( candidate.c_str(), X_OK ) == 0 )
        {
            return true;
        }

        if ( separator == std::string_view::npos )
        {
            break;
        }

        cursor = separator + 1;
    }

    return false;
}

Options
parseOptions( int argc, char** argv )
{
    const auto defaults = buildDefaultPaths( argv[0] );

    argparse::ArgumentParser program( "compile_with_plugin" );
    configureProgram( program, defaults );
    program.parse_args( argc, argv );

    const auto workdir = resolveIndependentPath( program.get<std::string>( "--workdir" ) );

    return Options{
        .workdir           = workdir,
        .source            = resolveWorkdirPath( workdir, program.get<std::string>( "--source" ) ),
        .plugin_path       = resolveIndependentPath( program.get<std::string>( "--plugin-path" ) ),
        .converter_path    = resolveIndependentPath( program.get<std::string>( "--converter-path" ) ),
        .profile_tool_path = resolveIndependentPath( program.get<std::string>( "--profile-tool-path" ) ),
        .before_ll         = resolveWorkdirPath( workdir, program.get<std::string>( "--before-ll" ) ),
        .after_ll          = resolveWorkdirPath( workdir, program.get<std::string>( "--after-ll" ) ),
        .before_json       = resolveWorkdirPath( workdir, program.get<std::string>( "--before-json" ) ),
        .after_json        = resolveWorkdirPath( workdir, program.get<std::string>( "--after-json" ) ),
        .before_dot        = resolveWorkdirPath( workdir, program.get<std::string>( "--before-dot" ) ),
        .after_dot         = resolveWorkdirPath( workdir, program.get<std::string>( "--after-dot" ) ),
        .profile_json      = resolveWorkdirPath( workdir, program.get<std::string>( "--profile-json" ) ),
        .profile_dot       = resolveWorkdirPath( workdir, program.get<std::string>( "--profile-dot" ) ),
        .before_svg        = resolveWorkdirPath( workdir, program.get<std::string>( "--before-svg" ) ),
        .after_svg         = resolveWorkdirPath( workdir, program.get<std::string>( "--after-svg" ) ),
        .profile_svg       = resolveWorkdirPath( workdir, program.get<std::string>( "--profile-svg" ) ),
        .binary_out        = resolveWorkdirPath( workdir, program.get<std::string>( "--binary-out" ) ),
        .opt_level         = program.get<std::string>( "--opt-level" ),
        .no_svg            = program.get<bool>( "--no-svg" ),
        .inject_logging    = program.get<bool>( "--inject-logging" ),
        .profile_after_run = program.get<bool>( "--profile-after-run" ),
        .run_args          = program.get<std::vector<std::string>>( "--run-arg" ),
        .extra_clang_args  = program.get<std::vector<std::string>>( "--extra-clang-arg" ),
    };
}

void
validateOptions( const Options& options )
{
    requireExistingFile( options.source, "Source file" );
    requireExistingFile( options.plugin_path, "Plugin library" );
    requireExistingFile( options.converter_path, "Graph converter executable" );

    if ( options.profile_after_run )
    {
        requireExistingFile( options.profile_tool_path, "Graph profile merge executable" );
    }

    if ( !options.no_svg && !hasExecutableInPath( "dot" ) )
    {
        throw std::runtime_error( "Graphviz 'dot' executable was not found in PATH." );
    }
}

void
execClang( const Options& options )
{
    ensureParentDir( options.binary_out );
    ensureParentDir( options.before_ll );
    ensureParentDir( options.after_ll );
    ensureParentDir( options.before_json );
    ensureParentDir( options.after_json );

    std::vector<std::string> command = {
        "clang",
        "-Xclang",
        "-load",
        "-Xclang",
        options.plugin_path.string(),
        "-fpass-plugin=" + options.plugin_path.string(),
        "-mllvm",
        "-dumper-pass-json-out-before-opt=" + options.before_json.string(),
        "-mllvm",
        "-dumper-pass-json-out-after-opt=" + options.after_json.string(),
        "-mllvm",
        "-dumper-pass-ir-out-before-opt=" + options.before_ll.string(),
        "-mllvm",
        "-dumper-pass-ir-out-after-opt=" + options.after_ll.string(),
    };

    if ( options.inject_logging || options.profile_after_run )
    {
        command.push_back( "-mllvm" );
        command.push_back( "-dumper-pass-enable-logging-injection" );
    }

    command.insert( command.end(), options.extra_clang_args.begin(), options.extra_clang_args.end() );
    command.push_back( options.source.string() );
    command.push_back( "-" + options.opt_level );
    command.push_back( "-o" );
    command.push_back( options.binary_out.string() );

    execCommand( command );
}

void
generateDot( const Options& options, const fs::path& json_path, const fs::path& dot_path )
{
    requireExistingFile( json_path, "Serialized graph dump" );
    ensureParentDir( dot_path );

    execCommand(
        { options.converter_path.string(),
          "--input-json",
          json_path.string(),
          "--output-dot",
          dot_path.string() } );
}

void
generateSvg( const fs::path& dot_path, const fs::path& svg_path )
{
    requireExistingFile( dot_path, "Dot dump" );
    ensureParentDir( svg_path );

    execCommand( { "dot", "-Tsvg", dot_path.string(), "-o", svg_path.string() } );
}

void
mergeProfile( const Options& options )
{
    requireExistingFile( options.profile_tool_path, "Graph profile merge executable" );
    requireExistingFile( options.binary_out, "Compiled binary" );
    requireExistingFile( options.after_json, "After-opt serialized graph dump" );
    ensureParentDir( options.profile_json );

    if ( options.run_args.empty() )
    {
        std::cerr << "[compile_with_plugin] profiling run will execute the binary without runtime "
                     "arguments. If the program expects argv, pass them via --run-arg.\n";
    }

    std::vector<std::string> command = {
        options.profile_tool_path.string(),
        "--input-json",
        options.after_json.string(),
        "--binary",
        options.binary_out.string(),
        "--output-json",
        options.profile_json.string(),
    };

    for ( const auto& arg : options.run_args )
    {
        command.push_back( "--binary-arg" );
        command.push_back( arg );
    }

    execCommand( command );
}

} // namespace

int
main( int argc, char** argv )
{
    try
    {
        const auto options = parseOptions( argc, argv );
        validateOptions( options );

        execClang( options );
        generateDot( options, options.before_json, options.before_dot );
        generateDot( options, options.after_json, options.after_dot );

        if ( options.profile_after_run )
        {
            mergeProfile( options );
            generateDot( options, options.profile_json, options.profile_dot );
        }

        if ( !options.no_svg )
        {
            generateSvg( options.before_dot, options.before_svg );
            generateSvg( options.after_dot, options.after_svg );

            if ( options.profile_after_run )
            {
                generateSvg( options.profile_dot, options.profile_svg );
            }
        }
    } catch ( const std::exception& ex )
    {
        std::cerr << "[compile_with_plugin] " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
