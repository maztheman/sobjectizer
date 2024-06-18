/*
 * A simple test for getting count of named mboxes
 * from run-time monitoring messages.
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

class a_test_t : public so_5::agent_t
	{
	public :
		a_test_t( context_t ctx )
			:	so_5::agent_t( ctx )
			,	m_m1( so_environment().create_mbox( "m1" ) )
			,	m_m2( so_environment().create_mbox( "m2" ) )
			,	m_m3( so_environment().create_mbox( "m3" ) )
			{}

		virtual void
		so_define_agent() override
			{
				so_default_state().event(
						so_environment().stats_controller().mbox(),
						&a_test_t::evt_monitor_quantity );
			}

		virtual void
		so_evt_start() override
			{
				so_environment().stats_controller().turn_on();
			}

	private :
		const so_5::mbox_t m_m1;
		const so_5::mbox_t m_m2;
		const so_5::mbox_t m_m3;

		unsigned int m_actual_values = { 0 };

		void
		evt_monitor_quantity(
			const so_5::stats::messages::quantity< std::size_t > & evt )
			{
				namespace stats = so_5::stats;

				std::cout << evt.m_prefix
						<< evt.m_suffix
						<< ": " << evt.m_value << std::endl;

				if( stats::prefixes::mbox_repository() == evt.m_prefix )
					{
						// Check as_string_view method for prefix_t.
						if( stats::prefixes::mbox_repository().as_string_view()
								!= evt.m_prefix.c_str() )
							{
								throw std::runtime_error(
										"as_string_view() for prefix_t doesn't work" );
							}

						if( stats::suffixes::named_mbox_count() == evt.m_suffix )
							{
								// Check as_string_view method for suffix_t.
								if( stats::suffixes::named_mbox_count().as_string_view()
										!= evt.m_suffix.c_str() )
									{
										throw std::runtime_error(
												"as_string_view() for suffix_t doesn't work" );
									}

								if( 3 != evt.m_value )
									throw std::runtime_error( "unexpected count of "
											"named mboxes: " +
											std::to_string( evt.m_value ) );
								else
									++m_actual_values;
							}
					}

				if( 1 == m_actual_values )
					so_deregister_agent_coop_normally();
			}
	};

void
init( so_5::environment_t & env )
	{
		env.register_agent_as_coop(
				env.make_agent< a_test_t >() );
	}

int
main()
{
	try
	{
		run_with_time_limit(
			[]()
			{
				so_5::launch( &init );
			},
			20,
			"simple named mboxes count monitoring test" );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

