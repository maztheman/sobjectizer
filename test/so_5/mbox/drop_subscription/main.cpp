/*
 * A test for so_drop_subscription methods.
 */

#include <iostream>
#include <sstream>

#include <so_5/all.hpp>

#include <test/3rd_party/various_helpers/time_limited_execution.hpp>

class test_mbox_t : public so_5::abstract_message_box_t
	{
	private :
		const so_5::mbox_t m_actual_mbox;

		unsigned int m_subscriptions = 0;
		unsigned int m_unsubscriptions = 0;

	public :
		test_mbox_t( so_5::environment_t & env )
			:	m_actual_mbox( env.create_mbox() )
			{
			}

		~test_mbox_t() override
			{
				if( m_subscriptions != m_unsubscriptions )
					{
						std::cerr << "subscriptions(" << m_subscriptions
								<< ") != unsubscriptions(" << m_unsubscriptions
								<< "). Test aborted!" << std::endl;

						std::abort();
					}
			}

		virtual so_5::mbox_id_t
		id() const override
			{
				return m_actual_mbox->id();
			}

		virtual void
		do_deliver_message(
			so_5::message_delivery_mode_t delivery_mode,
			const std::type_index & type_index,
			const so_5::message_ref_t & message_ref,
			unsigned int redirection_deep ) override
			{
				m_actual_mbox->do_deliver_message(
						delivery_mode,
						type_index,
						message_ref,
						redirection_deep );
			}

		virtual void
		subscribe_event_handler(
			const std::type_index & type_index,
			so_5::abstract_message_sink_t & subscriber ) override
			{
				++m_subscriptions;
				m_actual_mbox->subscribe_event_handler( type_index, subscriber );
			}

		virtual void
		unsubscribe_event_handler(
			const std::type_index & type_index,
			so_5::abstract_message_sink_t & subscriber ) noexcept override
			{
				++m_unsubscriptions;
				m_actual_mbox->unsubscribe_event_handler( type_index, subscriber );
			}

		virtual std::string
		query_name() const override { return m_actual_mbox->query_name(); }

		virtual so_5::mbox_type_t
		type() const override
			{
				return m_actual_mbox->type();
			}

		virtual void
		set_delivery_filter(
			const std::type_index & msg_type,
			const so_5::delivery_filter_t & filter,
			so_5::abstract_message_sink_t & subscriber ) override
			{
				m_actual_mbox->set_delivery_filter( msg_type, filter, subscriber );
			}

		virtual void
		drop_delivery_filter(
			const std::type_index & msg_type,
			so_5::abstract_message_sink_t & subscriber ) noexcept override
			{
				m_actual_mbox->drop_delivery_filter( msg_type, subscriber );
			}

		so_5::environment_t &
		environment() const noexcept override
			{
				return m_actual_mbox->environment();
			}

		static so_5::mbox_t
		create( so_5::environment_t & env )
			{
				return so_5::mbox_t( new test_mbox_t( env ) );
			}
	};

struct msg_one : public so_5::signal_t {};
struct msg_two : public so_5::signal_t {};
struct msg_three : public so_5::signal_t {};

struct msg_four : public so_5::message_t {};

struct msg_five : public so_5::signal_t {};

class a_test_t : public so_5::agent_t
{
		typedef so_5::agent_t base_type_t;

	public :
		a_test_t(
			so_5::environment_t & env,
			so_5::subscription_storage_factory_t factory,
			std::string & sequence )
			:	base_type_t( env + factory )
			,	m_sequence( sequence )
			,	m_mbox( test_mbox_t::create( env ) )
		{
		}

		void
		so_define_agent() override
		{
			so_subscribe( m_mbox )
				.event( &a_test_t::evt_default_one )
				.event( &a_test_t::evt_default_two )
				.event( &a_test_t::evt_default_three )
				.event( &a_test_t::evt_default_four )
				.event( &a_test_t::evt_five );

			so_subscribe( m_mbox ).in( st_1 )
				.event( &a_test_t::evt_st_1_one )
				.event( &a_test_t::evt_st_1_two )
				.event( &a_test_t::evt_st_1_three )
				.event( &a_test_t::evt_default_four )
				.event( &a_test_t::evt_five );

			so_subscribe( m_mbox ).in( st_2 )
				.event( &a_test_t::evt_st_2_one )
				.event( &a_test_t::evt_st_2_two )
				.event( &a_test_t::evt_st_2_three )
				.event( &a_test_t::evt_five );
		}

		void
		so_evt_start() override
		{
			so_5::send< msg_one >( m_mbox );
			so_5::send< msg_two >( m_mbox );
			so_5::send< msg_four >( m_mbox );
			so_5::send< msg_five >( m_mbox );
		}

		void
		evt_default_one( mhood_t< msg_one > )
		{
			m_sequence += "d1:";
		}

		void
		evt_default_two( mhood_t< msg_two > )
		{
			m_sequence += "d2:";

			so_drop_subscription( m_mbox, &a_test_t::evt_default_one );

			so_5::send< msg_one >( m_mbox );
			so_5::send< msg_three >( m_mbox );
		}

		void
		evt_default_three( mhood_t< msg_three > )
		{
			m_sequence += "d3:";

			so_change_state( st_1 );

			so_5::send< msg_one >( m_mbox );
			so_5::send< msg_two >( m_mbox );
		}

		void
		evt_default_four( const msg_four & )
		{
			m_sequence += "d4:";

			so_5::send< msg_four >( m_mbox );

			so_drop_subscription( m_mbox, &a_test_t::evt_default_four );
			so_drop_subscription( m_mbox, st_1, &a_test_t::evt_default_four );
		}

		void
		evt_five(mhood_t< msg_five >)
		{
			m_sequence += "d5:";

			so_5::send< msg_five >( m_mbox );

			so_drop_subscription_for_all_states< msg_five >( m_mbox );
		}

		void
		evt_st_1_one( mhood_t< msg_one > )
		{
			m_sequence += "1_d1:";
		}

		void
		evt_st_1_two( mhood_t< msg_two > )
		{
			m_sequence += "1_d2:";

			so_drop_subscription( m_mbox, st_1, &a_test_t::evt_st_1_one );

			so_5::send< msg_one >( m_mbox );
			so_5::send< msg_three >( m_mbox );
		}

		void
		evt_st_1_three( mhood_t< msg_three > )
		{
			m_sequence += "1_d3:";

			so_drop_subscription_for_all_states( m_mbox,
					&a_test_t::evt_default_one );

			so_change_state( st_2 );

			so_5::send< msg_one >( m_mbox );
			so_5::send< msg_two >( m_mbox );
			so_5::send< msg_three >( m_mbox );
		}

		void
		evt_st_1_four( const msg_four & )
		{
			m_sequence += "1_d4:";
		}

		void
		evt_st_2_one( mhood_t< msg_one > )
		{
			m_sequence += "2_d1:";
		}

		void
		evt_st_2_two( mhood_t< msg_two > )
		{
			m_sequence += "2_d2:";
		}

		void
		evt_st_2_three( mhood_t< msg_three > )
		{
			m_sequence += "2_d3:";

			so_environment().stop();
		}

	private :
		std::string & m_sequence;

		so_5::mbox_t m_mbox;

		so_5::state_t st_1{ this, "st_1" };
		so_5::state_t st_2{ this, "st_2" };
};

class test_env_t
{
	public :
		test_env_t(
			so_5::subscription_storage_factory_t factory )
			:	m_factory( std::move( factory ) )
		{}

		void
		init( so_5::environment_t & env )
		{
			env.register_agent_as_coop(
					env.make_agent< a_test_t >( m_factory, m_sequence ) );
		}

		void
		check_result() const
		{
			const std::string expected =
					"d1:d2:d4:d5:d3:1_d1:1_d2:1_d3:2_d2:2_d3:";

			if( m_sequence != expected )
				throw std::runtime_error( "Wrong message sequence: actial: " +
						m_sequence + ", expected: " + expected );
		}

	private :
		const so_5::subscription_storage_factory_t m_factory;

		std::string m_sequence;
};

void
do_test()
{
	using factory_info_t =
			std::pair< std::string, so_5::subscription_storage_factory_t >;
	
	factory_info_t factories[] = {
		{ "vector[1]", so_5::vector_based_subscription_storage_factory( 1 ) }
	,	{ "vector[8]", so_5::vector_based_subscription_storage_factory( 8 ) }
	,	{ "vector[16]", so_5::vector_based_subscription_storage_factory( 16 ) }
	,	{ "map", so_5::map_based_subscription_storage_factory() }
	,	{ "hash_table", so_5::hash_table_based_subscription_storage_factory() }
	,	{ "adaptive[1]", so_5::adaptive_subscription_storage_factory( 1 ) }
	,	{ "adaptive[2]", so_5::adaptive_subscription_storage_factory( 2 ) }
	,	{ "adaptive[3]", so_5::adaptive_subscription_storage_factory( 3 ) }
	,	{ "adaptive[8]", so_5::adaptive_subscription_storage_factory( 8 ) }
	,	{ "flat_set[1]", so_5::flat_set_based_subscription_storage_factory( 1 ) }
	,	{ "flat_set[8]", so_5::flat_set_based_subscription_storage_factory( 8 ) }
	,	{ "flat_set[16]", so_5::flat_set_based_subscription_storage_factory( 16 ) }
	,	{ "default", so_5::default_subscription_storage_factory() }
	}; 

	for( auto & f : factories )
	{
		std::cout << "checking factory: " << f.first << " -> " << std::flush;

		run_with_time_limit(
			[f] {
				test_env_t test_env{ f.second };
				so_5::launch(
					[&]( so_5::environment_t & env )
					{
						test_env.init( env );
					} );

				test_env.check_result();
			}, 
			20,
			"checking factory " + f.first );

		std::cout << "OK" << std::endl;
	}
}

int
main()
{
	try
	{
		do_test();
	}
	catch( const std::exception & ex )
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
