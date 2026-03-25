#include <array>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <unistd.h>
#include <vector>

#include <sys/wait.h>

#include <argparse/argparse.hpp>

#include "ir_graph/ir_graph_profile.h"
#include "ir_graph/ir_graph_serde.h"

namespace {

struct BinaryRunResult
{
    int         exit_code = 0;
    std::string stdout_output;
};

class FileDescriptor {
  public:
    FileDescriptor() = default;

    explicit FileDescriptor( int value ) : value_( value ) {}

    FileDescriptor( const FileDescriptor& )            = delete;
    FileDescriptor& operator=( const FileDescriptor& ) = delete;

    FileDescriptor( FileDescriptor&& other ) noexcept : value_( other.release() ) {}

    FileDescriptor&
    operator=( FileDescriptor&& other ) noexcept
    {
        if ( this != &other )
        {
            reset( other.release() );
        }

        return *this;
    }

    ~FileDescriptor()
    {
        reset();
    }

    int
    get() const
    {
        return value_;
    }

    int
    release()
    {
        const int released = value_;
        value_             = -1;
        return released;
    }

    void
    reset( int new_value = -1 )
    {
        if ( value_ >= 0 )
        {
            ::close( value_ );
        }

        value_ = new_value;
    }

  private:
    int value_ = -1;
};

void
configureProgram( argparse::ArgumentParser& program )
{
    program.add_description(
        "Run an instrumented binary, collect [ir-log] events, and merge them into an IR graph JSON."
    );
    program.add_argument( "--input-json" ).required().help( "Path to the static graph JSON." );
    program.add_argument( "--binary" ).required().help( "Path to the instrumented binary." );
    program.add_argument( "--output-json" ).required().help( "Path to the merged output JSON." );
    program.add_argument( "--binary-arg" )
        .default_value( std::vector<std::string>{} )
        .append()
        .help( "Argument passed to the profiled binary. Can be specified multiple times." );
}

void
throwSystemError( const std::string& action )
{
    throw std::system_error( errno, std::generic_category(), action );
}

BinaryRunResult
runBinaryAndCaptureStdout( const std::string& binary, const std::vector<std::string>& binary_args )
{
    std::array<int, 2> stdout_pipe = { -1, -1 };
    if ( ::pipe( stdout_pipe.data() ) != 0 )
    {
        throwSystemError( "pipe" );
    }

    FileDescriptor stdout_read( stdout_pipe[0] );
    FileDescriptor stdout_write( stdout_pipe[1] );

    const pid_t child_pid = ::fork();
    if ( child_pid < 0 )
    {
        throwSystemError( "fork" );
    }

    if ( child_pid == 0 )
    {
        if ( ::dup2( stdout_write.get(), STDOUT_FILENO ) < 0 )
        {
            std::perror( "dup2" );
            std::_Exit( 127 );
        }

        stdout_read.reset();
        stdout_write.reset();

        std::vector<char*> argv;
        argv.reserve( binary_args.size() + 2 );
        argv.push_back( const_cast<char*>( binary.c_str() ) );
        for ( const auto& argument : binary_args )
        {
            argv.push_back( const_cast<char*>( argument.c_str() ) );
        }
        argv.push_back( nullptr );

        ::execvp( binary.c_str(), argv.data() );
        std::perror( "execvp" );
        std::_Exit( 127 );
    }

    stdout_write.reset();

    BinaryRunResult result;
    std::array<char, 4096> buffer{};

    while ( true )
    {
        const auto bytes_read = ::read( stdout_read.get(), buffer.data(), buffer.size() );
        if ( bytes_read == 0 )
        {
            break;
        }

        if ( bytes_read < 0 )
        {
            if ( errno == EINTR )
            {
                continue;
            }

            throwSystemError( "read" );
        }

        result.stdout_output.append( buffer.data(), static_cast<std::size_t>( bytes_read ) );
    }

    stdout_read.reset();

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
        result.exit_code = WEXITSTATUS( wait_status );
        return result;
    }

    if ( WIFSIGNALED( wait_status ) )
    {
        throw std::runtime_error( "binary terminated by signal " +
                                  std::to_string( WTERMSIG( wait_status ) ) );
    }

    throw std::runtime_error( "binary terminated unexpectedly" );
}

bool
startsWith( std::string_view text, std::string_view prefix )
{
    return text.size() >= prefix.size() && text.substr( 0, prefix.size() ) == prefix;
}

void
forwardProgramStdout( std::string_view stdout_output )
{
    constexpr std::string_view kLogPrefix = "[ir-log] ";

    std::size_t line_begin = 0;
    while ( line_begin <= stdout_output.size() )
    {
        const auto line_end = stdout_output.find( '\n', line_begin );
        const bool has_newline = line_end != std::string_view::npos;
        const auto line = stdout_output.substr(
            line_begin,
            has_newline ? line_end - line_begin : stdout_output.size() - line_begin );

        if ( !startsWith( line, kLogPrefix ) && !line.empty() )
        {
            std::cout << line;
            if ( has_newline )
            {
                std::cout << '\n';
            }
        }

        if ( !has_newline )
        {
            break;
        }

        line_begin = line_end + 1;
    }
}

} // namespace

int
main( int argc, char** argv )
{
    try
    {
        argparse::ArgumentParser program( "ir_graph_profile_merge" );
        configureProgram( program );
        program.parse_args( argc, argv );

        const auto input_json  = program.get<std::string>( "--input-json" );
        const auto binary      = program.get<std::string>( "--binary" );
        const auto output_json = program.get<std::string>( "--output-json" );
        const auto binary_args = program.get<std::vector<std::string>>( "--binary-arg" );

        std::ifstream input( input_json );
        if ( !input )
        {
            throw std::runtime_error( "unable to open input json: " + input_json );
        }

        auto graph = ir_graph::deserialize( input );

        const auto run_result = runBinaryAndCaptureStdout( binary, binary_args );
        forwardProgramStdout( run_result.stdout_output );

        if ( run_result.exit_code != 0 )
        {
            throw std::runtime_error( "binary exited with code " +
                                      std::to_string( run_result.exit_code ) );
        }

        const auto profile = ir_graph::parseRuntimeProfile( run_result.stdout_output );
        if ( profile.empty() )
        {
            throw std::runtime_error( "no runtime profiling events were captured from binary" );
        }

        ir_graph::applyRuntimeProfile( graph, profile );

        std::ofstream output( output_json );
        if ( !output )
        {
            throw std::runtime_error( "unable to open output json: " + output_json );
        }

        output << ir_graph::serialize( graph );
    } catch ( const std::exception& ex )
    {
        std::cerr << "[ir-graph-profile-merge] " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
