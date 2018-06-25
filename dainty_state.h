/******************************************************************************

 MIT License

 Copyright (c) 2018 kieme, frits.germs@gmx.net

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

******************************************************************************/

#ifndef _DAINTY_STATE_
#define _DAINTY_STATE_

#include "dainty_named.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

#define DAINTY_SM_START                                          \
      if (curr_ == stop_) {                                      \
        debug_start(*this, sid);                                 \
        curr_ = initial_point(sid);                              \
        debug(*this, stop_, curr_);                              \
        return do_transition(get_state(curr_)->entry_point());   \
      }                                                          \
      return curr_;

#define DAINTY_SM_STOP                                           \
      if (curr_ != stop_) {                                      \
        get_state(curr_)->exit_point();                          \
        debug(*this, curr_, stop_);                              \
        curr_ = stop_;                                           \
        final_point();                                           \
        debug_stop(*this);                                       \
      }

#define DAINTY_SM_TRANSITION                                     \
      if (next != curr_) {                                       \
        if (next != stop_) {                                     \
          get_state(curr_)->exit_point();                        \
          debug(*this, curr_, next);                             \
          curr_ = next;                                          \
          next = do_transition(get_state(curr_)->entry_point()); \
        } else                                                   \
          stop();                                                \
      }                                                          \
      return curr_;

///////////////////////////////////////////////////////////////////////////////////////////////////

namespace dainty
{
namespace state
{
  //
  // state and statemachine works together to form a framework for building
  // statemachines.
  //
  // classes:
  //
  //   state        -> represents a specific state
  //   statemachine -> consists of states and manage their changes.
  //
  //   each class have four specializations:
  //
  //     1. accept a user reference and specify an interface.
  //     2. accept a user reference and no interface is enforced.
  //     3. has no user reference   and specify an interface.
  //     4. has no user reference   and no interface is enforced.
  //
  //  The user reference can be:
  //
  //    + shared data managed and shared between the states.
  //    + functionality to call to do work from the state.
  //
  //  With these four specializations any type of statemaschine can be
  //  constructed.
  //
  //  A template statemashine has been written that can be copied and pasted
  //  as a starting point and then be modified.

  using named::t_void;

  ///////////////////////////////////////////////////////////////////////////

  enum t_no_user { };
  enum t_no_if   { };

  ///////////////////////////////////////////////////////////////////////////

  template<typename I,    // enum type defining states
           typename U,    // user type which is used by the states
           typename IF>   // enforced interface
  class t_state;

  template<typename I,    // enum type defining states
           typename U,    // user type which is used by the states
           typename IF>   // enforced interface
  class t_statemachine;

  ///////////////////////////////////////////////////////////////////////////

  template<typename I, typename U = t_no_user, typename IF = t_no_if>
  struct t_traits {
    t_traits() = delete;

    using t_self         = state::t_traits      <I, U, IF>;
    using t_state        = state::t_state       <I, U, IF>;
    using t_statemachine = state::t_statemachine<I, U, IF>;
    using t_state_id     = I;
    using t_interface    = IF;
    using t_user         = U;
    using t_user_ref     = U&;
    using t_user_cref    = const U&;
    using p_state        = t_state*;
    using p_cstate       = const t_state*;
  };

  template<typename I, typename IF>
  struct t_traits<I, t_no_user, IF> {
    t_traits() = delete;

    using t_self         = state::t_traits      <I, t_no_user, IF>;
    using t_state        = state::t_state       <I, t_no_user, IF>;
    using t_statemachine = state::t_statemachine<I, t_no_user, IF>;
    using t_state_id     = I;
    using t_interface    = IF;
    using p_state        = t_state*;
    using p_cstate       = const t_state*;
  };

  template<typename I, typename U>
  struct t_traits<I, U, t_no_if> {
    t_traits() = delete;

    using t_self         = state::t_traits      <I, U, t_no_if>;
    using t_state        = state::t_state       <I, U, t_no_if>;
    using t_statemachine = state::t_statemachine<I, U, t_no_if>;
    using t_state_id     = I;
    using t_user         = U;
    using t_user_ref     = U&;
    using t_user_cref    = const U&;
    using p_state        = t_state*;
    using p_cstate       = const t_state*;
  };

  template<typename I>
  struct t_traits<I, t_no_user, t_no_if> {
    t_traits() = delete;

    using t_self         = state::t_traits      <I, t_no_user, t_no_if>;
    using t_state        = state::t_state       <I, t_no_user, t_no_if>;
    using t_statemachine = state::t_statemachine<I, t_no_user, t_no_if>;
    using t_state_id     = I;
    using p_state        = t_state*;
    using p_cstate       = const t_state*;
  };

  ///////////////////////////////////////////////////////////////////////////

  template<typename I, typename U = t_no_user, typename IF = t_no_if>
  class t_state : public IF {
  public:
    using t_traits = state::t_traits<I, U, IF>;
    using t_sid    = typename t_traits::t_state_id;

    // identification of the state
    t_sid get_sid() const                                { return sid_; }

  protected:
    using t_user_ref  = typename t_traits::t_user_ref;
    using t_user_cref = typename t_traits::t_user_cref;

    // state object must have an associated id and access to user
    t_state(t_sid sid, t_user_ref user) : sid_(sid), user_(user)      { }
    virtual ~t_state()                                                { }

    // use to indicate that a state change is requested or not.
    virtual t_sid request_transition(t_sid sid) const    { return sid;  }
            t_sid no_transition     ()          const    { return sid_; }

    t_user_ref  get_user ()                             { return user_; }
    t_user_cref get_user () const                       { return user_; }
    t_user_cref get_cuser() const                       { return user_; }

  private:
    template<typename, typename, typename> friend class t_statemachine;

    // generic actions. entry_point can request a state change.
    virtual t_sid  entry_point()              { return no_transition(); }
    virtual t_void exit_point()                                       { }

    const t_sid sid_;
    t_user_ref  user_;
  };

  template<typename I, typename IF>
  class t_state<I, t_no_user, IF> : public IF {
  public:
    using t_traits    = state::t_traits<I, t_no_user, IF>;
    using t_sid       = typename t_traits::t_state_id;

    // identification of the state
    t_sid get_sid() const                             { return sid_; }

  protected:
    // state object must have an associated id
    t_state(t_sid sid) : sid_(sid)                                 { }
    virtual ~t_state()                                             { }

    // use to indicate that a state change is requested or not.
    virtual t_sid request_transition(t_sid sid) const { return sid;  }
            t_sid no_transition     ()          const { return sid_; }

  private:
    template<typename, typename, typename> friend class t_statemachine;

    // generic actions. entry_point can request a state change.
    virtual t_sid  entry_point()           { return no_transition(); }
    virtual t_void exit_point()                                    { }

    const t_sid sid_;
  };

  template<typename I, typename U>
  class t_state<I, U, t_no_if> {
  public:
    using t_traits = state::t_traits<I, U, t_no_if>;
    using t_sid    = typename t_traits::t_state_id;

    // identification of the state
    t_sid get_sid() const                                { return sid_; }

  protected:
    using t_user_ref  = typename t_traits::t_user_ref;
    using t_user_cref = typename t_traits::t_user_cref;

    // state object must have an associated id and access to user
    t_state(t_sid sid, t_user_ref user) : sid_(sid), user_(user)      { }
    virtual ~t_state()                                                { }

    // use to indicate that a state change is requested or not.
    virtual t_sid request_transition(t_sid sid) const    { return sid;  }
            t_sid no_transition     ()          const    { return sid_; }

    t_user_ref  get_user ()                             { return user_; }
    t_user_cref get_user () const                       { return user_; }
    t_user_cref get_cuser() const                       { return user_; }

  private:
    template<typename, typename, typename> friend class t_statemachine;

    // generic actions. entry_point can request a state change.
    virtual t_sid  entry_point()              { return no_transition(); }
    virtual t_void exit_point()                                       { }

    const t_sid sid_;
    t_user_ref  user_;
  };

  template<typename I>
  class t_state<I, t_no_user, t_no_if> {
  public:
    using t_traits = state::t_traits<I, t_no_user, t_no_if>;
    using t_sid    = typename t_traits::t_state_id;

    // identification of the state
    t_sid get_sid() const                             { return sid_; }

  protected:
    // state object must have an associated id
    t_state(t_sid sid) : sid_(sid)                                 { }
    virtual ~t_state()                                             { }

    // use to indicate that a state change is requested or not.
    virtual t_sid request_transition(t_sid sid) const { return sid;  }
            t_sid no_transition     ()          const { return sid_; }

  private:
    template<typename, typename, typename> friend class t_statemachine;

    // generic actions. entry_point can request a state change.
    virtual t_sid  entry_point()           { return no_transition(); }
    virtual t_void exit_point()                                    { }

    const t_sid sid_;
  };

  ///////////////////////////////////////////////////////////////////////////

  template<typename I, typename U, typename IF>
  inline
  t_void debug_start(const t_statemachine<I,U,IF>&, I start)     { }

  template<typename I, typename U, typename IF>
  inline
  t_void debug_stop(const t_statemachine<I,U,IF>&)               { }

  template<typename I, typename U, typename IF>
  inline
  t_void debug(const t_statemachine<I,U,IF>&, I current, I next) { }

  ///////////////////////////////////////////////////////////////////////////

  template<typename I, typename U = t_no_user, typename IF = t_no_if>
  class t_statemachine : public IF {
  public:
    using t_traits = state::t_traits<I, U, IF>;
    using t_sid    = typename t_traits::t_state_id;

    t_sid get_current_sid() const                         { return curr_; }

  protected:
    using t_user_ref  = typename t_traits::t_user_ref;
    using t_user_cref = typename t_traits::t_user_cref;
    using p_state     = typename t_traits::p_state;
    using p_cstate    = typename t_traits::p_cstate;

    // important. user is NOT owned but used. PRE should be an invalid state.
    t_statemachine(t_sid stop, t_user_ref user)
      : stop_(stop), curr_(stop_), user_(user)                          { }

    // start the statemachine in the id state.
    inline
    t_sid start(t_sid sid) {
      DAINTY_SM_START
    }

    // start the statemachine in the id state.
    inline
    t_sid restart(t_sid sid) {
      stop();
      return start(sid);
    }

    // stop the statemachine in the id state. this is the terminating state.
    inline
    t_void stop() {
      DAINTY_SM_STOP
    }

    // access user
    t_user_ref  get_user ()                              { return user_; }
    t_user_cref get_user () const                        { return user_; }
    t_user_cref get_cuser() const                        { return user_; }

    // access to current state pointer
    p_state  get_current()                    { return get_state(curr_); }
    p_cstate get_current()  const             { return get_state(curr_); }
    p_cstate get_ccurrent() const           { return get_current(curr_); }

    // control state changes if required. return indicate if state was changed
    inline
    t_sid do_transition(t_sid next) {
      DAINTY_SM_TRANSITION
    }

  private:
    // provide methods that detect when the statemachine starts/stops
    virtual t_sid  initial_point(t_sid id) { return id; }
    virtual t_void final_point  ()                    { }

    // access to state pointer associated with their ids.
    virtual p_state  get_state (t_sid)          = 0;
    virtual p_cstate get_state (t_sid) const    = 0;

    inline
    p_cstate get_cstate(t_sid sid) const        { return get_state(sid); }

    const t_sid stop_;
          t_sid curr_;
    t_user_ref  user_;
  };

  template<typename I, typename IF>
  class t_statemachine<I, t_no_user, IF> : public IF {
  public:
    using t_traits = state::t_traits<I, t_no_user, IF>;
    using t_sid    = typename t_traits::t_state_id;

    t_sid get_current_sid() const               { return curr_; }

  protected:
    using p_state  = typename t_traits::p_state;
    using p_cstate = typename t_traits::p_cstate;

    // important. user is NOT owned but used. PRE should be an invalid state.
    t_statemachine(t_sid stop) : stop_(stop), curr_(stop_) { }

    // start the statemachine in the id state.
    t_sid start(t_sid sid) {
      DAINTY_SM_START
    }

    // start the statemachine in the id state.
    t_sid restart(t_sid sid) {
      stop();
      return start(sid);
    }

    // stop the statemachine in the id state. this is the terminating state.
    t_void stop() {
      DAINTY_SM_STOP
    }

    // access to current state pointer
    p_state  get_current ()       { return get_state (curr_); }
    p_cstate get_current () const { return get_state (curr_); }
    p_cstate get_ccurrent() const { return get_cstate(curr_); }

    // control state changes if required. return indicate if state was changed
    t_sid do_transition(t_sid next) {
      DAINTY_SM_TRANSITION
    }

  private:
    // provide methods that detect when the statemachine starts/stops
    virtual t_sid  initial_point(t_sid id) { return id; }
    virtual t_void final_point  ()                    { }

    // access to state pointer associated with their ids.
    virtual p_state  get_state(t_sid)       = 0;
    virtual p_cstate get_state(t_sid) const = 0;

    inline
    p_cstate get_cstate(t_sid sid) const { return get_state(sid); }

    const t_sid stop_;
          t_sid curr_;
  };

  template<typename I, typename U>
  class t_statemachine<I, U, t_no_if> {
  public:
    using t_traits = state::t_traits<I, U, t_no_if>;
    using t_sid    = typename t_traits::t_state_id;

    t_sid get_current_sid() const                         { return curr_; }

  protected:
    using t_user_ref  = typename t_traits::t_user_ref;
    using t_user_cref = typename t_traits::t_user_cref;
    using p_state     = typename t_traits::p_state;
    using p_cstate    = typename t_traits::p_cstate;

    // important. user is NOT owned but used. PRE should be an invalid state.
    t_statemachine(t_sid stop, t_user_ref user)
      : stop_(stop), curr_(stop_), user_(user)                          { }

    // start the statemachine in the id state.
    t_sid start(t_sid sid) {
      DAINTY_SM_START
    }

    // start the statemachine in the id state.
    t_sid restart(t_sid sid) {
      stop();
      return start(sid);
    }

    // stop the statemachine in the id state. this is the terminating state.
    t_void stop() {
      DAINTY_SM_STOP
    }

    // NOTE:     current() is not given because there is no interface.
    // NOTE use: get_state(current_sid()) to get current state from the
    //           the derived class.

    // access user
    t_user_ref  get_user ()                     { return user_; }
    t_user_cref get_user () const               { return user_; }
    t_user_cref get_cuser() const               { return user_; }

    // access to state pointer associated with their ids.
    virtual p_state  get_state(t_sid)       = 0;
    virtual p_cstate get_state(t_sid) const = 0;

    inline
    p_cstate get_cstate(t_sid sid) const { return get_state(sid); }

    // control state changes if required. return indicate if state was changed
    t_sid do_transition(t_sid next) {
      DAINTY_SM_TRANSITION
    }

  private:
    // provide methods that detect when the statemachine starts/stops
    virtual t_sid  initial_point(t_sid id) { return id; }
    virtual t_void final_point  ()                    { }

    const t_sid stop_;
          t_sid curr_;
    t_user_ref  user_;
  };

  template<typename I>
  class t_statemachine<I, t_no_user, t_no_if> {
  public:
    using t_traits = state::t_traits<I, t_no_user, t_no_if>;
    using t_sid    = typename t_traits::t_state_id;

    t_sid get_current_sid() const               { return curr_; }

  protected:
    using p_state  = typename t_traits::p_state;
    using p_cstate = typename t_traits::p_cstate;

    // important. user is NOT owned but used. PRE should be an invalid state.
    t_statemachine(t_sid stop) : stop_(stop), curr_(stop_) { }

    // start the statemachine in the id state.
    t_sid start(t_sid sid) {
      DAINTY_SM_START
    }

    // start the statemachine in the id state.
    t_sid restart(t_sid sid) {
      stop();
      return start(sid);
    }

    // stop the statemachine in the id state. this is the terminating state.
    t_void stop() {
      DAINTY_SM_STOP
    }

    // NOTE:     current() is not given because there is no interface.
    // NOTE use: get_state(current_sid()) to get current state from the
    //           the derived class.

    // access to state pointer associated with their ids.
    virtual p_state  get_state(t_sid)       = 0;
    virtual p_cstate get_state(t_sid) const = 0;

    inline p_cstate get_cstate(t_sid sid) const { return get_state(sid); }

    // control state changes if required. return indicate if state was changed
    t_sid do_transition(t_sid next) {
      DAINTY_SM_TRANSITION
    }

  private:
    // provide methods that detect when the statemachine starts/stops
    virtual t_sid  initial_point(t_sid id) { return id; }
    virtual t_void final_point  ()                    { }

    const t_sid stop_;
          t_sid curr_;
  };

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
}
}

#endif
