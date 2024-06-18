/*
 * A test for simple stop_guard.
 */

#include <so_5/all.hpp>

#include <test/3rd_party/various_helpers/time_limited_execution.hpp>
#include <test/3rd_party/various_helpers/ensure.hpp>

#include <test/3rd_party/utest_helper/helper.hpp>

using namespace std;

class dummy_stop_guard_t
	: public so_5::stop_guard_t
	, public std::enable_shared_from_this<dummy_stop_guard_t>
{
public :
	dummy_stop_guard_t( so_5::environment_t & env )
		: m_env(env)
	{}

	std::shared_ptr< dummy_stop_guard_t >
	shptr() { return shared_from_this(); }

	virtual void
	stop() noexcept override
	{
		m_env.remove_stop_guard( shptr() );
	}

private :
	so_5::environment_t & m_env;
};

void make_stuff( so_5::environment_t & env )
{
	auto guard = std::make_shared< dummy_stop_guard_t >( std::ref(env) );
	env.setup_stop_guard( guard );

	class actor_t final : public so_5::agent_t {
	public :
		using so_5::agent_t::agent_t;

		void so_evt_start() override { so_environment().stop(); }
	};

	env.introduce_coop( [&]( so_5::coop_t & coop ) {
		coop.make_agent< actor_t >();
	} );
}

int
main()
{
	try
	{
		run_with_time_limit(
			[]() {
				so_5::launch(
					[&](so_5::environment_t & env) {
						make_stuff( env );
					},
					[](so_5::environment_params_t & params) {
						(void)params;
					} );
			},
			5 );
	}
	catch(const exception & ex)
	{
		cerr << "Error: " << ex.what() << endl;
		return 1;
	}

	return 0;
}

