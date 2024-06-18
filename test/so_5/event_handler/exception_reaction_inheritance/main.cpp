/*
 * A test for checking exception reaction inheritance from
 * coop, parent_coop and from so_environment.
 */

#include <iostream>
#include <stdexcept>

#include <so_5/all.hpp>

#include <test/3rd_party/various_helpers/time_limited_execution.hpp>

struct msg_test_signal : public so_5::signal_t {};

class a_test_t
	:	public so_5::agent_t
{
		typedef so_5::agent_t base_type_t;

	public :
		a_test_t(
			so_5::environment_t & env,
			const so_5::mbox_t & self_mbox )
			:	base_type_t( env )
			,	m_self_mbox( self_mbox )
		{}

		void
		so_define_agent() override
		{
			so_subscribe( m_self_mbox ).event( [](mhood_t< msg_test_signal >) {
					throw std::runtime_error( "Exception from a_test_t!" );
				} );
		}

	private :
		const so_5::mbox_t m_self_mbox;
};

class a_parent_t
	:	public so_5::agent_t
{
	public :
		a_parent_t(
			so_5::environment_t & env )
			:	so_5::agent_t( env )
		{}

		void
		so_evt_start() override
		{
			auto child = so_environment().make_coop( so_coop() );

			auto mbox = so_environment().create_mbox();
			child->make_agent< a_test_t >( mbox );

			so_environment().register_coop( std::move(child) );

			so_5::send< msg_test_signal >( mbox );
		}
};

void
init( so_5::environment_t & env )
{
	auto coop = env.make_coop();
	coop->make_agent< a_parent_t >();

	env.register_coop( std::move( coop ) );
}

int
main()
{
	try
	{
		run_with_time_limit( [] {
				so_5::launch( &init,
					[]( so_5::environment_params_t & params )
					{
						params.exception_reaction(
								so_5::shutdown_sobjectizer_on_exception );
					} );
			},
			10 );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

