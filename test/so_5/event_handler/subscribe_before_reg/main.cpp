/*
 * A simple test for subscription before agent registration.
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
	struct msg_1 : public so_5::signal_t {};
	struct msg_2 : public so_5::signal_t {};
	struct msg_3 : public so_5::signal_t {};

	const so_5::state_t st_1{ this, "st_1" };
	const so_5::state_t st_2{ this, "st_2" };
	const so_5::state_t st_3{ this, "st_3" };

public :
	a_test_t( context_t ctx )
		:	so_5::agent_t( ctx )
	{
		this >>= st_1;

		st_1.event( [this](mhood_t< msg_1 >) {
				st_2.activate();
				so_5::send< msg_2 >( *this );
			} );
		st_2.event( [this](mhood_t< msg_2 >) {
				st_3.activate();
				so_5::send< msg_3 >( *this );
			} );
		st_3.event( [this](mhood_t< msg_3 >) {
				so_deregister_agent_coop_normally();
			} );
	}

	virtual void
	so_evt_start() override
	{
		so_5::send< msg_1 >( *this );
	}
};

int
main()
{
	try
	{
		run_with_time_limit(
			[]()
			{
				so_5::launch( []( so_5::environment_t & env ) {
						env.introduce_coop( []( so_5::coop_t & coop ) {
								coop.make_agent< a_test_t >();
							} );
					} );
			},
			20,
			"subscription before registration test" );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

