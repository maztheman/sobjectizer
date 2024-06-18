/*
 * A test for custom mbox creation.
 */

#include <iostream>
#include <sstream>

#include <so_5/all.hpp>

#include <test/3rd_party/various_helpers/time_limited_execution.hpp>

struct msg_one : public so_5::signal_t {};
struct msg_two : public so_5::signal_t {};
struct msg_three : public so_5::signal_t {};
struct msg_four : public so_5::signal_t {};

class my_mbox_t : public so_5::abstract_message_box_t
{
public :
	my_mbox_t(
		so_5::mbox_t actual_mbox )
		:	m_actual_mbox(std::move(actual_mbox))
		{}

	virtual so_5::mbox_id_t
	id() const override
		{
			return m_actual_mbox->id();
		}

	virtual void
	subscribe_event_handler(
		const std::type_index & type_index,
		so_5::abstract_message_sink_t & subscriber ) override
		{
			if( is_enabled_message( type_index ) )
				m_actual_mbox->subscribe_event_handler(
						type_index, subscriber );
		}

	virtual void
	unsubscribe_event_handler(
		const std::type_index & type_index,
		so_5::abstract_message_sink_t & subscriber ) noexcept override
		{
			if( is_enabled_message( type_index ) )
				m_actual_mbox->unsubscribe_event_handler( type_index, subscriber );
		}

	virtual std::string
	query_name() const override
		{
			return "<MY_MBOX>";
		}

	virtual so_5::mbox_type_t
	type() const override
		{
			return m_actual_mbox->type();
		}

	virtual void
	do_deliver_message(
		so_5::message_delivery_mode_t delivery_mode,
		const std::type_index & msg_type,
		const so_5::message_ref_t & message,
		unsigned int redirection_deep ) override
		{
			m_actual_mbox->do_deliver_message(
					delivery_mode,
					msg_type,
					message,
					redirection_deep );
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

private :
	const so_5::mbox_t m_actual_mbox;

	static bool
	is_enabled_message( const std::type_index & msg_type )
		{
			static const std::type_index t1 = typeid(msg_one);
			static const std::type_index t4 = typeid(msg_four);

			return (t1 == msg_type || t4 == msg_type);
		}
};

class a_test_t : public so_5::agent_t
{
	using base_type_t = so_5::agent_t;

	public :
		a_test_t(
			context_t ctx,
			so_5::mbox_t mbox,
			std::string & sequence )
			:	base_type_t( std::move(ctx) )
			,	m_mbox( std::move(mbox) )
			,	m_sequence( sequence )
		{
		}

		void
		so_define_agent() override
		{
			so_subscribe( m_mbox )
				.event( &a_test_t::evt_one )
				.event( &a_test_t::evt_two )
				.event( &a_test_t::evt_three )
				.event( &a_test_t::evt_four );
		}

		void
		evt_one( mhood_t<msg_one> )
		{
			m_sequence += "e1:";
		}

		void
		evt_two( mhood_t<msg_two> )
		{
			m_sequence += "e2:";
		}

		void
		evt_three( mhood_t<msg_three> )
		{
			m_sequence += "e3:";
		}

		void
		evt_four( mhood_t<msg_four> )
		{
			m_sequence += "e4:";

			so_environment().stop();
		}

	private :
		const so_5::mbox_t m_mbox;
		std::string & m_sequence;
};

int
main()
{
	run_with_time_limit( [] {
		std::string sequence;

		so_5::launch(
			[&sequence]( so_5::environment_t & env )
			{
				auto actual_mbox = env.create_mbox();
				auto my_mbox = env.make_custom_mbox(
						[actual_mbox]( const so_5::mbox_creation_data_t & ) {
							return so_5::mbox_t( new my_mbox_t(actual_mbox) );
						} );

				env.introduce_coop( [my_mbox, &sequence]( so_5::coop_t & coop ) {
							coop.make_agent< a_test_t >( my_mbox, std::ref(sequence) );
						} );

				so_5::send< msg_one >(my_mbox);
				so_5::send< msg_two >(my_mbox);
				so_5::send< msg_three >(my_mbox);
				so_5::send< msg_four >(my_mbox);
			} );

		const std::string expected = "e1:e4:";
		if( sequence != expected )
			throw std::runtime_error( "sequence mismatch! "
					"expected: '" + expected + "', actual: '"
					+ sequence + "'" );

	},
	10 );

	return 0;
}

