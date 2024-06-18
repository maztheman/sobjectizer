/*
 * A test for simple_mtsafe_st_env_infastructure with one simple agent
 * and periodic message.
 */

#include <so_5/all.hpp>

#include <test/3rd_party/various_helpers/time_limited_execution.hpp>

#include <test/3rd_party/utest_helper/helper.hpp>

using namespace std;

class a_test_t final : public so_5::agent_t
{
public :
	struct tick : public so_5::signal_t {};

	a_test_t( context_t ctx ) : so_5::agent_t( std::move(ctx) )
	{
		so_subscribe_self().event( [this](mhood_t< tick >) {
				so_deregister_agent_coop_normally();
			} );
	}
};

int
main()
{
	try
	{
		run_with_time_limit(
			[]() {
				thread outside_thread;

				so_5::launch(
					[&]( so_5::environment_t & env ) {
						so_5::mbox_t test_mbox = env.introduce_coop(
							[&]( so_5::coop_t & coop ) {
								return coop.make_agent< a_test_t >()->so_direct_mbox();
							} );

						outside_thread = thread( [test_mbox] {
							this_thread::sleep_for( chrono::milliseconds( 350 ) );
							so_5::send_delayed< a_test_t::tick >(
									test_mbox,
									chrono::milliseconds(100) );
						} );
					},
					[]( so_5::environment_params_t & params ) {
						params.infrastructure_factory(
								so_5::env_infrastructures::simple_mtsafe::factory() );
					} );

				outside_thread.join();
			},
			5,
			"simple agent with delayed message from outside" );
	}
	catch( const exception & ex )
	{
		cerr << "Error: " << ex.what() << endl;
		return 1;
	}

	return 0;
}

