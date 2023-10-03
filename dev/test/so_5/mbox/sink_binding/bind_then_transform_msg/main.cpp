/*
 * Test cases for bind_then_transform and messages.
 */

#include "common.hpp"

namespace test {

void
run_implicit_type_no_optional_no_dr();

void
run_implicit_type_no_optional_with_dr();

void
run_implicit_type_with_optional_no_dr();

void
run_implicit_type_with_optional_with_dr();

void
run_explicit_type_no_optional_no_dr();

void
run_explicit_type_no_optional_with_dr();

void
run_explicit_type_with_optional_no_dr();

void
run_explicit_type_with_optional_with_dr();

void
run_tests()
	{
		run_implicit_type_no_optional_no_dr();
		run_implicit_type_no_optional_with_dr();
		run_implicit_type_with_optional_no_dr();
		run_implicit_type_with_optional_with_dr();
		run_explicit_type_no_optional_no_dr();
		run_explicit_type_no_optional_with_dr();
		run_explicit_type_with_optional_no_dr();
		run_explicit_type_with_optional_with_dr();
	}

} /* namespace test */

int
main()
	{
		test::run_tests();

		return 0;
	}
