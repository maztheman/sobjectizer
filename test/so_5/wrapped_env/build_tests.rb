#!/usr/local/bin/ruby
require 'mxx_ru/cpp'

MxxRu::Cpp::composite_target {

	path = 'test/so_5/wrapped_env'

	required_prj( "#{path}/simple/prj.ut.rb" )
	required_prj( "#{path}/external_stop_then_join/prj.ut.rb" )
	required_prj( "#{path}/async_start/prj.ut.rb" )
	required_prj( "#{path}/simple_sync_start/prj.ut.rb" )
	required_prj( "#{path}/exception_from_sync_start/prj.ut.rb" )
	required_prj( "#{path}/add_coop_after_start/prj.ut.rb" )
	required_prj( "#{path}/work_thread_activity_on/prj.ut.rb" )
}
