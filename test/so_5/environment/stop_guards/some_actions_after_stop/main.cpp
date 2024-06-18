/*
 * A test for simple stop_guard.
 */

#include <so_5/all.hpp>

#include <test/3rd_party/various_helpers/time_limited_execution.hpp>
#include <test/3rd_party/various_helpers/ensure.hpp>

#include <test/3rd_party/utest_helper/helper.hpp>

using namespace std;
using namespace std::chrono_literals;

class empty_stop_guard_t
	: public so_5::stop_guard_t
	, public std::enable_shared_from_this<empty_stop_guard_t>
{
public :
	empty_stop_guard_t()
	{}

	virtual void
	stop() noexcept override
	{}
};

void make_stuff(
	so_5::environment_t & env,
	so_5::outliving_reference_t< bool > step_3_completed )
{
	auto guard = std::make_shared< empty_stop_guard_t >();
	env.setup_stop_guard( guard );

	class actor_t final : public so_5::agent_t
	{
		so_5::outliving_reference_t< bool > m_step_3_completed;
		so_5::stop_guard_shptr_t m_guard;

		struct step_1 final : public so_5::signal_t {};
		struct step_2 final : public so_5::signal_t {};
		struct step_3 final : public so_5::signal_t {};

	public :
		actor_t(
			context_t ctx,
			so_5::outliving_reference_t< bool > step_3_completed,
			so_5::stop_guard_shptr_t guard )
			:	so_5::agent_t{ std::move(ctx) }
			,	m_step_3_completed{ step_3_completed }
			,	m_guard{ std::move(guard) }
		{
			so_subscribe_self()
				.event( [this]( so_5::mhood_t<step_1> ) {
					so_5::send_delayed< step_2 >( *this, 50ms );
				} )
				.event( [this]( so_5::mhood_t<step_2> ) {
					so_5::send_delayed< step_3 >( *this, 50ms );
				} )
				.event( [this]( so_5::mhood_t<step_3> ) {
					m_step_3_completed.get() = true;
					so_environment().remove_stop_guard( m_guard );
				} );
		}

		void so_evt_start() override
		{
			so_environment().stop();
			so_5::send_delayed< step_1 >( *this, 50ms );
		}
	};

	env.introduce_coop( [&]( so_5::coop_t & coop ) {
		coop.make_agent< actor_t >( step_3_completed, guard );
	} );
}

int
main()
{
	try
	{
		run_with_time_limit(
			[]() {
				bool step_3_completed = false;
				so_5::launch(
					[&](so_5::environment_t & env) {
						make_stuff( env, so_5::outliving_mutable( step_3_completed ) );
					},
					[](so_5::environment_params_t & params) {
						(void)params;
					} );

				if( !step_3_completed )
					throw std::runtime_error( "step 3 is not completed" );
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

