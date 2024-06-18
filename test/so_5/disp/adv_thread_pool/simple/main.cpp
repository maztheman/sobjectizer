/*
 * A simple test for thread_pool dispatcher.
 */

#include <iostream>
#include <map>
#include <exception>
#include <stdexcept>
#include <cstdlib>
#include <thread>
#include <chrono>

#include <so_5/all.hpp>

#include <test/3rd_party/various_helpers/time_limited_execution.hpp>

#include "../for_each_lock_factory.hpp"

namespace atp_disp = so_5::disp::adv_thread_pool;

struct msg_hello : public so_5::signal_t {};

class a_test_t : public so_5::agent_t
{
	public:
		a_test_t(
			so_5::environment_t & env )
			:	so_5::agent_t( env )
		{}

		void
		so_define_agent() override
		{
			so_subscribe_self().event( &a_test_t::evt_hello );
		}

		void
		so_evt_start() override
		{
			so_5::send< msg_hello >( *this );
		}

		void
		evt_hello(mhood_t< msg_hello >)
		{
			so_environment().stop();
		}
};

void
do_test( atp_disp::queue_traits::lock_factory_t factory )
{
	run_with_time_limit(
		[&]()
		{
			so_5::launch(
				[&]( so_5::environment_t & env )
				{
					using namespace atp_disp;
					auto disp = make_dispatcher(
							env,
							"thread_pool",
							disp_params_t{}
								.thread_count( 4 )
								.set_queue_params( queue_traits::queue_params_t{}
									.lock_factory( factory ) ) );

					env.register_agent_as_coop(
							env.make_agent< a_test_t >(),
							disp.binder() );
				} );
		},
		20,
		"simple adv_thread_pool dispatcher test" );
}

int
main()
{
	try
	{
		for_each_lock_factory( []( atp_disp::queue_traits::lock_factory_t factory ) {
				do_test( factory );
			} );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

