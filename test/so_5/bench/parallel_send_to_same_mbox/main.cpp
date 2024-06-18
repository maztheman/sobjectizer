/*
 * A benchmark of parallel send from different agents to the same mbox.
 *
 * There is no a receiver for the messages. Benchmark shows only
 * the price of parallel access to the mbox.
 */

#include <iostream>
#include <iterator>
#include <numeric>
#include <cstdlib>

#include <so_5/all.hpp>

#include <test/3rd_party/various_helpers/benchmark_helpers.hpp>

#if defined(__clang__) && (__clang_major__ >= 16)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#endif

struct msg_send : public so_5::signal_t {};

struct msg_complete : public so_5::signal_t {};

class a_sender_t
	:	public so_5::agent_t
	{
	public :
		a_sender_t(
			so_5::environment_t & env,
			const so_5::mbox_t & mbox,
			unsigned int send_count )
			:	so_5::agent_t( env )
			,	m_mbox( mbox )
			,	m_send_count( send_count )
			{}

		void
		so_evt_start() override
			{
				for( unsigned int i = 0; i != m_send_count; ++i )
					so_5::send< msg_send >( m_mbox );

				so_5::send< msg_complete >( m_mbox );
			}

	private :
		const so_5::mbox_t m_mbox;

		unsigned int m_send_count;
	};

class a_shutdowner_t
	:	public so_5::agent_t
	{
	public :
		a_shutdowner_t(
			so_5::environment_t & env,
			const so_5::mbox_t & mbox,
			unsigned int sender_count )
			:	so_5::agent_t( env )
			,	m_sender_count( sender_count )
			{
				so_subscribe( mbox ).event(
					[this](mhood_t< msg_complete >) {
						m_sender_count -= 1;
						if( !m_sender_count )
							so_environment().stop();
					} );
			}

	private :
		unsigned int m_sender_count;
	};

void
init(
	so_5::environment_t & env,
	unsigned int agent_count,
	unsigned int send_count )
	{
		auto mbox = env.create_mbox();

		auto coop = env.make_coop(
				so_5::disp::active_obj::make_dispatcher(
						env, "active_obj" ).binder() );
		
		for( unsigned int i = 0; i != agent_count; ++i )
			coop->make_agent< a_sender_t >( mbox, send_count );

		coop->make_agent_with_binder< a_shutdowner_t >(
				so_5::make_default_disp_binder( env ),
				mbox, agent_count );

		env.register_coop( std::move( coop ) );
	}

void
print_usage()
{
	std::cout << "Usage: parallel_sent_to_same_mbox <agent_count> <send_count>\n\n"
			"<agent_count> and <send_count> must not be 0"
			<< std::endl;
}

struct cmd_line_exception : public std::invalid_argument
{
	cmd_line_exception( const char * what )
		:	std::invalid_argument( what )
	{}
};

int
main( int argc, char ** argv )
{
	try
	{
		auto ensure_args_validity = []( bool p, const char * msg ) {
			if( !p ) throw cmd_line_exception( msg );
		};
		ensure_args_validity( 3 == argc, "wrong number of arguments" );

		const unsigned int agent_count = static_cast< unsigned int >(std::atoi( argv[1] ));
		ensure_args_validity( agent_count != 0, "agent_count must not be 0" );

		const unsigned int send_count = static_cast< unsigned int >(std::atoi( argv[2] ));
		ensure_args_validity( send_count != 0, "send_count must not be 0" );

		benchmarker_t benchmark;
		benchmark.start();

		so_5::launch(
			[agent_count, send_count]( so_5::environment_t & env )
			{
				init( env, agent_count, send_count );
			} );

		benchmark.finish_and_show_stats(
				static_cast< unsigned long long >(agent_count) * send_count,
				"sends" );
	}
	catch( const cmd_line_exception & ex )
	{
		std::cerr << "Command line argument(s) error: " << ex.what()
				<< "\n\n" << std::flush;
		print_usage();
		return 1;
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 2;
	}

	return 0;
}

