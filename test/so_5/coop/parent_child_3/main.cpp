/*
 * A test for sequence of child cooperation deregistration.
 */

#include <iostream>
#include <sstream>

#include <so_5/all.hpp>

#include <test/3rd_party/various_helpers/time_limited_execution.hpp>

struct msg_child_started : public so_5::signal_t {};

void
create_and_register_agent(
	const so_5::coop_handle_t & parent,
	so_5::environment_t & env,
	int ordinal,
	int max_deep );

class a_test_t : public so_5::agent_t
{
		typedef so_5::agent_t base_type_t;

	public :
		a_test_t(
			so_5::environment_t & env,
			int ordinal,
			int max_deep )
			:	base_type_t( env )
			,	m_ordinal( ordinal )
			,	m_max_deep( max_deep )
			,	m_self_mbox(
					env.create_mbox( mbox_name( ordinal ) ) )
		{
		}

		void
		so_define_agent() override
		{
			so_subscribe( m_self_mbox )
				.event( &a_test_t::evt_child_started );
		}

		void
		so_evt_start() override
		{
			if( m_ordinal != m_max_deep )
				create_and_register_agent(
						so_coop(),
						so_environment(),
						m_ordinal + 1,
						m_max_deep );
			else
				notify_parent();
		}

		void
		evt_child_started(
			mhood_t< msg_child_started > )
		{
			if( m_ordinal )
				notify_parent();
			else
				so_environment().stop();
		}

	private :
		const int m_ordinal;

		const int m_max_deep;

		so_5::mbox_t m_self_mbox;

		static std::string
		mbox_name( int ordinal )
		{
			std::ostringstream s;
			s << "agent_" << ordinal;
			return s.str();
		}

		void
		notify_parent()
		{
			so_5::send< msg_child_started >(
					so_environment().create_mbox( mbox_name( m_ordinal - 1 ) ) );
		}
};

std::string
create_coop_name( int ordinal )
{
	std::ostringstream s;
	s << "coop_" << ordinal;
	return s.str();
}

void
create_and_register_agent(
	const so_5::coop_handle_t & parent,
	so_5::environment_t & env,
	int ordinal,
	int max_deep )
{
	auto coop = env.make_coop( parent );

	coop->make_agent< a_test_t >( ordinal, max_deep );

	env.register_coop( std::move( coop ) );
}

class a_test_starter_t : public so_5::agent_t
{
	typedef so_5::agent_t base_type_t;

	public :
		a_test_starter_t( so_5::environment_t & env )
			:	base_type_t( env )
		{}

		void
		so_evt_start() override
		{
			create_and_register_agent( so_coop(), so_environment(), 0, 5 );
		}
};

// We just know that the first coop will have this ID in the current
// version of SO-5.
constexpr so_5::coop_id_t starter_coop_id = 2u;

struct init_deinit_data_t
{
	std::vector< so_5::coop_id_t > m_init_sequence;
	std::vector< so_5::coop_id_t > m_deinit_sequence;
};

class test_coop_listener_t
	:	public so_5::coop_listener_t
{
	public :
		test_coop_listener_t( init_deinit_data_t & data )
			:	m_data( data )
			,	m_active_coops( 0 )
		{}

		void
		on_registered(
			so_5::environment_t &,
			const so_5::coop_handle_t & coop ) noexcept override
		{
			std::lock_guard< std::mutex > lock{ m_lock };

			std::cout << "registered: " << coop << std::endl;

			if( starter_coop_id != coop.id() )
			{
				m_data.m_init_sequence.push_back( coop.id() );

				++m_active_coops;
			}
		}

		void
		on_deregistered(
			so_5::environment_t & env,
			const so_5::coop_handle_t & coop,
			const so_5::coop_dereg_reason_t & reason ) noexcept override
		{
			bool need_stop = false;
			{
				std::lock_guard< std::mutex > lock{ m_lock };

				std::cout << "deregistered: " << coop
						<< ", reason: " << reason.reason() << std::endl;

				if( starter_coop_id != coop.id() )
				{
					m_data.m_deinit_sequence.insert(
							m_data.m_deinit_sequence.begin(),
							coop.id() );

					--m_active_coops;

					if( !m_active_coops )
						need_stop = true;
				}
			}

			if( need_stop )
				env.stop();
		}

		static so_5::coop_listener_unique_ptr_t
		make( init_deinit_data_t & data )
		{
			return so_5::coop_listener_unique_ptr_t(
					new test_coop_listener_t( data ) );
		}

	private :
		std::mutex m_lock;

		init_deinit_data_t & m_data;

		int m_active_coops;
};

std::string
sequence_to_string( const std::vector< so_5::coop_id_t > & s )
{
	std::ostringstream ss;
	for( auto i = s.begin(); i != s.end(); ++i )
	{
		if( i != s.begin() )
			ss << ", ";
		ss << *i;
	}

	return ss.str();
}

class test_env_t
{
	public :
		void
		init( so_5::environment_t & env )
		{
			env.register_agent_as_coop(
					env.make_agent< a_test_starter_t >() );
		}

		so_5::coop_listener_unique_ptr_t
		make_listener()
		{
			return test_coop_listener_t::make( m_data );
		}

		void
		check_result() const
		{
			if( m_data.m_init_sequence != m_data.m_deinit_sequence )
				throw std::runtime_error( "Wrong deinit sequence: init_seq: " +
						sequence_to_string( m_data.m_init_sequence ) +
						", deinit_seq: " +
						sequence_to_string( m_data.m_deinit_sequence ) );
		}

	private :
		init_deinit_data_t m_data;
};

int
main()
{
	run_with_time_limit( [] {
			test_env_t test_env;
			so_5::launch(
					[&test_env]( so_5::environment_t & env )
					{
						test_env.init( env );
					},
					[&test_env]( so_5::environment_params_t & params )
					{
						params.coop_listener( test_env.make_listener() );
						params.disable_autoshutdown();
					} );

			test_env.check_result();
		},
		60 );

	return 0;
}
