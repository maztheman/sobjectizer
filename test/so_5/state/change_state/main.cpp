/*
 * A test for messages handling in difference states.
 */

#include <iostream>
#include <exception>
#include <stdexcept>

#include <so_5/all.hpp>

struct test_message : public so_5::signal_t {};

class test_agent_t
	:
		public so_5::agent_t
{
		typedef so_5::agent_t base_type_t;

		const so_5::state_t m_first_state{ this };
		const so_5::state_t m_second_state{ this };
		const so_5::state_t m_third_state{ this };

	public:

		test_agent_t(
			so_5::environment_t & env )
			:
				base_type_t( env ),
				m_test_mbox( so_environment().create_mbox() )
		{}

		~test_agent_t() override
		{}

		void
		so_define_agent() override;

		void
		so_evt_start() override;

		void
		evt_in_state_default(
			mhood_t< test_message > );

		void
		evt_in_state_1(
			mhood_t< test_message > );

		void
		evt_in_state_2(
			mhood_t< test_message > );

		void
		evt_in_state_3(
			mhood_t< test_message > );

		// Count for event handlers calls.
		static int m_handler_in_state_default_calls;
		static int m_handler_in_state_1_calls;
		static int m_handler_in_state_2_calls;
		static int m_handler_in_state_3_calls;

	private:
		so_5::mbox_t m_test_mbox;
};

int test_agent_t::m_handler_in_state_default_calls = 0;
int test_agent_t::m_handler_in_state_1_calls = 0;
int test_agent_t::m_handler_in_state_2_calls = 0;
int test_agent_t::m_handler_in_state_3_calls = 0;

void
test_agent_t::so_define_agent()
{
	// Subscribe to message in default state...
	so_subscribe( m_test_mbox )
			.event(
				&test_agent_t::evt_in_state_default );

	// ...in first state...
	so_subscribe( m_test_mbox )
		.in( m_first_state )
			.event(
				&test_agent_t::evt_in_state_1 );

	// ...in second state...
	so_subscribe( m_test_mbox )
		.in( m_second_state )
			.event(
				&test_agent_t::evt_in_state_2 );

	// ...in third state.
	so_subscribe( m_test_mbox )
		.in( m_third_state )
			.event(
				&test_agent_t::evt_in_state_3 );
}

void
test_agent_t::so_evt_start()
{
	so_5::send< test_message >( m_test_mbox );
}

void
test_agent_t::evt_in_state_default(
	mhood_t< test_message > )
{
	++m_handler_in_state_default_calls;
	so_5::send< test_message >( m_test_mbox );

	// Change state after message has been sent.
	so_change_state( m_first_state );
}

void
test_agent_t::evt_in_state_1(
	mhood_t< test_message > )
{
	++m_handler_in_state_1_calls;

	// Change state before message will be sent.
	so_change_state( m_second_state );

	so_5::send< test_message >( m_test_mbox );
}

void
test_agent_t::evt_in_state_2(
	mhood_t< test_message > )
{
	++m_handler_in_state_2_calls;

	// Change state before message will be sent.
	so_change_state( m_third_state );

	so_5::send< test_message >( m_test_mbox );
}

void
test_agent_t::evt_in_state_3(
	mhood_t< test_message > )
{
	++m_handler_in_state_3_calls;

	std::cout << "Stop\n";

	// Shutting down.
	so_environment().stop();
}

void
init( so_5::environment_t & env )
{
	env.register_agent_as_coop( env.make_agent< test_agent_t >() );
}

int
main()
{
	try
	{
		so_5::launch( &init );

		if( test_agent_t::m_handler_in_state_default_calls != 1 ||
			test_agent_t::m_handler_in_state_1_calls != 1 ||
			test_agent_t::m_handler_in_state_2_calls != 1 ||
			test_agent_t::m_handler_in_state_3_calls != 1 )
		{
			std::cerr
				<< "test_agent_t::m_handler_in_state_default_calls = "
				<< test_agent_t::m_handler_in_state_default_calls << "\n"
				<< "test_agent_t::m_handler_in_state_1_calls = "
				<< test_agent_t::m_handler_in_state_1_calls << "\n"
				<< "test_agent_t::m_handler_in_state_2_calls = "
				<< test_agent_t::m_handler_in_state_2_calls << "\n"
				<< "test_agent_t::m_handler_in_state_3_calls = "
				<< test_agent_t::m_handler_in_state_3_calls << "\n";

			throw std::runtime_error( "handler calls count error" );
		}

	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
