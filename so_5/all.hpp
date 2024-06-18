/*
 * SObjectizer-5
 */

/*!
 * \file
 * \brief A helper header file for including all public SObjectizer stuff.
 *
 * \since v.5.4.0
 */

#pragma once

#include <so_5/rt.hpp>
#include <so_5/api.hpp>
#include <so_5/wrapped_env.hpp>
#include <so_5/env_infrastructures.hpp>

#include <so_5/single_sink_binding.hpp>
#include <so_5/multi_sink_binding.hpp>

#include <so_5/enveloped_msg.hpp>

#include <so_5/mchain_helper_functions.hpp>
#include <so_5/thread_helper_functions.hpp>

#include <so_5/disp/one_thread/pub.hpp>
#include <so_5/disp/nef_one_thread/pub.hpp>
#include <so_5/disp/active_obj/pub.hpp>
#include <so_5/disp/active_group/pub.hpp>
#include <so_5/disp/thread_pool/pub.hpp>
#include <so_5/disp/adv_thread_pool/pub.hpp>
#include <so_5/disp/nef_thread_pool/pub.hpp>
#include <so_5/disp/prio_one_thread/strictly_ordered/pub.hpp>
#include <so_5/disp/prio_one_thread/quoted_round_robin/pub.hpp>
#include <so_5/disp/prio_dedicated_threads/one_per_prio/pub.hpp>

#include <so_5/version.hpp>

#include <so_5/unique_subscribers_mbox.hpp>

#include <so_5/bind_transformer_helpers.hpp>

