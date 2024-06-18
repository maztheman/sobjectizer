/*
 * A test for receiving different messages.
 */

#include <iostream>
#include <exception>
#include <stdexcept>

#include <so_5/all.hpp>

struct test_message_1
	:
		public so_5::message_t
{
	test_message_1(): m_year_1( 2010 ), m_year_2( 2011 ) {}

	int m_year_1;
	int m_year_2;
};

struct test_message_2
	:
		public so_5::message_t
{
	test_message_2(): m_so( "SObjectizer" ), m_ver( "5" ) {}

	std::string m_so;
	std::string m_ver;
};

struct test_message_3
	:
		public so_5::message_t
{
	test_message_3(): m_where( "Gomel" ) {}

	std::string m_where;
};

class test_agent_t
	:
		public so_5::agent_t
{
		typedef so_5::agent_t base_type_t;

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
		evt_test_1(
			mhood_t< test_message_1 > msg );

		void
		evt_test_2(
			mhood_t< test_message_2 > msg );

		void
		evt_test_3(
			mhood_t< test_message_3 > msg );

	private:
		so_5::mbox_t m_test_mbox;
};

void
test_agent_t::so_define_agent()
{
	so_default_state().event( m_test_mbox, &test_agent_t::evt_test_1 );

	so_default_state().event( m_test_mbox, &test_agent_t::evt_test_2 );

	so_default_state().event( m_test_mbox, &test_agent_t::evt_test_3 );
}

void
test_agent_t::so_evt_start()
{
	so_5::send< test_message_1 >( m_test_mbox );

	so_5::send< test_message_2 >( m_test_mbox );

	so_5::send< test_message_3 >( m_test_mbox );
}

void
test_agent_t::evt_test_1(
	mhood_t< test_message_1 > msg )
{
	if( nullptr == msg.get() )
		throw std::runtime_error(
			"evt_test_1: 0 == msg.get()" );

	if( msg->m_year_1 != 2010 || msg->m_year_2 != 2011 )
	{
		throw std::runtime_error(
			"evt_test_1: "
			"msg->m_year_1 != 2010 || msg->m_year_2 != 2011" );
	}
}

void
test_agent_t::evt_test_2(
	mhood_t< test_message_2 > msg )
{
	if( nullptr == msg.get() )
		throw std::runtime_error(
			"evt_test_2: 0 == msg.get()" );

	if( msg->m_so !="SObjectizer" || msg->m_ver != "5" )
	{
		throw std::runtime_error(
			"evt_test_2: "
			"msg->m_so !=\"SObjectizer\" || msg->m_ver != \"5\"" );
	}
}

void
test_agent_t::evt_test_3(
	mhood_t< test_message_3 > msg )
{
	if( nullptr == msg.get() )
		throw std::runtime_error(
			"evt_test_3: 0 == msg.get()" );

	if( msg->m_where !="Gomel" )
	{
		throw std::runtime_error(
			"evt_test_3: "
			"msg->m_where !=\"Gomel\"" );
	}

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
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}

