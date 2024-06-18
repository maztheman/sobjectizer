/*
 * A test for resending delayed messages/signals via mhood to mchain.
 * This test is for various formats of send_delayed template functions.
 */

#include <so_5/all.hpp>

#include <test/3rd_party/various_helpers/time_limited_execution.hpp>
#include <test/3rd_party/various_helpers/ensure.hpp>

#include <test/3rd_party/utest_helper/helper.hpp>

using namespace std;

class first_tester final : public so_5::agent_t
{
	struct hello final : public so_5::signal_t {};
	struct mchain_has_messages final : public so_5::signal_t {};

public :
	first_tester(context_t ctx)
		:	so_5::agent_t(std::move(ctx))
	{
		m_mchain = so_environment().create_mchain(
				so_5::make_unlimited_mchain_params().not_empty_notificator(
					[this]{ so_5::send< mchain_has_messages >(*this); } ) );

		so_subscribe_self()
			.event( &first_tester::on_hello )
			.event( &first_tester::on_mchain_has_messages );
	}

	virtual void
	so_evt_start() override
	{
		so_5::send_delayed< so_5::immutable_msg<hello> >(
				*this,
				std::chrono::milliseconds(50) );
	}

private :
	so_5::mchain_t m_mchain;

	void on_hello( mhood_t< hello > cmd )
	{
		so_5::send_delayed(
				m_mchain,
				std::chrono::milliseconds(25),
				std::move(cmd) );
	}

	void on_mchain_has_messages( mhood_t< mchain_has_messages > )
	{
		so_5::receive( from(m_mchain).handle_n(1),
				[this]( mhood_t< hello > ) {
					so_deregister_agent_coop_normally();
				} );
	}
};

class second_tester final : public so_5::agent_t
{
	struct hello final : public so_5::message_t
	{
		std::string m_greeting;
		hello( std::string greeting ) : m_greeting(std::move(greeting)) {}

		// Copy or move constructors should not be invoked during
		// redirection.
		hello( const hello & ) = delete;
		hello( const hello && ) = delete;
	};
	struct mchain_has_messages final : public so_5::signal_t {};

public :
	second_tester(context_t ctx)
		:	so_5::agent_t(std::move(ctx))
	{
		m_mchain = so_environment().create_mchain(
				so_5::make_unlimited_mchain_params().not_empty_notificator(
					[this]{ so_5::send< mchain_has_messages >(*this); } ) );

		so_subscribe_self()
			.event( &second_tester::on_hello )
			.event( &second_tester::on_mchain_has_messages );
	}

	virtual void
	so_evt_start() override
	{
		so_5::send_delayed< so_5::immutable_msg<hello> >(
				*this,
				std::chrono::milliseconds(50),
				"hello" );
	}

private :
	so_5::mchain_t m_mchain;

	void on_hello( mhood_t< hello > cmd )
	{
		so_5::send_delayed(
				m_mchain,
				std::chrono::milliseconds(25),
				std::move(cmd) );
	}

	void on_mchain_has_messages( mhood_t< mchain_has_messages > )
	{
		so_5::receive( from(m_mchain).handle_n(1),
				[this]( mhood_t< hello > cmd ) {
					ensure_or_die( "hello" == cmd->m_greeting,
							"'hello' is expected in the message" );
					so_deregister_agent_coop_normally();
				} );
	}
};

class third_tester final : public so_5::agent_t
{
	struct hello final : public so_5::message_t
	{
		std::string m_greeting;
		hello( std::string greeting ) : m_greeting(std::move(greeting)) {}

		// Copy or move constructors should not be invoked during
		// redirection.
		hello( const hello & ) = delete;
		hello( const hello && ) = delete;
	};
	struct mchain_has_messages final : public so_5::signal_t {};

public :
	third_tester(context_t ctx)
		:	so_5::agent_t(std::move(ctx))
	{
		m_mchain = so_environment().create_mchain(
				so_5::make_unlimited_mchain_params().not_empty_notificator(
					[this]{ so_5::send< mchain_has_messages >(*this); } ) );

		so_subscribe_self()
			.event( &third_tester::on_hello )
			.event( &third_tester::on_mchain_has_messages );
	}

	virtual void
	so_evt_start() override
	{
		so_5::send_delayed< so_5::mutable_msg<hello> >(
				*this,
				std::chrono::milliseconds(50),
				"hello" );
	}

private :
	so_5::mchain_t m_mchain;

	void on_hello( mutable_mhood_t< hello > cmd )
	{
		so_5::send_delayed(
				m_mchain,
				std::chrono::milliseconds(25),
				std::move(cmd) );
	}

	void on_mchain_has_messages( mhood_t< mchain_has_messages > )
	{
		so_5::receive( from(m_mchain).handle_n(1),
				[this]( mutable_mhood_t< hello > cmd ) {
					ensure_or_die( "hello" == cmd->m_greeting,
							"'hello' is expected in the message" );
					so_deregister_agent_coop_normally();
				} );
	}
};

int
main()
{
	try
	{
		run_with_time_limit(
			[]() {
				so_5::launch(
					[&](so_5::environment_t & env) {
						env.register_agent_as_coop(
								env.make_agent< first_tester >());

						env.register_agent_as_coop(
								env.make_agent< second_tester >());

						env.register_agent_as_coop(
								env.make_agent< third_tester >());
					},
					[](so_5::environment_params_t & params) {
						(void)params;
#if 0
						params.message_delivery_tracer( so_5::msg_tracing::std_cout_tracer() );
#endif
					} );
			},
			5,
			"simple agent");
	}
	catch(const exception & ex)
	{
		cerr << "Error: " << ex.what() << endl;
		return 1;
	}

	return 0;
}

