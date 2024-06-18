/*
 * A simple test for getting stats about work thread activity.
 */

#include <iostream>
#include <map>
#include <exception>
#include <stdexcept>
#include <cstdlib>
#include <thread>
#include <chrono>

#include <so_5/all.hpp>

#include <test/3rd_party/various_helpers/ensure.hpp>
#include <test/3rd_party/various_helpers/time_limited_execution.hpp>

class a_test_t : public so_5::agent_t
	{
	public :
		a_test_t( context_t ctx )
			:	so_5::agent_t( ctx )
			{}

		virtual void
		so_define_agent() override
			{
				so_default_state().event(
						so_environment().stats_controller().mbox(),
						&a_test_t::evt_monitor_activity );
			}

		virtual void
		so_evt_start() override
			{
				create_child_coops();

				so_environment().stats_controller().turn_on();
			}

	private :
		unsigned int m_actual_values = { 0 };

		void
		evt_monitor_activity(
			const so_5::stats::messages::work_thread_activity & evt )
			{
				namespace stats = so_5::stats;

				std::cout << evt.m_prefix << evt.m_suffix
						<< " [" << evt.m_thread_id << "] ->\n"
						<< "  working: " << evt.m_stats.m_working_stats << "\n"
						<< "  waiting: " << evt.m_stats.m_waiting_stats << std::endl;

				++m_actual_values;

				if( 4 == m_actual_values )
					so_environment().stop();
			}

		void
		create_child_coops()
			{
				class empty_actor_t final : public so_5::agent_t
				{
				public :
					using so_5::agent_t::agent_t;
				};

				for( int i = 0; i != 10; ++i )
					{
						auto coop = so_5::create_child_coop( *this );
						coop->make_agent< empty_actor_t >();

						so_environment().register_coop( std::move( coop ) );
					}
			}
	};

void
init( so_5::environment_t & env )
	{
		ensure_or_die(
				so_5::work_thread_activity_tracking_t::on ==
						env.work_thread_activity_tracking(),
				"work_thread_activity_tracking should be 'on' at this point" );

		env.introduce_coop( []( so_5::coop_t & coop ) {
			coop.make_agent< a_test_t >();

			class actor_t final : public so_5::agent_t
			{
				struct next final : public so_5::signal_t {};
			public :
				using so_5::agent_t::agent_t;

				void so_evt_start() override
				{
					so_subscribe_self().event( [this](mhood_t<next>) {
							so_5::send< next >( *this );
							std::this_thread::sleep_for( std::chrono::seconds(1) );
						} );

					so_5::send< next >( *this );
				}
			};

			coop.make_agent_with_binder< actor_t >(
				so_5::disp::one_thread::make_dispatcher(
						coop.environment(), "busy" ).binder() );
		} );
	}

int
main()
{
	try
	{
		run_with_time_limit(
			[]()
			{
				so_5::wrapped_env_t env{
					&init,
					[]( so_5::environment_params_t & params ) {
						params.turn_work_thread_activity_tracking_on();
					}
				};

				std::cout << "Waiting for the completion..." << std::endl;
				env.join();
			},
			20 );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

