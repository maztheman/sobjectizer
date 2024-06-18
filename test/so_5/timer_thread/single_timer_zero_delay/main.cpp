/*
 * Test of scheduling timer with zero milliseconds delay.
 */

#include <iostream>
#include <cstdlib>
#include <stdexcept>

#include <so_5/all.hpp>

struct msg_test : public so_5::message_t
{};

struct msg_do_resend : public so_5::signal_t
{};

struct msg_stop : public so_5::signal_t
{};

class a_test_t : public so_5::agent_t
{
		typedef so_5::agent_t base_type_t;

	public :
		a_test_t(
			so_5::environment_t & env,
			int & message_counter )
			:	base_type_t( env )
			,	m_message_counter( message_counter )
			,	m_mbox( env.create_mbox() )
		{
		}

		void
		so_define_agent() override
		{
			so_subscribe( m_mbox )
				.event( &a_test_t::evt_delayed_message );

			so_subscribe( m_mbox )
				.event( &a_test_t::evt_do_resend );

			so_subscribe( m_mbox )
				.event( &a_test_t::evt_stop );
		}

		void
		so_evt_start() override
		{
			so_5::send_delayed< msg_stop >( m_mbox,
					std::chrono::milliseconds(1000) );

			so_5::send_delayed< msg_test >( m_mbox,
					std::chrono::milliseconds(0) );

			so_5::send< msg_do_resend >( m_mbox );
		}

		void
		evt_delayed_message( mhood_t< msg_test > )
		{
			m_message_counter += 1;
		}

		void
		evt_do_resend( mhood_t< msg_do_resend > )
		{
			so_5::send_delayed< msg_test >( m_mbox,
					std::chrono::milliseconds(0) );
		}

		void
		evt_stop( mhood_t< msg_stop > )
		{
			so_environment().stop();
		}

	private :
		int & m_message_counter;

		so_5::mbox_t m_mbox;
};

struct test_env_t
{
	int m_message_counter;

	test_env_t()
		:	m_message_counter( 0 )
	{}

	void
	init( so_5::environment_t & env )
	{
		env.register_agent_as_coop(
				env.make_agent< a_test_t >( m_message_counter ) );
	}
};

int
main()
{
	try
	{
		test_env_t test_env;

		so_5::launch(
			[&]( so_5::environment_t & env )
			{
				test_env.init( env );
			} );

		if( 2 != test_env.m_message_counter )
		{
			std::cerr << "expected and actual message_counter value mismatch, "
					"actual value: " << test_env.m_message_counter << std::endl;
			std::abort();
		}

		return 0;
	}
	catch( const std::exception & x )
	{
		std::cerr << "Exception caught: " << x.what() << std::endl;
	}

	return 2;
}


