/*
 * A test for adv_thread_pool dispatcher: subscription
 * management methods must throw exception in thread safe event handlers.
 */

#include <iostream>
#include <set>
#include <vector>
#include <exception>
#include <stdexcept>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <sstream>

#include <so_5/all.hpp>

#include <test/3rd_party/various_helpers/time_limited_execution.hpp>

#include "../for_each_lock_factory.hpp"

namespace atp_disp = so_5::disp::adv_thread_pool;

struct msg_shutdown : public so_5::signal_t {};

struct msg_safe_signal : public so_5::signal_t {};

struct msg_unsafe_signal : public so_5::signal_t {};

void
expect_throw(
	const std::string & description,
	int error_code,
	std::function< void() > f )
{
	bool exception_thrown = true;
	try
	{
		f();
		exception_thrown = false;
	}
	catch( const so_5::exception_t & x )
	{
		if( error_code == x.error_code() )
			std::cout << description << ", expected exception: "
				<< x.what() << std::endl;
		else
		{
			std::ostringstream ss;
			ss << description << " expect exception with error_code("
				<< error_code << ", but has: " << x.what();
			throw std::runtime_error( ss.str() );
		}
	}

	if( !exception_thrown )
		throw std::runtime_error(
				"an exception expected for " + description );
}

class a_test_t : public so_5::agent_t
{
	public:
		a_test_t(
			so_5::environment_t & env )
			:	so_5::agent_t( env )
		{
		}

		void
		so_define_agent() override
		{
			so_change_state( st_safe );

			so_subscribe_self().in( st_unsafe )
				.event( &a_test_t::evt_shutdown );

			so_subscribe_self().in( st_safe )
				.event( &a_test_t::evt_safe_signal, so_5::thread_safe )
				.event( &a_test_t::evt_unsafe_signal );
		}

		void
		so_evt_start() override
		{
			so_5::send< msg_safe_signal >( *this );

			so_5::send< msg_unsafe_signal >( *this );

			so_5::send< msg_shutdown >( *this );
		}

		void
		evt_shutdown(mhood_t< msg_shutdown >)
		{
			so_environment().stop();
		}

		void
		evt_safe_signal(mhood_t< msg_safe_signal >)
		{
			expect_throw( "so_drop_subscription",
					so_5::rc_operation_enabled_only_on_agent_working_thread,
					[this]() {
						so_drop_subscription(
								so_direct_mbox(),
								st_safe,
								&a_test_t::evt_safe_signal );
					} );

			expect_throw( "so_drop_subscription_for_all_states",
					so_5::rc_operation_enabled_only_on_agent_working_thread,
					[this]() {
						so_drop_subscription_for_all_states(
								so_direct_mbox(),
								&a_test_t::evt_safe_signal );
					} );

			expect_throw( "so_subscribe",
					so_5::rc_operation_enabled_only_on_agent_working_thread,
					[this]() {
						so_subscribe_self().event(
								&a_test_t::evt_safe_signal );
					} );
		}

		void
		evt_unsafe_signal(mhood_t< msg_unsafe_signal >)
		{
			so_change_state( st_unsafe );
		}

	private :
		const so_5::state_t st_safe{ this, "safe" };
		const so_5::state_t st_unsafe{ this, "unsafe" };
};

void
run_sobjectizer( atp_disp::queue_traits::lock_factory_t factory )
{
	so_5::launch(
		[&]( so_5::environment_t & env )
		{
			using namespace atp_disp;

			env.register_agent_as_coop(
					env.make_agent< a_test_t >(),
					make_dispatcher(
							env,
							"thread_pool",
							disp_params_t{}.set_queue_params(
									queue_traits::queue_params_t{}
											.lock_factory( factory ) ) )
					.binder() );
		} );
}

int
main()
{
	try
	{
		for_each_lock_factory( []( atp_disp::queue_traits::lock_factory_t factory ) {
			run_with_time_limit(
				[&]()
				{
					run_sobjectizer( factory );
				},
				20,
				"subscr_in_safe test" );
		} );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

