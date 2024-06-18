/*
 * A test for checking so_5::ignore_exception behaviour.
 */

#include <iostream>
#include <stdexcept>

#include <so_5/all.hpp>

struct msg_test_signal : public so_5::signal_t {};

class a_test_t
	:	public so_5::agent_t
{
		typedef so_5::agent_t base_type_t;

	public :
		a_test_t(
			so_5::environment_t & env )
			:	base_type_t( env )
			,	m_self_mbox( env.create_mbox() )
			,	m_counter( 0 )
			,	m_max_attempts( 3 )
		{}

		void
		so_define_agent() override
		{
			so_subscribe( m_self_mbox ).event( &a_test_t::evt_signal );
		}

		void
		so_evt_start() override
		{
			so_5::send< msg_test_signal >( m_self_mbox );
		}

		void
		evt_signal( mhood_t< msg_test_signal > )
		{
			if( m_counter < m_max_attempts )
			{
				m_counter += 1;
				so_5::send< msg_test_signal >( m_self_mbox );

				throw std::runtime_error( "Another exception from evt_signal" );
			}
			else
				so_environment().stop();
		}

		so_5::exception_reaction_t
		so_exception_reaction() const noexcept override
		{
			return so_5::ignore_exception;
		}

	private :
		const so_5::mbox_t m_self_mbox;

		int m_counter;
		const int m_max_attempts;
};

void
init( so_5::environment_t & env )
{
	env.register_agent_as_coop( env.make_agent< a_test_t >() );
}

int
main()
{
	try
	{
		so_5::launch( &init );
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

